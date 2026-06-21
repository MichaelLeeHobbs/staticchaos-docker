>speech_prog citizen help work child town news farm cow~
if rand(50)
mpecho The citizen smiles and says 'We farm, we pray, we watch the Citadel and hope it stays asleep.'
else
mpecho The page says 'I carry messages between people who could kill each other faster than I run. Good work if you live.'
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
