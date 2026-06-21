>speech_prog monster beast practice kill target blob snail~
if rand(50)
mpecho The leashed monster exists so you can learn to swing without consequences. Savor it; consequences are the default elsewhere.
else
mpecho The snail is trying to get out of your way. Even the tutorial thinks you should pick a real target.
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
