>speech_prog vampire guardian light dark limbo~
mpecho The guardian vampire shields his face from your light and says 'Bring a brighter flame or a quieter death. I am equipped for either.'
~
>speech_prog gate danger law guard trouble city recall~
if rand(14)
mpecho $I says 'Keep your weapon down unless you want the street to remember you poorly.'
else
if rand(17)
mpecho $I says 'If you get lost, recall. If recall fails, you picked the wrong room or the wrong curse.'
else
if rand(20)
mpecho $I says 'Gates let trouble in. I'm the part that decides how it leaves.'
else
if rand(25)
mpecho $I says 'I've stood at this post so long I've named the cracks in the wall. That one's Gerald. Don't touch Gerald.'
else
if rand(33)
mpecho $I says 'Crime's down this month. Mostly because the criminals keep wandering into things with teeth.'
else
if rand(50)
mpecho $I says 'You look suspicious. Everyone looks suspicious. That's the job. Move along.'
else
mpecho $I adjusts a halberd that has clearly never needed sharpening and never will.
endif
endif
endif
endif
endif
endif
~
|
