/*
* mod_dup - duplicates apache requests
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

#pragma once

/*
 * Body to serve for tests purposes
 */

const char *testBody42 = "I am the body 42";

const char *testBody43p1 = "Mr. Brown: Let me tell you what Like a Virgin is about. Its all about a girl who digs a guy with a big dick. The entire song. Its a metaphor for big dicks.\nMr. Blonde: No, no. Its about a girl who is very vulnerable. Shes been fucked over a few times. Then she meets some guy whos really sensitive..\nMr. Brown: Whoa, whoa, whoa, whoa, whoa... Time out Greenbay. Tell that fucking bullshit to the tourists\nJoe: Toby... Who the fuck is Toby? Toby..\nMr. Brown: Like a Virgin is not about this sensitive girl who meets a nice fella. Thats what True Blue is about, now, granted, no argument about that Mr. Orange: Which one is True Blue Nice Guy Eddie: True Blue was a big ass hit for Madonna. I dont even follow this Tops In Pops shit, and Ive at least heard of True BlueMr. Orange: Look, asshole, I didnt say I aint heard of it. All I asked was how does it go? Excuse me for not being the worlds biggest Madonna fan Mr. Blonde: Personally, I can do without her Mr. Blue: I like her early stuff. You know, Lucky Star, Borderline - but once she got into her Papa Dont Preach phase, I dont know, I tuned out Mr. Brown: Hey, you guys are making me lose my... train of thought here. I was saying something, what was it Joe: Oh, Toby was this Chinese girl, what was her last name Mr. White: Whats that? Joe: I found this old address book in a jacket I aint worn in a coons age. What was that name? Mr. Brown: What the fuck was I talking about? Mr. Pink: You said True Blue was about a nice girl, a sensitive girl who meets a nice guy, and that Like a Virgin was a metaphor for big dicks. Mr. Brown: Lemme tell you what Like a Virgin is about. Its all about this cooze whos a regular fuck machine, Im talking morning, day, night, afternoon, dick, dick, dick, dick, dick, dick, dick, dick, dick. Mr. Blue: How many dicks is that? Mr. White: A lot. Mr. Brown: Then one day she meets this John Holmes motherfucker and its like, whoa baby, I mean this cat is like Charles Bronson in the Great Escape, hes digging tunnels. Now, shes gettin the serious dick action and shes feeling something she aint felt since forever. Pain. Pain. Joe: Chew? Toby Chew? Mr. Brown: It hurts her. It shouldnt hurt her, you know, her pussy should be Bubble Yum by now, but when this cat fucks her it hurts. It hurts just like it did the first time. You see the pain is reminding a fuck machine what it once was like to be a virgin. Hence, Like a Virgin. Joe: Wong? Mr. Pink: [walks in] Was that a fucking set up or what? [sees a bloodied Mr. Orange lying on the floor] Mr. Pink: Shit! Orange got tagged? Mr. White: Gut shot. Mr. Pink: Fuck! Wheres, uh, Brown? Mr. White: Dead. Mr. Pink: Howd he die? Mr. White: How the fuck do you think? The cops shot him. Mr. Pink: This is bad. This is so fucking bad. Is it bad? Mr. White: As opposed to good? Mr. Pink: Man, this is fucked up. This is so fucked up. Somebody fucked us up big time, man. Mr. White: You really think we were set up? Mr. Pink: Do you even doubt it, man? I dont THINK we got set up, I KNOW we got set up! I mean, really, seriously, where did all those cops come from, huh? One minute theyre not there, the next minute theyre there? I didnt hear any sirens. The alarm went off, okay. Okay, when an alarm goes off, you got an average of four minutes response time. Unless a patrol car is cruising that street, at that particular moment, you got four minutes before they can realistically respond. In one minute there were seventeen blue boys out there. All loaded for bear, all knowing exactly what the fuck they were doing, and they were all just there! Remember that second wave that showed up in the cars? Those were the ones responding to the alarm, but those first motherfuckers were already there, they were waiting for us. Havent you fucking thought about this?  Mr. Pink: [Mr. Blonde and Mr. White begin to quarrel; Mr. Pink pushes them away from each other] Hey, you two assholes knock it the fuck off and calm down! Cut the bullshit. We aint on a fuckin playground! I dont beleive this shit. Both of you guys got ten years on me and Im the only one acting like a professional. You guys act like a bunch of fuckin niggers. You wanna be niggers, huh? Theyre just like you two - always fightin and always sayin their gonna kill each other... Mr. White: You said yourself you thought about takin him out! Mr. Blonde: You fuckin said that? Mr. Pink: Yeah, I did. But that time has passed. Right now, Mr. Blonde is the only one I completely trust. Hes too fuckin homicidal to be workin with the cops. Mr. White: You takin his side? Mr. Pink: No, man. Fuck sides! What we need here is a little solidarity! Somebodys shoving a red hot poker up our asses and I wanna find out whose name is on the handle. Now, I know Im no piece of shit. [referring to Mr. White] Mr. Pink: And Im pretty sure youre a good boy. [referring to Mr. Blonde] Mr. Pink: And Im fucking positive youre on the level. So lets figure out who the bad guy is.  Mr. Orange: What happens if the manager wont give you the diamonds? Mr. White: When youre dealing with a store like this, theyre insured up the ass. Theyre not supposed to give you any resistance whatsoever. If you get a customer, or an employee, who thinks hes Charles Bronson, take the butt of your gun and smash their nose in. Everybody jumps. He falls down screaming, blood squirts out of his nose, nobody says fucking shit after that. You might get some bitch talk shit to you, but give her a look like youre gonna smash her in the face next, watch her shut the fuck up. Now if its a manager, thats a different story. Managers know better than to fuck around, so if you get one thats giving you static, he probably thinks hes a real cowboy, so you gotta break that son of a bitch in two. If you wanna know something and he wont tell you, cut off one of his fingers. The little one. Then tell him his thumbs next. After that hell tell you if he wears ladies underwear. Im hungry. Lets get a taco. Joe: So, you guys like to tell jokes, huh? Gigglin and laughin like a bunch of young broads sittin in a schoolyard. Well, let me tell a joke. Five guys, sittin in a bullpen, in San Quentin. All wondering how the fuck they got there. What should we have done, what didnt we do, whos fault is it, is it my fault, your fault, his fault, all that bullshit. Then one of them says, hey. Wait a minute. When we were planning this caper, all we did was sit around tellin fuckin jokes! Get the message? Boys, I dont mean to holler at ya. When this capers over - and Im sure itll be a successful one - well get down to the Hawaiian Islands, hell, Ill roll and laugh with all of ya. Youll find me a different character down there. Right now, its a matter of business.  Joe: This man set us up. Nice Guy Eddie: Dad, Im sorry, but I dont know what the hells happening Joe: Its all right, Eddie. I do Mr. White: What the fuck are you talking about Joe: That lump of shits working with the L.A.P.D Mr. Orange: Joe, I dont have the slightest fucking idea what youre talking about Mr. White: Joe, I dont know what you think you know, but youre wrong Joe: Like hell I am Mr. White: Joe, trust me on this. Youve made a mistake. Hes a good kid. I understand. Youre hot, youre super fucking pissed. Were all real emotional. But youre barking up the wrong tree. I know this man. He wouldnt do that Joe: You dont know jack shit! I do! The cocksucker tipped off the cops and had Mr. Brown and Mr. Blue killed Mr. Pink: Mr. Blue is dead Joe: Dead as Dillinger Mr. White: How do you know all this? Mr. Pink: Im very sorry the government taxes their tips, thats fucked up. That aint my fault. It would seem to me that waitresses are one of the many groups the government fucks in the ass on a regular basis. Look, if you ask me to sign something that says the government shouldnt do that, Ill sign it, put it to a vote, Ill vote for it, but what I wont do is play ball. And as for this non-college bullshit I got two words for that: learn to fuckin type, cause if youre expecting me to help out with the rent youre in for a big fuckin surprise.  Nice Guy Eddie: Yeah Im sorry, I should have picked you up myself. This whole week has been fucked up, Ive had my head up my ass the whole time. Mr. Blonde: You know, thats funny, cause thats what me and your dad were just talkin about Nice Guy Eddie: That I should have picked you up Mr. Blonde: No, that you got your head up your ass. I mean I walked in here and Joes like, Vic, thank god youre back. My son Eddies a fuck-up. I mean I love the guy, but hes just flushin everything down the toilet. Hes ruining me. I mean thats what you said, right Joe? Im not makin this up Joe: Im sorry you had to hear it like this Eddie. Vic asked me how business was, and you cant lie to a man whos done time for you Nice Guy Eddie: Thats very true [he and Vic get into a wrestling contest] Mr. Pink: We were set up. The cops were waiting for us. Nice Guy Eddie: What? Nobody set anybody up Mr. Pink: The cops were there waiting for us Nice Guy Eddie: Bull shit Mr. Pink: Hey, fuck you, man! You werent there - we were! And Im tellin ya, the cops had that store staked out Nice Guy Eddie: Okay, Mr. Fucking Detective! Youre so fucking smart. Who did it? Who set us up";

const char *testBody43p2 = " Mr. Pink: What the fuck dya think weve been askin each other Nice Guy Eddie: And what are your answers? Was it me? You think I set you up Mr. Pink: I dont know, but somebody did Nice Guy Eddie: Nobody did! You assholes turn the jewelry store into a wild west show, and you wonder why the cops show up?  Mr. White: [fighting over what to do with the dying Mr. Orange] If I have to tell you again to back off, you an me are gonna go round and round. Mr. Pink: We aint taking him to a hospital Mr. White: If we dont, hell die Mr. Pink: And Im very sad about that, but some fellas are lucky, and some aint Mr. White: [shoving Mr. Pink] That fuckin did it Mr. Pink: Dont you fuckin touch me, man Mr. White: [punches Mr. Pink. He falls and Mr. White proceeds to kick him across the floor] You little motherfucker Mr. Pink: [rolling over and pulling out a gun] You wanna fuck with me? Ill show ya who youre fuckin with Mr. White: [aiming his gun at Mr. Pinks face] You wanna shoot me, you little piece of shit? Take a shot Mr. Pink: Fuck you, White! I didnt create the situation, Im just dealin with it! Youre acting like a first year fucking theif - Im acting like a professional! If they get him, they can get you. They get you, they get closer to me, and that cant happen! And you, motherfucker, are lookin at me like its MY fault. I didnt tell him my name. I didnt tell him where I was from. I didnt tell him what I knew better than NOT to tell him! Fuck, fifteen minutes ago you almost told me your name! You, buddy, are stuck in a situation YOU created. So, if you wanna throw bad looks somewhere, throw em at a mirror!";

extern int bodyServed;