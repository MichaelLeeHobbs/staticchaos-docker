>give_prog foot hamster doom component~
if ispc($n)
  if carries($n) == 3153
    mpjunk all.foot
    mpechoat $n Berthold's eyes light up at the second grisly foot.
    say Aha -- the second foot!  Now hand me back that unfinished slipper
    say and I'll fit the pair together at last.
    mpforce $n give token berthold
  else
    mpjunk all.foot
    mpechoat $n Berthold turns the grisly little foot over in his hands, nodding.
    say Aye -- a foot off the Hamster of Doom that haunts the market square!
    say That's the first component I need.  Here, I've started one slipper.
    mpoload 3153
    give token $n
    say A pair needs two feet, friend.  Slay the Hamster again, bring me a
    say second foot, and I'll finish the job.
  endif
endif
~
>give_prog token unfinished receipt hamster~
if ispc($n)
  mpjunk all.token
  mpechoat $n Berthold stitches, hammers, and waxes with astonishing speed.
  say There -- a matched pair, padded with the softest hamster fur!
  mpoload 3152
  give doomslippers $n
  say Wear them in good health, and watch where you tread.
endif
~
>give_prog all~
if ispc($n)
  say I've no use for that.  Bring me a foot of the Hamster of Doom -- the
  say cute little beast that haunts the market square -- if you dare.
  mpechoaround $n Berthold politely hands the item straight back to $n.
  give all $n
endif
~
>greet_prog 100~
if ispc($n)
  say Welcome to my workshop!  They say the Hamster on the square is harmless...
  say but those who 'mess with it' rarely live to tell.  Slay it twice, bring
  say me two of its feet, and I'll craft you slippers fit for a champion.
endif
~
|
