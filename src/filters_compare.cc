/*
* mod_compare - compare apache requests
*
* Copyright (C) 2013 Orange
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "mod_compare.hh"
#include "RequestInfo.hh"
#include "Utils.hh"
#include "libws_diff/DiffPrinter/diffPrinter.hh"
#include "libws_diff/DiffPrinter/multilineDiffPrinter.hh"
#include "libws_diff/DiffPrinter/jsonDiffPrinter.hh"


#include <http_config.h>
#include <httpd.h>
// Work-around boost::chrono 1.53 conflict on CR typedef vs define in apache
#undef CR

#include <assert.h>
#include <stdexcept>
#include <boost/thread/detail/singleton.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/tokenizer.hpp>
#include <boost/smart_ptr.hpp>
#include <iomanip>

#define DEF_METHOD(name) static const char *g##name = #name;

DEF_METHOD(GET)
DEF_METHOD(POST)
DEF_METHOD(PUT)
DEF_METHOD(PATCH)
DEF_METHOD(DELETE)


namespace CompareModule {

void
printRequest(request_rec *pRequest, std::string pBody)
{
    const char *reqId = apr_table_get(pRequest->headers_in, CommonModule::c_UNIQUE_ID);
    Log::debug("[COMPARE] Filtering a request with ID: %s, body size:%ld", reqId, pBody.size());
    Log::debug("[COMPARE] Uri:%s", pRequest->uri);
    Log::debug("[COMPARE] Request args: %s", pRequest->args);
}

void
changeMethod(request_rec *pRequest, const std::string& pMethod){

    if( ! pMethod.compare(pRequest->method)){
        return;
    }

    if ( !pMethod.compare(gGET)){
        pRequest->method = gGET;
        pRequest->method_number = M_GET;
    }
    else if( !pMethod.compare(gPUT) ){
        pRequest->method = gPUT;
        pRequest->method_number = M_PUT;
    }
    else if( !pMethod.compare(gPATCH) ){
        pRequest->method = gPATCH;
        pRequest->method_number = M_PATCH;
    }
    else if( !pMethod.compare(gPOST) ){
        pRequest->method = gPOST;
        pRequest->method_number = M_POST;
    }
    else if( !pMethod.compare(gDELETE) ){
        pRequest->method = gDELETE;
        pRequest->method_number = M_DELETE;
    }
}


/*
 * Callback to iterate over the headers tables
 * Pushes a copy of key => value in a list
 */
int iterateOverHeadersCallBack(void *d, const char *key, const char *value) {
    std::map< std::string, std::string> *lHeader = reinterpret_cast< std::map< std::string, std::string> *>(d);

    (*lHeader)[std::string(key)] = std::string(value);

    return 1;
}

/** @brief initializes the RequestInfo and input headers
 * Called in translateHook or inputFilter, which ever is called first
 * It is used to remove the DUP headers and change the request method
 */
boost::shared_ptr<DupModule::RequestInfo>* initRequest(request_rec *pRequest)
{
    boost::shared_ptr<DupModule::RequestInfo>* shReqInfo = CommonModule::makeRequestInfo<DupModule::RequestInfo,&compare_module>(pRequest);
    DupModule::RequestInfo *info = shReqInfo->get();
    
    const char *lMethod = apr_table_get(pRequest->headers_in, "X_DUP_METHOD");
    if(lMethod){
        changeMethod(pRequest, lMethod);
        apr_table_unset(pRequest->headers_in, "X_DUP_METHOD");
    }
    
    const char *lDupType = apr_table_get(pRequest->headers_in, "Content-Type");
    if ( lDupType && ( ! strncmp(lDupType, "application/x-dup-serialized", 28) ) ) {
        // leave a compare header for decorator only if duplicating with response
        apr_table_set(pRequest->headers_in, "X-COMPARE-TRANSLATED", "1");
    }
    
    const char *lContentType = apr_table_get(pRequest->headers_in, "X_DUP_CONTENT_TYPE");
    if(lContentType){
        apr_table_set(pRequest->headers_in, "Content-Type", lContentType);
        ap_set_content_type(pRequest, lContentType );
        apr_table_unset(pRequest->headers_in, "X_DUP_CONTENT_TYPE");
    }
    
    // Copy headers in our object
    apr_table_do(&iterateOverHeadersCallBack, &(info->mReqHeader), pRequest->headers_in, NULL);
    
    // We retrieve the original request HTTP status from X_DUP_HTTP_STATUS header
    // if it does not exist, we set it to -1
    std::map<std::string,std::string>::const_iterator it = info->mReqHeader.find("X_DUP_HTTP_STATUS");
    try {
        info->mReqHttpStatus = it != info->mReqHeader.end() ? boost::lexical_cast<int>(it->second) : -1;
    } catch (boost::bad_lexical_cast& e) {
        info->mReqHttpStatus = -1;
        Log::warn(1, "Invalid X_DUP_HTTP_STATUS header value (not a number?)");
    }
    
    Log::debug("[DEBUG][COMPARE] REMAINING: %ld", pRequest->remaining);
    apr_table_unset(pRequest->headers_in, "ELAPSED_TIME_BY_DUP");
    apr_table_unset(pRequest->headers_in, "X_DUP_HTTP_STATUS");
    
    return shReqInfo;
}

/** @brief Translate_name level HOOK
 * It will be called before the input filters
 * It is only called if there is no Alias on this path
 * It is used to remove the DUP headers and change the request method
 */
int translateHook(request_rec *pRequest) {
    if (!pRequest->per_dir_config)
        return DECLINED;
    CompareConf *conf = reinterpret_cast<CompareConf *>(ap_get_module_config(pRequest->per_dir_config, &compare_module));
    if ((!conf) || (!conf->mIsActive)) {
        // Not a location that we treat, we decline the request
        return DECLINED;
    }
    if (!pRequest->connection->pool) {
        Log::error(42, "No connection pool associated to the request");
        return DECLINED;
    }
    Log::debug("[DEBUG][COMPARE] Inside translateHook");
    
    initRequest(pRequest);

    return DECLINED;
}


/**
 * @brief deserializes the body, stores it in RequestInfo, and reinjects the actual body if needed
 * If the request handler does not read body bytes, this will never be called and compare cannot work
 * This is called first and translateHook is never called if the path is an Alias
 */
apr_status_t inputFilterHandler(ap_filter_t *pF, apr_bucket_brigade *pB, ap_input_mode_t pMode, apr_read_type_e pBlock, apr_off_t pReadbytes)
{
    Log::debug("[DEBUG][COMPARE] Inside inputFilterHandler");

    apr_status_t lStatus;
    request_rec *pRequest = pF->r;
    if (!pRequest) {
        Log::debug("[DEBUG][COMPARE] inputFilterHandler request_rec null");
        return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes);
    }
    
    const char *lDupType = apr_table_get(pRequest->headers_in, "Duplication-Type");
    if (( lDupType == NULL ) || ( strcmp("Response", lDupType) != 0) ) {
        Log::debug("[DEBUG][COMPARE] inputFilterHandler not a duplicated request on path %s, nothing to compare", pRequest->uri);
        return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes);
    }

    if(pRequest->per_dir_config == NULL){
        Log::debug("[DEBUG][COMPARE] inputFilterHandler per_dir_config is null");
        return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes);
    }
    struct CompareConf *tConf = reinterpret_cast<CompareConf *>(ap_get_module_config(pRequest->per_dir_config, &compare_module));
    if (!tConf) {
        Log::debug("[DEBUG][COMPARE] inputFilterHandler per_dir_config is null");
        return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes); // SHOULD NOT HAPPEN
    }

    // No context? new request or request called with an alias, translateHook was not called
    if (!pF->ctx) {
        Log::debug("[DEBUG][COMPARE] inputFilterHandler Assigning filter ctx");
        boost::shared_ptr<DupModule::RequestInfo> *shPtr = reinterpret_cast<boost::shared_ptr<DupModule::RequestInfo> *>(ap_get_module_config(pRequest->request_config, &compare_module));
        if ( ! shPtr ) {
            shPtr = initRequest(pRequest);
            Log::debug("[DEBUG][COMPARE] Translate hook was not called, request created directly in the input filter");
        }
        assert(shPtr->get());
        // Backup of info struct in the request context
        pF->ctx = shPtr->get();

        DupModule::RequestInfo *lRI = static_cast<DupModule::RequestInfo *>(pF->ctx);
        Log::debug("[DEBUG][COMPARE] inputFilterHandler Starting extractBrigadeContent");
        while (!CommonModule::extractBrigadeContent(pB, pF->next, lRI->mBody)){
            apr_brigade_cleanup(pB);
        }
        pF->ctx = (void *)1;
        apr_brigade_cleanup(pB);
        lRI->offset = 0;
        Log::debug("[DEBUG][COMPARE] inputFilterHandler Starting deserializeBody");
        lStatus =  deserializeBody(*lRI);

        // reset timer to not take deserializing computation time into account
        (*shPtr)->resetStartTime();

        if(lStatus != APR_SUCCESS){
            return lStatus;
        }

        apr_table_set(pRequest->headers_in, "Content-Length",boost::lexical_cast<std::string>(lRI->mReqBody.size()).c_str());

        printRequest(pRequest, lRI->mReqBody);
    }
    if (pF->ctx == (void *)1) {
        // Request is already read and deserialized, sending it to the client
        boost::shared_ptr<DupModule::RequestInfo> * reqInfo(reinterpret_cast<boost::shared_ptr<DupModule::RequestInfo> *>(ap_get_module_config(pF->r->request_config,
                                                                                                                         &compare_module)));
        DupModule::RequestInfo *lRI = reqInfo->get();
        std::string &lBodyToSend = lRI->mReqBody;

        int toSend = std::min((lBodyToSend.size() - lRI->offset), (size_t)pReadbytes);
        if (toSend > 0){
            apr_status_t st;
            if ((st = apr_brigade_write(pB, NULL, NULL, lBodyToSend.c_str() + lRI->offset, toSend)) != APR_SUCCESS ) {
                Log::warn(1, "Failed to write request body in a brigade");
                return st;
            }
            lRI->offset += toSend;
            return APR_SUCCESS;
        } else {
            return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes);
        }
    }
    // Everything is read and rewritten, simply returning a get brigade call
    return ap_get_brigade(pF->next, pB, pMode, pBlock, pReadbytes);
}


/// @brief first output filter, stores the response in RequestInfo
apr_status_t
outputFilterHandler(ap_filter_t *pFilter, apr_bucket_brigade *pBrigade) {
    Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler");

    request_rec *pRequest = pFilter->r;
    apr_status_t lStatus;
    if (pFilter->ctx == (void *)-1){
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    // Truncate the log before writing if the URI is set to "comp_truncate"
    std::string lArgs( static_cast<const char *>(pRequest->uri) );
    if ( lArgs.find("comp_truncate") != std::string::npos){
        gFile.close();
        gFile.open(gFilePath, std::ofstream::out | std::ofstream::trunc );
        pFilter->ctx = (void *) -1;
        lStatus = ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    if (!pRequest || !pRequest->per_dir_config) {
        pFilter->ctx = (void *) -1;
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    const char *lDupType = apr_table_get(pRequest->headers_in, "Duplication-Type");
    if ( ( lDupType == NULL ) || ( strcmp("Response", lDupType) != 0) ) {
        pFilter->ctx = (void *) -1;
        lStatus = ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    struct CompareConf *tConf = reinterpret_cast<CompareConf *>(ap_get_module_config(pRequest->per_dir_config, &compare_module));
    if( tConf == NULL ){
        pFilter->ctx = (void *) -1;
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    boost::shared_ptr<DupModule::RequestInfo> *shPtr(reinterpret_cast<boost::shared_ptr<DupModule::RequestInfo> *>(ap_get_module_config(pRequest->request_config, &compare_module)));

    if (!shPtr || !shPtr->get()) {
        pFilter->ctx = (void *) -1;
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }
    DupModule::RequestInfo *req = shPtr->get();

    apr_bucket *currentBucket;
    apr_status_t rv;
    for ( currentBucket = APR_BRIGADE_FIRST(pBrigade);
          currentBucket != APR_BRIGADE_SENTINEL(pBrigade);
          currentBucket = APR_BUCKET_NEXT(currentBucket) ) {
        const char *data;
        apr_size_t len;

        if (APR_BUCKET_IS_EOS(currentBucket)) {
            Log::debug("[DEBUG][COMPARE] in outputFilterHandler eos_seen");
            req->eos_seen(true);
            continue;
        }

        rv = apr_bucket_read(currentBucket, &data, &len, APR_BLOCK_READ);

        if ((rv == APR_SUCCESS) && (data != NULL)) {
            req->mDupResponseBody.append(data, len);
        }
    }

    if (apr_table_get(pRequest->headers_in, "X_COMP_LOG") != NULL) {
        Log::debug("[DEBUG][COMPARE] all the body is received,X_COMP_LOG is set, responses will be compared");
        std::string out = "COMPARED - with conf from ";
        out += tConf->mDirName;
        out += " logged in ";
        if (gWriteInFile) {
            out += gFilePath;
        } else {
            out += "SYSLOG";
        }
        out += " in format ";
        out += LibWsDiff::diffPrinter::diffTypeStr(tConf->mLogType);
        apr_table_set( pRequest->headers_out,"X-COMP-STATUS", out.c_str());
    }

    rv = ap_pass_brigade(pFilter->next, pBrigade);
    apr_brigade_cleanup(pBrigade);
    return rv;
}


/// @brief second output filter, performs the actual comparison
apr_status_t
outputFilterHandler2(ap_filter_t *pFilter, apr_bucket_brigade *pBrigade) {
    Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler2");

    apr_status_t lStatus;
    if (pFilter->ctx == (void *)-1){
        Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler2 pass because ctx -1");
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    request_rec *pRequest = pFilter->r;

    struct CompareConf *tConf = reinterpret_cast<CompareConf *>(ap_get_module_config(pRequest->per_dir_config, &compare_module));
    if( tConf == NULL ){
        Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler2 pass because no per dir conf");
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    boost::shared_ptr<DupModule::RequestInfo> *shPtr(reinterpret_cast<boost::shared_ptr<DupModule::RequestInfo> *>(ap_get_module_config(pRequest->request_config, &compare_module)));

    if ( !shPtr || !shPtr->get()) {
        Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler2 pass because no request info");
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
        return lStatus;
    }

    DupModule::RequestInfo *req = shPtr->get();
    if (!req->eos_seen()) {
        Log::debug("[DEBUG][COMPARE] Inside outputFilterHandler2 pass because not eos");
        lStatus =  ap_pass_brigade(pFilter->next, pBrigade);
        apr_brigade_cleanup(pBrigade);
         return lStatus;
    }

    req->mRequest = std::string(pRequest->unparsed_uri);
    //write headers in Map
    apr_table_do(&iterateOverHeadersCallBack, &(req->mDupResponseHeader), pRequest->headers_out, NULL);

    //Check if output is json or not
    boost::scoped_ptr<LibWsDiff::diffPrinter> printer(LibWsDiff::diffPrinter::createDiffPrinter(req->mId,tConf->mLogType));

    if (tConf->mCompareDisabled) {
        Log::debug("[DEBUG][COMPARE] comparison disabled: write serialized request to file");
        writeSerializedRequest(*req);
    } else {
        req->mDupResponseHttpStatus = pRequest->status;
        boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
        Log::debug("[DEBUG][COMPARE] retrieve differences if any");
        bool headerDiff = tConf->mCompHeader.retrieveDiff(req->mResponseHeader,req->mDupResponseHeader,*printer);
        bool bodyDiff = tConf->mCompBody.retrieveDiff(req->mResponseBody,req->mDupResponseBody,*printer);
        if ( headerDiff || bodyDiff) {
            Log::debug("[DEBUG][COMPARE] header or body differences found");
            if(printer->isDiff() || checkCassandraDiff(req->mId) || (req->mReqHttpStatus!=-1 && (req->mReqHttpStatus != req->mDupResponseHttpStatus)) ){
                Log::debug("[DEBUG][COMPARE] write differences to file or syslog");
                writeDifferences(*req,*printer,boost::posix_time::microsec_clock::universal_time()-start);
            }
        }
    }
    pFilter->ctx = (void *) -1;
    lStatus = ap_pass_brigade(pFilter->next, pBrigade);
    apr_brigade_cleanup(pBrigade);
    return lStatus;
}

};
