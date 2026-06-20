>give_prog foot rabbit lucky tuft~
if ispc($n)
  mpjunk all.foot
  mpechoat $n Berthold turns the grisly little foot over in his hands, nodding.
  say Aye -- a foot off the beast of Caerbannog, no mistake!  That's the
  say first component I need.  Here, I've already started shaping a slipper.
  mpoload 3153
  give slipper $n
  say Now bring that half-finished slipper back to me for the second fitting
  say and I'll turn it into a proper pair of leaping-slippers.
endif
~
>give_prog slipper rabbit half unfinished~
if ispc($n)
  mpjunk all.slipper
  mpechoat $n Berthold stitches, hammers, and waxes with astonishing speed.
  say There -- the second fitting's done.  A matched pair, light as a hare!
  mpoload 3152
  give slippers $n
  say Wear them in good health, and try not to bounce off the walls.
endif
~
>give_prog all~
if ispc($n)
  say I've no use for that.  Bring me a foot of the killer rabbit -- the
  say cute little beast that haunts the market square -- if you dare.
  mpechoaround $n Berthold politely hands the item straight back to $n.
  give all $n
endif
~
>greet_prog 100~
if ispc($n)
  say Welcome to my workshop!  They say the rabbit on the square is harmless...
  say but those who 'mess with it' rarely live to tell.  Slay it, bring me
  say its foot, and I'll craft you slippers that fairly fly.
endif
~
|
