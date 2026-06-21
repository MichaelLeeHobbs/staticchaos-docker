>speech_prog horse smithy smith talk clue hint hay~
if rand(50)
mpecho Mr. Smithy the talking horse chews his hay and says 'A talking horse who gives good advice, and still nobody listens. Britannia in one stable, friend.'
else
mpecho Mr. Smithy says 'Want the famous hint? Mind thy virtues, watch thy back, and never trust a chest in a room full of clever-looking junk.'
endif
~
>speech_prog pet follow good hungry name~
if rand(20)
mpecho $I looks at you with the deep, patient calm of something that will outlive your character.
else
if rand(25)
mpecho $I makes a noise that is either affection or a threat. With pets, that line is thin.
else
if rand(33)
mpecho $I wags, purrs, or chitters, fully aware it has no idea what you said and no plans to learn.
else
if rand(50)
mpecho $I sniffs your boots, judges your entire life from the smell, and walks off unimpressed.
else
mpecho $I stares at a spot just past your head. Whatever it sees there, you'll be glad you can't.
endif
endif
endif
endif
~
|
