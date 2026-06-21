>speech_prog warg wolf rabbit bear beast danger~
if rand(50)
mpecho The ferocious rabbit glares at you with murder in its tiny eyes. Yes. The rabbit. Do not laugh.
else
mpecho A warg lifts its head and decides whether you are a threat or a meal. It is not a long deliberation.
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
