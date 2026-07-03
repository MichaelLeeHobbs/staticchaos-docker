# Static Chaos NPC Keyword Dialogue Pack

Generated for adding non-hostile NPC reactions to `say` / `yell` keyword triggers.

Sources used:
- `BESTIARY.md` generated from the Static Chaos area files: 836 creatures across 44 areas.
- `world-graph.json` generated from `area/area.lst`: 2500 rooms, 5831 directed walk edges, recall defaults to room `3001` (The Temple of Midgaard); no speech-triggered transfers were auto-extracted.
- This file is a design/implementation spec, not an area-file patch.

Every spoken line here is original flavor written to match the existing Static Chaos voice. Where an NPC comes from referenced source material (Britannia/Ultima, the gods of Olympus, the zodiac in Galaxy, the Tolkien-flavored Shire and Moria), the *content* of the line is kept faithful to that lore, but no source dialogue is reproduced. Keep it that way if you extend this file.

## Implementation Intent

Static Chaos already has a blunt, jokey, slightly hostile MUD tone. These NPC replies should feel like old help-room flavor and mobprog hints, not modern quest markers.

Use this as a data pack for MOBProgs or a new speech trigger layer:

```yaml
speech_trigger:
  listen_to: [say, yell]
  match: keyword_any
  cooldown_player: 60s
  cooldown_room: 20s
  chance: 75
  require:
    actor_not_fighting: true
    actor_not_aggressive: true
    actor_can_see_speaker: false
  response_mode: say
```

### Matching rules

- Lowercase and strip punctuation from player speech.
- Match whole words or configured aliases.
- `say` should normally trigger only NPCs in the same room.
- `yell` may trigger same-area NPCs only if you intentionally want area hints. Use a stricter cooldown for `yell`.
- Do not trigger while the NPC is fighting.
- Do not let an NPC reveal exact hidden mechanics it should not know.
- Prefer short responses: 1 sentence, rarely 2.
- For repeated queries, rotate variants or fall back to: `"I already told you, pay attention."`
- A keyword that names the NPC's own area or a neighbor it would plausibly know about is fair game. A keyword that names a faraway boss it has no business knowing should fall through to flavor, not produce a spoiler.
- If multiple keyword groups match one line of speech, prefer the most specific group (a named NPC over a generic alias).

### Response action format

Use one of these styles:

```yaml
- vnum: 3012
  actor: healer
  area: midgaard
  keywords: [heal, poison, curse, blind]
  response: "The healer says 'Rest if you can, flee if you must, and don't come crying to me while cursed.'"
```

or grouped:

```yaml
- actor_group: midgaard_shopkeepers
  vnums: [3001, 3002, 3003, 3004, 3009, 3010]
  response_sets:
    gear:
      keywords: [gear, equipment, weapon, armor, armour]
      responses:
        - "The shopkeeper says 'Hitroll, damroll, saves, AC, hp, mana, move. That's what gear does. The rest is pretty lies.'"
```

## Verified World Facts (for accurate hints)

These are confirmed from `world-graph.json`. Hint-writers should not contradict them.

```yaml
world_facts:
  recall_room: 3001            # The Temple of Midgaard; default recall target
  recall_blocked_by: [no_recall_flag, curse, fighting]
  need_fly_to_enter:           # rooms in sector "air"
    - air        # the entire "In the Air" area is air-sector
    - olympus    # a couple of sky rooms
    - astral     # one air room
    - midgaard   # one air room
  need_boat_or_flight:         # rooms in sector "water_noswim"
    - ultima     # the most (around the isles)
    - midgaard   # the moat/river around the city
    - eastern    # desert oasis water
    - haven
    - wyvern
    - marsh
    - vacation
    - gnome
  heavy_no_recall_zones:       # bring a way out; recall will fail a lot
    - apoc       # Apocalypse HQ is almost entirely no-recall
    - hitower
    - gnome
    - eastern
    - mahntor
    - ultima
    # all clan HQs (cithdeux, divergent, renegades, teikoku, zrollers, malokteri)
  hq_rule: "Portals and gates cannot be used from or into a clan HQ; plan to walk out."
```

These let an NPC say true things like "bring a boat" or "recall won't save you here" without lying to the player.

## Global Role Templates

These apply to similar non-hostile NPCs even if not listed individually below. Each pool is a grab-bag — the engine picks one at weighted random, so the more lines a role has, the less any single NPC repeats itself. Keep adding to these freely; the templates are where cheap flavor lives.

### Shopkeepers / merchants

```yaml
keywords: [buy, sell, list, value, shop, wares, gear, equipment]
responses:
  - "$n says 'Type list before you drool on the counter.'"
  - "$n says 'Gear helps your body. Training makes you dangerous. Coin helps me. Mostly the last one.'"
  - "$n says 'If it says strength or wisdom on the tag, ask an immortal whether the world has caught up yet.'"
  - "$n says 'I buy cheap and sell honest. The honest part is the one that hurts.'"
  - "$n says 'No, I will not haggle. Yes, I will watch you try. It's the highlight of my day.'"
  - "$n says 'Browsing is free. Touching is a deposit. Bleeding on the merchandise is a purchase.'"
  - "$n says 'Everything here has killed at least one previous owner. Adds character, I find.'"
  - "$n eyes your inventory the way a vulture eyes a slow horse."
```

### Trainers / practice mobs

```yaml
keywords: [train, practice, skill, spell, class, primal, experience]
responses:
  - "$n says 'Practice is cheap. Power is not. Guess which one you're short on.'"
  - "$n says 'Don't ask me to make you strong if you won't go get blood on your boots.'"
  - "$n says 'Primal buys the trick. Sweat buys the timing. You bring the sweat.'"
  - "$n says 'I've trained heroes and idiots. The paperwork is identical. So are the funerals, usually.'"
  - "$n says 'Swing it again. No — worse than that. There. That's your natural talent showing.'"
  - "$n says 'You can practice that until your fingers bleed, or you can do it right. Pick one, I'm patient.'"
  - "$n sighs the sigh of a teacher who has seen this exact mistake nine thousand times."
```

### Guards

```yaml
keywords: [gate, danger, law, guard, trouble, city, recall]
responses:
  - "$n says 'Keep your weapon down unless you want the street to remember you poorly.'"
  - "$n says 'If you get lost, recall. If recall fails, you picked the wrong room or the wrong curse.'"
  - "$n says 'Gates let trouble in. I'm the part that decides how it leaves.'"
  - "$n says 'I've stood at this post so long I've named the cracks in the wall. That one's Gerald. Don't touch Gerald.'"
  - "$n says 'Crime's down this month. Mostly because the criminals keep wandering into things with teeth.'"
  - "$n says 'You look suspicious. Everyone looks suspicious. That's the job. Move along.'"
  - "$n adjusts a halberd that has clearly never needed sharpening and never will."
```

### Healers / priests

```yaml
keywords: [heal, cure, poison, blind, curse, sanctuary, bless, recall]
responses:
  - "$n says 'A blessing is nice. Sanctuary is better. Not dying is best.'"
  - "$n says 'Curse locks the road home. Remove it before you start praying at the altar.'"
  - "$n says 'I mend what swords open. I cannot mend what stupidity opens. Stop testing me.'"
  - "$n says 'You again. At this rate I should just follow you around and skip the middle part.'"
  - "$n says 'Sit still. Bleeding is rude in a temple, and you're being very rude.'"
  - "$n says 'I can cure poison, blindness, and disease. I cannot cure whatever made you walk in there.'"
  - "$n says 'The gods love a trier. They love a survivor more. Try to be the second one.'"
```

### Prisoners / slaves

```yaml
keywords: [escape, guard, boss, key, help, way, exit]
responses:
  - "$n whispers 'The loud ones know less than the beaten ones. Watch the guards, not their mouths.'"
  - "$n whispers 'If a room hates recall, believe it. I learned that the slow way.'"
  - "$n whispers 'Free me or don't, but if you free the wrong thing, run first and apologize never.'"
  - "$n whispers 'I've counted the guard's steps for a year. He limps on the left. Make of that what you will.'"
  - "$n whispers 'They keep the real keys on the one who never speaks. Funny how that works.'"
  - "$n whispers 'Hope is a luxury down here. So is a name. Take what I tell you and go.'"
```

### Bards / gossips

```yaml
keywords: [rumor, gossip, song, hero, boss, treasure]
responses:
  - "$n says 'Every treasure has an owner. Every owner has teeth.'"
  - "$n says 'Ask around enough and even idiots become a map.'"
  - "$n sings 'Heroes are just survivors with better press.'"
  - "$n says 'I'd tell you the rumor, but rumors are like ale — first round's the only honest one.'"
  - "$n says 'They say the boss has a weakness. They always say that. Sometimes they're even alive to say it.'"
  - "$n leans in. 'I heard a thing. It's probably a lie. The best ones usually are.'"
  - "$n says 'A song about your deeds? Sure. I'll need to know you survived them first. Awkward, I know.'"
```

### Innkeepers / bartenders / waitstaff

```yaml
keywords: [rest, sleep, room, drink, food, rumor, inn, tavern]
responses:
  - "$n says 'Rest before the road. The road never rests before you.'"
  - "$n says 'Drink, sleep, or leave. Bleeding on the floor is none of those.'"
  - "$n says 'Heroes who forget to stand up before walking out die in my doorway. Don't.'"
  - "$n says 'The stew is brown. Don't ask what kind of brown. The brave don't ask.'"
  - "$n says 'Room's upstairs, ale's downstairs, regret's wherever you left it. Enjoy your stay.'"
  - "$n says 'I've heard every adventurer's life story twice. Tip well and I'll pretend yours is new.'"
  - "$n polishes a mug that was clean an hour ago and will be clean an hour from now."
```

### Druids / hermits / herbalists

```yaml
keywords: [herb, forest, nature, path, woods, heal, hermit]
responses:
  - "$n says 'Herbs grow where blood has not yet ruined the soil. That narrows it down. Barely.'"
  - "$n says 'If the trees go quiet, something is hunting. Probably you.'"
  - "$n says 'I left the cities because people there talk too much. And here you are. Talking.'"
  - "$n says 'Nature is balance. The bear eats you, the worms eat the bear, I eat neither and live longer.'"
  - "$n says 'Lost? The path's right there. It's always right there. People just stop looking down.'"
  - "$n mutters to a plant, listens to its answer, and seems more satisfied with that conversation than this one."
```

### Civilians / children / townsfolk

```yaml
keywords: [help, town, city, lost, work, news]
responses:
  - "$n says 'I just live here. The dying is done by people like you.'"
  - "$n shrugs and points vaguely at whatever is most likely to kill you."
  - "$n says 'News? A hero came through last week. We're still cleaning up the news.'"
  - "$n says 'I'd help, but I have a job, a family, and a strong commitment to staying boring and alive.'"
  - "$n says 'You adventurers are great for the economy and terrible for the windows.'"
  - "$n eyes your blood-stained gear and very deliberately crosses to the other side of the street."
```

### Animals / pets (flavor only, tier 0)

```yaml
keywords: [pet, follow, good, hungry, name]
responses:
  - "$n looks at you with the deep, patient calm of something that will outlive your character."
  - "$n makes a noise that is either affection or a threat. With pets, that line is thin."
  - "$n wags, purrs, or chitters, fully aware it has no idea what you said and no plans to learn."
  - "$n sniffs your boots, judges your entire life from the smell, and walks off unimpressed."
  - "$n stares at a spot just past your head. Whatever it sees there, you'll be glad you can't."
```

### Hostile boss bark template (opt-in; see Boss Taunts section)

```yaml
# Only fires if you deliberately override actor_not_aggressive for this vnum.
keywords: [name, fight, die, who]
responses:
  - "$n says 'You said my name like it was a spell. It isn't. It's a warning.'"
  - "$n says 'Talk while you can. The talking part of you has the shortest future here.'"
```

## Universal Chatter Pool

This is the big lever. Any **non-hostile** NPC can fall back on these when the player says something common that isn't covered by an area- or role-specific keyword. It's how the whole world feels alive without writing a custom line for all 800 mobs: a greeting, an insult, or a fourth-wall poke gets a reply from *anyone*. Matching priority is area-specific → role template → universal pool, so these only fire when nothing more specific catches. Keep them tier 0 — pure flavor, never hints.

```yaml
area: _universal
chatter:
  - group: greeting
    keywords: [hi, hello, hey, greetings, yo, sup, hail, howdy]
    responses:
      - "$n says 'Hello yourself. That's the warm part of this conversation over with.'"
      - "$n says 'Greetings, stranger. You'll be a stranger right up until you owe me money or a funeral.'"
      - "$n nods the exact amount of nod required by law and no more."
      - "$n says 'Hi. Yes. Hello. We've done the pleasant part. What do you actually want?'"

  - group: farewell
    keywords: [bye, later, goodbye, cya, farewell, leaving, gtg]
    responses:
      - "$n says 'Leaving already? Try to do it in one piece. It's harder than it sounds around here.'"
      - "$n says 'Goodbye. If you come back through still breathing, I'll be mildly impressed.'"
      - "$n waves you off with the energy of someone who has waved off a great many people who never returned."

  - group: gratitude
    keywords: [thanks, thank, ty, cheers, appreciate, grateful]
    responses:
      - "$n says 'Thanks don't spend. But I'll allow it, just this once.'"
      - "$n says 'You're welcome. Now go be useful somewhere that isn't directly in front of me.'"
      - "$n says 'Gratitude. From an adventurer. I'll have this moment mounted on the wall.'"

  - group: apology
    keywords: [sorry, oops, apologies, mybad, pardon]
    responses:
      - "$n says 'Sorry doesn't unbreak the vase, the window, or the previous hero. But noted.'"
      - "$n says 'Apology accepted. Reluctantly. Provisionally. Watch yourself.'"
      - "$n says 'Oh, NOW you're sorry. The dramatic timing on you people, I swear.'"

  - group: identity
    keywords: [who, name, you, yourself, are]
    responses:
      - "$n says 'Who am I? I'm the one still standing here after watching a hundred of you not.'"
      - "$n says 'My name's my business. Yours is about to be on a gravestone if you keep wandering like that.'"
      - "$n says 'I'm nobody important. That's why I'm alive to answer you. Think about it.'"

  - group: confusion_spam
    keywords: [what, huh, lol, lmao, haha, asdf, blah, qwerty, gibberish]
    responses:
      - "$n says 'I have no idea what that was, and I refuse to learn.'"
      - "$n says 'Was that a word? In this realm? Bold of you.'"
      - "$n stares at you the way you'd stare at a dog that started reciting poetry."
      - "$n says 'Laugh all you want. The monsters down the road laugh too, and theirs is worse.'"

  - group: compliment
    keywords: [nice, cool, awesome, love, great, amazing, beautiful]
    responses:
      - "$n says 'Flattery. From a stranger covered in someone else's blood. How wholesome.'"
      - "$n says 'Yes, I am wonderful. It changes nothing about your situation, but yes.'"
      - "$n almost smiles, catches itself, and returns to a professional scowl."

  - group: insult
    keywords: [stupid, idiot, ugly, suck, noob, dumb, loser, trash]
    responses:
      - "$n says 'Says the hero who's been lost in the same three rooms for ten minutes.'"
      - "$n says 'Insults? From you? That's the bravest thing you've done all day, and it's a low bar.'"
      - "$n says 'I've been called worse by better, and I outlived all of them. Care to apply?'"
      - "$n says 'Big words from a creature with a respawn timer.'"

  - group: meta_fourthwall
    keywords: [mud, game, lag, link, bug, immortal, imm, code, reboot, copyover]
    responses:
      - "$n says 'The immortals built this place. They also broke it, fixed it, and broke it again. We don't ask.'"
      - "$n says 'If the world freezes, that's not magic, that's lag. Stand very still and pray to the coders.'"
      - "$n says 'Found a bug? Tell an immortal. They love that. They love it SO much.'"
      - "$n glances upward as if listening to something far above the sky, then shrugs it off."

  - group: begging
    keywords: [gold, plat, coin, free, give, spare, donate, beg]
    responses:
      - "$n says 'Spare gold? Adventurer, you're carrying a sword worth more than my house.'"
      - "$n says 'I don't give handouts. The monsters give handouts — handfuls, mostly your own.'"
      - "$n says 'You want free? Go to the newbie zones. Out here, everything bites back for payment.'"

  - group: flirt
    keywords: [marry, love, kiss, date, cute, hot, single]
    responses:
      - "$n says 'I've outlived three spouses and a continent. Try someone with a shorter memory.'"
      - "$n says 'Charming. The last person who flirted with me also tried to rob me. Funny coincidence.'"
      - "$n says 'No. But I admire the confidence of a person actively on fire asking me out.'"

  - group: threat
    keywords: [kill, fight, attack, die, murder, destroy]
    responses:
      - "$n says 'Threaten me? I'm not even the scary thing in this room, and I'm definitely scarier than you.'"
      - "$n says 'Go ahead. The guards are bored, the gods are watching, and I'm insured.'"
      - "$n says 'You want a fight? The whole realm is a fight. Walk twenty feet, it'll find you.'"

  - group: idle_ambient
    keywords: []   # no trigger; fire rarely on a long timer for background life
    responses:
      - "$n mutters something about the price of bread and the quality of heroes, both declining."
      - "$n watches the road with the tired patience of someone who has seen every kind of arrival and most kinds of departure."
      - "$n yawns, scratches, and continues the eternal business of not being the protagonist."
```


---

# Area Dialogue Packs

## Midgaard — city hub, guilds, shops, training

Midgaard has the most non-hostile NPCs in the game (shopkeepers, guildmasters, guards, waiters, civic folk, and pets). It is the right place for newbie reminders, class hints, equipment truth, recall/survival advice, and harmless flavor. Recall lands here, at the Temple (`3001`).

```yaml
area: midgaard
entries:
  - vnum: 3012
    actor: the healer
    keywords: [heal, poison, blind, curse, sanctuary, bless, recall, death, cure]
    responses:
      - "The healer says 'Curse and no-recall rooms make cowards into corpses. Fix the curse, then recall.'"
      - "The healer says 'Sanctuary first. Armor after. Style last.'"
      - "The healer says 'If you can still flee, you are not dead enough to be brave.'"
      - "The healer says 'Blind men miss. Poisoned men rot. Pick which you'd rather cure and pay me.'"

  - vnums: [3020, 3021, 3022, 3023]
    actor: guildmaster
    keywords: [guild, class, train, practice, primal, power, learn, level]
    responses:
      - "The guildmaster says 'Your class is not in your boots. Train your real powers.'"
      - "The guildmaster says 'Primal buys the tricks. Experience buys the scars.'"
      - "The guildmaster says 'Ask for help after you read your commands, not before.'"
      - "The guildmaster says 'A level is a number. Surviving the next one is the homework.'"

  - vnums: [3024, 3025, 3026, 3027]
    actor: class_gate_guards
    keywords: [sorcerer, templar, assassin, knight, guild, entrance, join]
    responses:
      - "The sorcerer says 'Magic is patience with a temper. The guild is behind me; your aptitude is your problem.'"
      - "The knight templar says 'We let in the faithful and the useful. Be at least one.'"
      - "The assassin says 'If you have to ask where the door is, you are not ready to walk through it.'"
      - "The knight says 'Honor opens this gate. Bluster bounces off it.'"

  - vnum: 3000
    actor: the wizard
    keywords: [magic, vortex, portal, recall, area, lost, where, transport]
    responses:
      - "The wizard says 'The Vortex is not a toy. Then again, neither are you. Probably.'"
      - "The wizard says 'Recall pulls you home unless the room, the curse, or your own stupidity says otherwise.'"
      - "The wizard says 'You want a portal? Portals refuse to open into a clan house. The clans like it that way.'"

  - vnum: 3011
    actor: the vortex guardian
    keywords: [vortex, guardian, portal, gate, travel]
    responses:
      - "The Guardian of the Vortex growls 'I guard the way between. Touch it wrong and you become a cautionary tale.'"
      - "The Guardian of the Vortex says 'Everyone wants the shortcut. Few survive arriving early.'"

  - vnums: [3003, 3004]
    actor: weaponsmith_and_armourer
    keywords: [weapon, armor, armour, gear, hitroll, damroll, ac, saves, smith]
    responses:
      - "The smith says 'Hitroll finds the hole. Damroll makes it hurt. AC keeps you from being the hole.'"
      - "The smith says 'Pretty numbers on dead stats don't win fights here. Useful gear says hp, mana, move, AC, hit, dam, or saves.'"
      - "The armourer says 'Buy the plate, then learn to move in it. Most people skip the second part.'"

  - vnums: [3001, 3002, 3009, 3010, 3100]
    actor: midgaard_shopkeepers
    keywords: [buy, sell, list, value, shop, bread, food, jewel, leather, gold]
    responses:
      - "The baker says 'Bread keeps your move points up. Heroics on an empty stomach is just fainting with a sword.'"
      - "The grocer says 'List, then buy. I have watched men negotiate with a shelf.'"
      - "The jeweller says 'Shiny is not the same as enchanted. Identify before you swoon.'"
      - "The leather worker says 'Light armor for the quick, heavy for the brave, none for the briefly alive.'"
      - "The maid says 'I clean up after adventurers. You would not believe how little is usually left.'"

  - vnum: 3006
    actor: the captain
    keywords: [boat, water, ocean, travel, river, sail, moat]
    responses:
      - "The captain says 'Water that says no-swim wants a boat. Flight works too, if you feel like cheating honestly.'"
      - "The captain says 'The moat around this city has drowned more confident men than any dragon.'"

  - vnum: 3007
    actor: the sailor
    keywords: [train, boat, sea, water, travel, swim]
    responses:
      - "The sailor says 'Train before the tide teaches you.'"
      - "The sailor says 'A boat is cheaper than drowning, most days.'"

  - vnum: 3008
    actor: the pet shop boy
    keywords: [pet, dragon, companion, follow, buy, charm]
    responses:
      - "The pet shop boy says 'Pets follow until they don't. Try not to feed them to something legendary.'"
      - "The pet shop boy says 'Buy the order receipt, then give it to the shopkeeper. The pet is the easy part; remembering the steps is where heroes fail.'"

  - vnums: [3040, 3041, 3042, 3043, 3044, 3045, 3046]
    actor: inn_staff
    keywords: [drink, room, rumor, gossip, job, gold, ale, special]
    responses:
      - "The waiter says 'Rumor says half the heroes die because they forgot to stand up first.'"
      - "The bartender says 'If you're bleeding on the floor, buy a drink or leave a corpse. Either way, don't block the door.'"
      - "The old sorcerer-waiter says 'Chants are louder than courage. If someone starts one, interrupt it or leave.'"
      - "Filthy says 'You want the special? The special is whatever I have not been caught serving yet.'"

  - vnum: 3143
    actor: the mayor
    keywords: [city, law, midgaard, guard, trouble, mayor]
    responses:
      - "The mayor says 'Midgaard is safe until someone proves otherwise. Do not volunteer.'"
      - "The mayor shakes your hand and says 'Welcome. Statistically, please come back.'"

  - vnum: 3142
    actor: the secretary
    keywords: [mayor, office, law, paper, help]
    responses:
      - "The secretary says 'The mayor is busy being important. I am busy being useful. Guess which one helps you.'"

  - vnums: [3060, 3067, 3068, 3069, 3141]
    actor: cityguard
    keywords: [gate, guard, trouble, pk, danger, law, fight]
    responses:
      - "The cityguard says 'Trouble comes through gates and leaves through corpses.'"
      - "The cityguard says 'If you are looking for fair fights, you are in the wrong world.'"
      - "The cityguard says 'Draw steel in my street and find out how fast the law runs.'"

  - vnum: 3140
    actor: the captain of the guard
    keywords: [captain, guard, order, trouble, thief]
    responses:
      - "The captain of the guard says 'Something's gone missing again. It is always something missing, and always the same kind of face asking about it.'"

  - vnum: 3145
    actor: Abashi
    keywords: [abashi, teleport, trans, travel, summon, transport]
    responses:
      - "Abashi says 'You want a trans? Stand still, stop yelling, and try not to land somewhere with teeth.'"
      - "Abashi mutters 'Everyone wants a free ride and nobody wants to know where the wheels have been.'"

  - vnum: 3050
    actor: Aod the Dealer
    keywords: [deal, aod, buy, dealer, shadow, special]
    responses:
      - "Aod the Dealer says from the shadows 'I have what the shops won't stock. The price is the part you'll remember.'"

  - vnums: [3063, 3005]
    actor: midgaard_lowlifes
    keywords: [thief, steal, vagabond, gold, coin, purse]
    responses:
      - "The vagabond says 'Keep your hand on your purse. Keep your eyes on mine while you're at it.'"
      - "The thief, all in black, says nothing useful and a great deal that is flattering. Watch your gold."

  - vnums: [3064, 3061, 3065, 3144, 3120, 3066]
    actor: midgaard_streetfolk
    keywords: [help, drunk, beggar, news, lost, work, clean]
    responses:
      - "The drunk sings something about heroes and hiccups, then loses the thread entirely."
      - "The beggar says 'A coin buys a blessing. A boot buys nothing but my opinion of you.'"
      - "The town crier weeps and says 'Bad news travels free. The good kind charges admission.'"
      - "The sexton says 'I keep the temple tidy and the dead organized. You will meet the second service eventually.'"

  - vnums: [3090, 3091, 3092, 3093, 3094, 3121, 3122, 3123, 3124]
    actor: midgaard_animals
    keywords: [pet, dog, cat, duck, swan, follow, good, name]
    responses:
      - "The puppy looks up at you with the unconditional trust of something you will probably get killed."
      - "The wolf watches you the way a professional watches an amateur."
      - "The duck quacks. It is, against all odds, the most honest reply you will get in this city."
```

## Mud School — tutorial area

```yaml
area: school
entries:
  - vnum: 3718
    actor: the adept of Furey
    keywords: [train, newbie, practice, school, commands, help, furey]
    responses:
      - "The adept of Furey says 'Look, exits, kill, flee, recall. Learn those before you learn dying.'"
      - "The adept of Furey says 'If a command fails, stop repeating it like a drunk parrot.'"
      - "The adept of Furey says 'Practice your skills here where the monsters are leashed. Out there, nothing is.'"

  - vnum: 3719
    actor: the priest of Hatchet
    keywords: [practice, spell, skill, level, death, hatchet, pray]
    responses:
      - "The priest of Hatchet says 'Practice what you can. Save when you should. Quit when you must.'"
      - "The priest of Hatchet says 'Hatchet watches. He is not impressed, but he watches.'"

  - vnums: [3700, 3707, 3717]
    actor: school_adepts
    keywords: [frag, kahn, furey, school, diploma, beast, graduate, leave, sell]
    responses:
      - "The adept of Frag says 'The diploma beast does not care about your feelings. This is an important lesson.'"
      - "The adept of Kahn says 'Graduation is just a nicer word for being thrown outside.'"
      - "The adept of Frag says 'Buy your starting gear here. The world charges extra for being unprepared.'"

  - vnum: 3720
    actor: the diploma beast
    keywords: [diploma, graduate, beast, present, leave, exit]
    responses:
      - "The diploma beast clutches your graduation present and grins. Apparently you have to take it from him. Welcome to everything else."

  - vnums: [3701, 3702, 3703, 3704, 3705, 3706, 3709, 3710, 3711, 3713, 3714, 3715, 3716]
    actor: school_practice_mobs
    keywords: [monster, beast, practice, kill, target, blob, snail]
    responses:
      - "The leashed monster exists so you can learn to swing without consequences. Savor it; consequences are the default elsewhere."
      - "The snail is trying to get out of your way. Even the tutorial thinks you should pick a real target."
```

## Plains of the North — road, hermits, druids, Ofcol approach

```yaml
area: plains
entries:
  - vnum: 301
    actor: Sorbus the Hermit
    keywords: [hermit, woods, stones, gharne, tunnel, rabbit, path, worm]
    responses:
      - "Sorbus says 'The old stones hum when fools walk near them. Follow the path and keep a light handy.'"
      - "Sorbus says 'The rabbit has better survival instincts than most warriors. Watch where it refuses to go.'"
      - "Sorbus says 'Something digs under G'harne. When the ground complains, it has earned the right.'"

  - vnum: 300
    actor: Aruncus the Druid
    keywords: [herbs, forest, path, heal, nature, gnarled, druid]
    responses:
      - "Aruncus says 'Herbs grow where blood has not yet ruined the soil. That narrows the search.'"
      - "Aruncus says 'If the trees go quiet, something is hunting.'"

  - vnum: 306
    actor: Luxan
    keywords: [shop, buy, sell, ofcol, road, gear]
    responses:
      - "Luxan says 'Ofcol lies where the road stops pretending to be safe.'"
      - "Luxan says 'Buy before you brag. Corpses are terrible customers.'"

  - vnum: 308
    actor: the Innkeeper
    keywords: [rest, sleep, room, ofcol, road, inn]
    responses:
      - "The Innkeeper says 'Rest before the road. The road never rests before you.'"

  - vnum: 305
    actor: Shudde-M'ell
    keywords: [worm, gharne, treasure, dig, tunnel, ground, burrow]
    flavor_only: true   # aggressive boss; see Boss Taunts section before enabling
    responses:
      - "The ground shudders, and something vast and patient decides you are not worth surfacing for. Yet."

  - vnums: [303, 304, 309]
    actor: plains_locals
    keywords: [citizen, stranger, rabbit, ofcol, news, help]
    responses:
      - "The citizen of Ofcol glares and says 'Stranger. We get your kind on the road. We bury most of them past the bend.'"
      - "The cute rabbit does nothing cute whatsoever and watches the treeline like it knows something."

  - vnum: 350
    actor: the pet dragon
    keywords: [dragon, pet, ravan, cute, wings]
    responses:
      - "Ravan's pet dragon flaps her cute little wings and looks at you with eyes that have watched gods get bored. Do not pet her."
```

## New Ofcol / Golden Citadel

```yaml
area: ofcol2
entries:
  - vnums: [600, 623, 634]
    actor: Ofcol cityguard
    keywords: [ofcol, guard, gate, citadel, trouble, dragon]
    responses:
      - "The Ofcol cityguard says 'The Golden Citadel is not a sightseeing tour. It is a test with walls.'"
      - "The Ofcol cityguard says 'If someone robed in gold tells you to kneel, check for dragons first.'"

  - vnums: [602, 603]
    actor: Ofcol captains
    keywords: [captain, guard, citadel, derrick, jacklyn, morale, order]
    responses:
      - "Captain Derrick says 'Discipline keeps the gate. Heroics lose it and then blame the gate.'"
      - "Captain Jacklyn says 'My soldiers' morale is fine. Yours is the one I'd worry about up those stairs.'"

  - vnum: 601
    actor: Marshall Diana
    keywords: [marshall, diana, ofcol, law, order, citadel]
    responses:
      - "Marshall Diana says 'I keep Ofcol standing. You are welcome to help by not falling down in the street.'"

  - vnum: 624
    actor: a bard
    keywords: [song, citadel, dragon, hero, ofcol, glory]
    responses:
      - "The bard sings 'Gold above, graves below, and heroes in between, mostly briefly.'"
      - "The bard says 'Ofcol's glory is easy to sing about. Surviving it is the difficult verse.'"

  - vnum: 629
    actor: Chaplain Jerrold
    keywords: [bless, citadel, dragonlord, faith, keeper, chaplain]
    responses:
      - "Chaplain Jerrold says 'Blessings help morale. Armor helps more. Having both is traditional.'"

  - vnum: 628
    actor: the Priestess of Ofcol
    keywords: [priestess, bless, heal, faith, ofcol, cure]
    responses:
      - "The Priestess of Ofcol smiles and says 'A blessing for the road. The road will try to spend it quickly.'"

  - vnums: [630, 631, 632, 633]
    actor: Golden Citadel attendants and Dragonknights
    keywords: [keeper, dragon, citadel, gold, lord, chamber, knight, scales]
    responses:
      - "The attendant says 'Speak softly in the Citadel. Gold remembers insults.'"
      - "The Dragonknight says 'The higher chambers are not for beggars, thieves, or the recently confident.'"
      - "The Dragonlord says 'I wear a dragon's scales so you remember what wore them first.'"
      - "The Ancient Gold Dragon stirs, decides you are beneath rousing for, and keeps one eye open anyway."

  - vnums: [604, 605, 606, 607, 608, 609, 622]
    actor: Ofcol merchants
    keywords: [buy, sell, leather, pawn, crop, food, gear, meat, blacksmith, ale]
    responses:
      - "Jim the Blacksmith says 'The city buys everything except excuses. Bring me a broken weapon or a full purse.'"
      - "Madam Tracy says 'Little treasures, big prices. If you want hero rates, bring hero loot.'"
      - "Granny Jenkins says 'The still never closes and the still never lies. Everything else in this town does both.'"

  - vnums: [617, 618, 619, 620, 621, 625, 626, 627, 611, 613, 614]
    actor: ofcol_townsfolk
    keywords: [citizen, help, work, child, town, news, farm, cow]
    responses:
      - "The citizen smiles and says 'We farm, we pray, we watch the Citadel and hope it stays asleep.'"
      - "The page says 'I carry messages between people who could kill each other faster than I run. Good work if you live.'"
```

## Miden'nir — goblin country, roadside inn

```yaml
area: midennir
entries:
  - vnum: 3503
    actor: the Innkeeper
    keywords: [rest, room, inn, drink, rumor, goblin, road]
    responses:
      - "The Innkeeper polishes a glass and says 'Goblins on the high road, wyverns over it. Rest while you still have a here to rest in.'"

  - vnum: 3504
    actor: the bard
    keywords: [song, bard, rumor, goblin, dark, horseman, drink]
    responses:
      - "The sullen bard says 'I'd sing of glory, but the goblins keep editing my audience.'"
      - "The sullen bard says 'There's a horseman who rides the dark roads. Don't flag him down. He stops for the wrong people.'"

  - vnums: [3506, 3507, 3501]
    actor: goblin_band
    keywords: [goblin, leader, lieutenant, train, mountain, gold]
    flavor_only: true   # aggressive trainers; barks only
    responses:
      - "The goblin lieutenant snarls orders nobody is following. A goblin warband is just a brawl with a flag."

  - vnum: 3502
    actor: the small boy
    keywords: [boy, help, hurt, lost, goblin]
    responses:
      - "The small boy licks his wounds and says 'They took the road and most of my courage. Mind the bend.'"
```

## Haon Dor — deep forest, druids, wargs

```yaml
area: haon
entries:
  - vnum: 6116
    actor: the elder druid
    keywords: [druid, forest, nature, home, intruder, woods, herb]
    flavor_only: true   # aggressive; he attacks intruders
    responses:
      - "The elder druid says 'This forest is mine to keep and yours to leave. The choice closes quickly.'"

  - vnum: 6000
    actor: John the Lumberjack
    keywords: [wood, tree, lumber, forest, chop, john, work]
    responses:
      - "John the Lumberjack says 'Plenty of trees, plenty of things in the trees. I chop the quiet ones.'"
      - "John says 'Druid don't like me felling his woods. Druid don't like much. Mind him.'"

  - vnum: 6115
    actor: Shargugh
    keywords: [brownie, shargugh, forest, trick, grin, help]
    responses:
      - "Shargugh the Forest Brownie grins far too wide and says 'Help? Oh, I'll help. You'll just disagree about what with.'"

  - vnums: [6003, 6100, 6101, 6102, 6103, 6002]
    actor: haon_beasts
    keywords: [warg, wolf, rabbit, bear, beast, danger]
    flavor_only: true
    responses:
      - "The ferocious rabbit glares at you with murder in its tiny eyes. Yes. The rabbit. Do not laugh."
      - "A warg lifts its head and decides whether you are a threat or a meal. It is not a long deliberation."

  - vnums: [6004, 6005]
    actor: haon_quiet_animals
    keywords: [deer, fox, peace, graze, quiet]
    responses:
      - "The fallow deer grazes, ears swiveling. When it bolts, you should already be running the other way."
```

## Gnome Village — gnomes, hobgoblins, the troll

```yaml
area: gnome
entries:
  - vnum: 1505
    actor: chief gnome
    keywords: [chief, gnome, village, troll, hobgoblin, treasure, guard]
    responses:
      - "The chief gnome says 'We are small, not suicidal. That is why there are guards.'"
      - "The chief gnome says 'The troll is big and dumb. The hobgoblins are organized. Decide which one you'd rather meet.'"

  - vnums: [1504]
    actor: gnome guard
    keywords: [guard, troll, village, danger, king, defend]
    responses:
      - "The gnome guard says 'The troll is big, the hobgoblins are organized, and visitors are usually the problem.'"

  - vnum: 1503
    actor: gnome scientist
    keywords: [science, gnome, invent, experiment, machine, secret]
    responses:
      - "The gnome scientist mutters 'It will work this time. It is statistically due. The smoke is part of the process.'"

  - vnum: 1521
    actor: treasurer
    keywords: [treasure, money, gold, king, vault, coin]
    responses:
      - "The treasurer says 'Every coin has a destination. Most prefer not to be your pocket.'"

  - vnums: [1500, 1501, 1502, 1522, 1523]
    actor: gnome_villagers
    keywords: [gnome, village, help, work, prisoner, child]
    responses:
      - "The gnome man says 'On my way to work. The work is mostly not dying. It pays in not dying.'"
      - "The gnome prisoner whispers 'The hobgoblins took the deep tunnels. Watch the king's bodyguard before the king.'"

  - vnum: 1519
    actor: king of the hobgoblins
    keywords: [hobgoblin, king, gnome, troll, throne]
    flavor_only: true   # not aggressive but a boss; keep it smug
    responses:
      - "The hobgoblin king says 'Diplomacy is what weaklings call reloading.'"
```

## Graveyard — drunk gravedigger, restless dead

```yaml
area: grave
entries:
  - vnum: 3600
    actor: Henry the Gardener
    keywords: [grave, henry, gardener, dead, zombie, skeleton, ghoul, drunk, dig]
    responses:
      - "Henry the Gardener hiccups and says 'I tend the graves. Lately the graves tend to get up and tend themselves.'"
      - "Henry says 'Don't drink before a graveyard. Don't fight after. I do both, which is why I garden now.'"
      - "Henry squints at a headstone and says 'Some of these names I dug twice. Bring light, and bring a friend who can run.'"
```

## Mob Factory — where the monsters come from

A joke/meta area. Lean into the fourth-wall humor: this is the factory floor where the world's mobs are stamped out.

```yaml
area: mobfact
entries:
  - vnum: 9402
    actor: Foreman Floyd
    keywords: [factory, foreman, floyd, mob, monster, work, make, plan]
    responses:
      - "Foreman Floyd says 'Every monster you've killed came off a line like this one. Try not to take it personally; we don't.'"
      - "Foreman Floyd says 'Quotas are up. The world keeps losing fido, so the world keeps ordering fido.'"

  - vnums: [9406, 9407]
    actor: factory_workers
    keywords: [worker, work, factory, bored, mob, slime, break]
    responses:
      - "The factory worker yawns and says 'Stamp a kobold, stamp a rat, stamp a kobold. The economy of menace, friend.'"
      - "The factory worker, asleep in his chair, mumbles something about overtime and a beastly fido that got out."

  - vnum: 9404
    actor: the revolving drunk
    keywords: [drunk, spin, factory, dizzy]
    responses:
      - "The Revolving Drunk spins in place and says, on the way past, 'I quality-test the dizziness spells. Hic. They work.'"
```

## Sewer — the wet dark under Midgaard

Mostly aggressive. The Guardian Naga is the kindly exception and makes a good source of true warnings; lean on it.

```yaml
area: sewer
entries:
  - vnum: 7042
    actor: the guardian naga
    keywords: [sewer, guardian, naga, dark, path, danger, mindflayer, way]
    responses:
      - "The guardian naga regards you kindly and says 'Down here, kindness is rare and brief. I am the rare. The brief is everything else.'"
      - "The guardian naga says 'The mindflayers nest deeper, master to junior, getting smarter as you go down. Smarter is worse.'"
      - "The guardian naga says 'If you must come this way, come with light and leave with haste.'"

  - vnums: [7045, 7046]
    actor: the ettins
    keywords: [ettin, jones, herald, cruncher, mouse, sewer]
    responses:
      - "Herald 'Mouse-Killer' the ettin looks confused at you with both heads and says 'Jones crunches the big ones. I get the mice. We do not discuss the arrangement.'"

  - vnums: [7000, 7013, 7002, 7204]
    actor: sewer_lurkers
    keywords: [rat, mud, monster, sewer, smell, dark]
    flavor_only: true
    responses:
      - "Something in the muck is slowly becoming a shape. The sewer is patient and the sewer is hungry; do not let it finish."
```

## Elemental Canyon

```yaml
area: canyon
entries:
  - vnums: [9208, 9209, 9210, 9211, 9212]
    actor: elemental rulers
    keywords: [earth, fire, air, lightning, water, element, ruler, canyon]
    responses:
      - "The Earth Ruler says 'Stone does not move for whining. It moves for pressure.'"
      - "The Fire Ruler says 'If it burns blue, it is done being normal.'"
      - "The Air Ruler says 'The fastest path is rarely the safest one.'"
      - "The Lightning Ruler says 'Metal, water, arrogance. All conduct well.'"
      - "The Water Ruler says 'Flow around what you cannot break. Drown what refuses to move.'"

  - vnum: 9204
    actor: the elemental guardian
    keywords: [guardian, canyon, element, ruler, way, deep]
    responses:
      - "The elemental guardian says 'Five rulers, five tempers. Insult one and you have made the canyon a list of enemies.'"

  - vnum: 9238
    actor: the elixir vendor
    keywords: [elixir, potion, quaff, buy, heal, cure]
    flavor_only: true   # aggro-all; he attacks everyone
    responses:
      - "The vendor says 'Potions are quaffed. If you drink one, I will laugh and still keep your money.'"

  - vnum: 9231
    actor: elemental magician
    keywords: [stone, spell, magic, canyon, element, gem]
    responses:
      - "The magician says 'The canyon is a spell with rocks for syllables.'"
      - "The magician says 'I need certain stones for the working. So does everything else with hands here, so hurry.'"

  - vnum: 9234
    actor: alchemist
    keywords: [herb, elixir, potion, ingredient, acid, lava, mix]
    responses:
      - "The alchemist says 'Do not lick the components. That rule has history.'"

  - vnum: 9233
    actor: illusionist
    keywords: [illusion, hidden, path, exit, mist, wall, secret]
    responses:
      - "The illusionist says 'If a wall seems too boring, look at it again. Boredom is camouflage.'"

  - vnum: 9201
    actor: the mountain climber
    keywords: [climb, mountain, canyon, path, trek, up]
    responses:
      - "The mountain climber says 'Same trek as you, friend. Difference is I packed. Did you pack, or just hope?'"

  - vnums: [9214, 9215, 9216, 9217, 9223, 9225, 9203, 9236]
    actor: tiny_elementals
    keywords: [small, tiny, element, particle, spark, snowflake, flame, imp]
    responses:
      - "The tiny elemental is too small to threaten you and far too small to care that it can't."
      - "A little imp runs along being annoying. That is its entire job and it is fully committed."
```

## Astral Plane

```yaml
area: astral
entries:
  - vnum: 1900
    actor: The Astral Guardian
    keywords: [astral, plane, guardian, gith, soul, exit, enter]
    responses:
      - "The Astral Guardian says 'The Astral Plane does not kill the careless. It lets them forget where their body was.'"
      - "The Astral Guardian says 'Thought is the ground here. Doubt is the cliff. Mind your footing.'"

  - vnum: 1905
    actor: githyanki hunter
    keywords: [githzerai, queen, gith, hunter, silver, sword]
    responses:
      - "The githyanki hunter says 'Githzerai hide where discipline outlasts fear. We are not patient enough to like them.'"

  - vnum: 1916
    actor: imprisoned githzerai
    keywords: [escape, queen, githyanki, key, help, prison, lich]
    responses:
      - "The githzerai whispers 'The queen's pride is the lock. Her guards are merely metal around it.'"
      - "The githzerai whispers 'A githyanki kneels to the lich-queen. We kneel to no one, which is why we are in cages.'"

  - vnum: 1917
    actor: tortured mindflayer prisoner
    keywords: [mind, torture, queen, gith, prisoner, secret]
    responses:
      - "The mindflayer whispers directly into your skull, 'Pain is loud here. Listen for what it hides.'"

  - vnum: 1918
    actor: enslaved red dragon
    keywords: [chain, furnace, dragon, fire, queen, free]
    responses:
      - "The enslaved red dragon rumbles 'Chains melt. Masters burn. Timing is the difference.'"

  - vnum: 1904
    actor: a soulless being
    keywords: [soul, lost, empty, wander, plane]
    responses:
      - "The soulless being wanders past. Where it should have a name, there is only the wind that the Astral makes of forgetting."
```

## Dwarven Kingdom

```yaml
area: dwarven
entries:
  - vnum: 6508
    actor: dwarven mine leader
    keywords: [mine, mazekeeper, wraith, king, tunnel, work, ore]
    responses:
      - "The mine leader says 'Good mines have ore. Bad mines have wraiths. This one has management.'"
      - "The mine leader says 'The mazekeeper guards the deep turns. Get past him and the maze does the rest of the murdering.'"

  - vnum: 6507
    actor: dwarven miner
    keywords: [mine, tunnel, maze, lizard, snake, doctor, ore, dig]
    responses:
      - "The dwarven miner says 'If the tunnel turns twice and smells wrong once, you're near something hungry.'"

  - vnum: 6509
    actor: dwarven doctor
    keywords: [heal, doctor, poison, snake, wraith, hurt]
    responses:
      - "The dwarven doctor says 'I can patch a pick wound. I cannot patch stupid. Stop testing me.'"

  - vnums: [6503, 6504]
    actor: dwarven shopkeepers
    keywords: [store, hide, tooth, bread, buy, sell, baker]
    responses:
      - "The Hide & Tooth storekeeper says 'Dwarven goods last longer than dwarven patience. Buy fast.'"
      - "Granite Head the baker says 'Bread hard enough to club a goblin. We call that two services for one coin.'"

  - vnum: 6515
    actor: man in waiting
    keywords: [king, waiting, nervous, court, help, throne]
    responses:
      - "The man in waiting says 'Waiting is easy. Knowing what for is the part that ruins a man.'"

  - vnums: [6510, 6501, 6513, 6512, 6511]
    actor: dwarven_commoners
    keywords: [dwarf, peon, worker, mine, help, daughter, son]
    responses:
      - "The dwarven peon says 'Help is always wanted and never paid. You'll fit right in.'"
      - "The grinning dwarf says 'Smile in the deep places. It confuses whatever was about to eat you.'"
```

## Dwarven Catacombs

```yaml
area: catacomb
entries:
  - vnum: 2003
    actor: spelunker
    keywords: [catacomb, dark, templar, vampire, necromancer, prism, dragon, tunnel]
    responses:
      - "The spelunker says 'Bring light. Bring friends. If you hear chanting, bring speed.'"
      - "The spelunker says 'I know these tunnels. Knowing them is why I'm still scared of them.'"

  - vnum: 2002
    actor: templar page
    keywords: [templar, necromancer, undead, vampire, shadow, page]
    responses:
      - "The templar page says, mildly confused, 'The templars yell about infidels. The undead do not seem impressed.'"

  - vnum: 2015
    actor: grand templar
    keywords: [templar, darkenbeast, prism, dragon, undead, faith]
    responses:
      - "The grand templar says 'Faith is no excuse for poor formation.'"
      - "The grand templar says 'The DarkenBeast smells you before it sees you. Faith does not cover the wind.'"
```

## Great Eastern Desert

```yaml
area: eastern
entries:
  - vnum: 5006
    actor: nomad leader
    keywords: [desert, oasis, worm, dracolich, nomad, prayer, sand]
    responses:
      - "The nomad leader says 'The desert does not hide its teeth. It only waits for you to call them scenery.'"
      - "The nomad leader says 'Bring water or a boat for the oasis. The desert charges interest on thirst.'"

  - vnum: 5007
    actor: nomad commander
    keywords: [worm, brass, dragon, dracolich, oasis, route, sand]
    responses:
      - "The nomad commander says 'If the sand moves against the wind, stop walking on it.'"
      - "The nomad commander says 'The brass dragon greets you politely and then tries to kill you. Manners are not mercy.'"

  - vnum: 5008
    actor: nomad warrior
    keywords: [fight, dervish, scorpion, snake, camel, sand]
    responses:
      - "The nomad warrior says 'The small things kill the careless. The large things kill the loud.'"

  - vnum: 5009
    actor: slave
    keywords: [escape, nomad, dragon, oasis, help, water]
    responses:
      - "The slave whispers 'The oasis smiles too much. Trust water only after it stays water.'"

  - vnum: 5014
    actor: myconoid shaman
    keywords: [myconoid, mushroom, chant, desert, dark, spore]
    responses:
      - "The myconoid shaman says something damp and holy. You understand none of it, which may be a blessing."

  - vnum: 5015
    actor: the dustdigger
    keywords: [oasis, water, dive, swim, trap, dustdigger]
    flavor_only: true   # the "oasis" is the monster
    responses:
      - "That inviting little oasis is not water. It is a mouth that learned what water looks like. Do not dive in."
```

## Old Marsh — swamp, hags, and lying lights

```yaml
area: marsh
entries:
  - vnum: 8308
    actor: the Will-O-Wisp
    keywords: [marsh, swamp, light, path, way, wisp, follow, lost]
    responses:
      - "The Will-O-Wisp bobs and giggles 'Follow me! Everyone does! Not all of them come back, but they all follow!'"
      - "The Will-O-Wisp says 'The pretty lights lead somewhere. Somewhere is not the same as safe. Have fun deciding.'"

  - vnum: 8312
    actor: the Marsh Hag
    keywords: [hag, marsh, swamp, witch, curse, secret, dark]
    responses:
      - "The Marsh Hag cackles 'You want a secret? The swamp keeps them all and trades none. But I gossip. Step closer.'"

  - vnums: [8304, 8301, 8302, 8320, 8303, 8306]
    actor: marsh_brutes
    keywords: [swamp, thing, troll, giant, ogre, beast, mud]
    flavor_only: true
    responses:
      - "The Swamp Thing shambles closer, hungry and unhurried. The marsh raised it to wait, and it has waited longer than you've lived."
```

## Thalos — the abandoned town that bites back

Thalos's signature is mimics: a chest, two "crushed skeletons," a torn-up floor, broken pottery, stacked planks, a shelf — all of them are the same hungry creature wearing furniture. The honest hint here is that ordinary scenery is the trap.

```yaml
area: thalos
entries:
  - vnum: 5207
    actor: the horned lizard
    keywords: [thalos, town, empty, chest, treasure, furniture, mimic, safe, danger]
    responses:
      - "The horned lizard flicks its tongue toward the furniture and refuses to go near any of it. The lizard is smarter than the last six adventurers."
      - "The horned lizard slithers off. In Thalos, the only thing that runs from the furniture is the only thing worth trusting."

  - vnum: 5208
    actor: the stone golem
    keywords: [golem, guard, watch, stone, thalos, corner]
    responses:
      - "The stone golem stands watch and says nothing, because it has watched every clever visitor try the chest. It is tired of the chest's diet."

  - vnums: [5202, 5203, 5204, 5205, 5206]
    actor: the mimics
    keywords: [chest, pottery, planks, floor, skeleton, shelf, open, loot]
    flavor_only: true
    responses:
      - "The chest is too eager. The skeletons too convenient. The floor too ripped. Everything in this room is the same mouth, and it is smiling at your loot reflex."
```

## Troll Den — a kill-room with a nursery

Almost entirely aggressive. Keep this sparse: tier-0 ambiance, no real hints. The dark joke of the area (a baby troll "playing Doctor with a cadaver") sets the tone; don't oversell it.

```yaml
area: trollden
entries:
  - actor_group: trollden_ambiance
    keywords: [troll, den, beast, mistress, carnage, bone]
    flavor_only: true
    responses:
      - "$n is a troll, and trolls in the den come in exactly two moods: hungry, and asleep on the hungry. You have woken the wrong one."
      - "Something young and terrible is learning anatomy on something that used to be an adventurer. Do not become the next lesson."
```

## Valley of the Elves — xenophobes and one old hermit

The valley elves attack outsiders on sight; the lone non-hostile is an old hermit with a pipe who'll actually talk.

```yaml
area: valley
entries:
  - vnum: 7810
    actor: an old hermit
    keywords: [valley, elf, elves, hermit, intruder, danger, elder, way, pipe]
    responses:
      - "The old hermit puffs his pipe and says 'The valley elves don't hate you personally. They hate everyone with your number of legs and the wrong ears.'"
      - "The old hermit says 'Sentries first, scouts second, elders last. By the time you reach the elders, the valley already knows your name and dislikes it.'"
      - "The old hermit says 'Talemon mixes his chemicals up the way, past the daemon. Up there, the hydra has a head for every excuse you'll make.'"

  - vnums: [7807, 7808, 7801, 7803, 7805]
    actor: valley_elf_hostiles
    keywords: [intruder, human, valley, elf, die, leave]
    flavor_only: true
    responses:
      - "The valley elf does not converse with intruders. The valley elf converses with intruders' corpses, briefly, to be sure."

  - vnums: [7800, 7814]
    actor: valley_animals
    keywords: [dog, cooshee, antelope, animal, valley]
    responses:
      - "The cooshee, a great elven hound, watches you with none of its masters' hatred and all of their attention."
```

## Dangerous Neighborhood — gang turf

An urban area: two rival gangs (the Ogres and the Trolls), a gang leader and bruisers, a put-upon patrolman, and a vandal who'll keep tagging walls while you talk.

```yaml
area: hood
entries:
  - vnum: 2106
    actor: a patrolman
    keywords: [hood, gang, peace, law, trouble, patrol, leader, ogre, troll]
    responses:
      - "The patrolman says 'Two gangs, one block, zero patience. I keep the peace by deciding which fire to not put out first.'"
      - "The patrolman says 'The Ogres run the east, the Trolls run the west, and the gang leader runs whoever's still standing. Don't be standing where he's looking.'"

  - vnum: 2107
    actor: a vandal
    keywords: [vandal, paint, wall, gang, tag, slogan]
    responses:
      - "The vandal keeps spraying and says over his shoulder 'It's not vandalism, it's a directory. Read the walls and you'll know whose block you're bleeding on.'"

  - vnums: [2103, 2104, 2101, 2102, 2108]
    actor: hood_gangsters
    keywords: [gang, member, bruiser, leader, pitbull, fight, turf]
    flavor_only: true
    responses:
      - "The gang member sizes you up the way a butcher sizes a side of beef. You are not a person here; you are inventory."
```

## In the Air — sky country

A two-creature area, but a useful one: every room here is air-sector, so the whole place demands a way to fly. The fairy dragon is harmless and chatty and makes a perfect "you need fly" reminder.

```yaml
area: air
entries:
  - vnum: 1000
    actor: a fairy dragon
    keywords: [air, sky, fly, fall, wing, fairy, dragon, lost, bored]
    responses:
      - "The fairy dragon flutters over and says 'No wings of your own? Brave. The ground up here is a very long apology.'"
      - "The fairy dragon says 'I'm bored. You're falling. We both have problems, but only one of us has a parachute spell.'"
      - "The fairy dragon giggles 'Fly spell, mobile suit, demon wings, whatever you've got, keep it running. The sky does not offer refunds.'"

  - vnum: 1001
    actor: a griffin
    keywords: [griffin, attack, sky, predator, air]
    flavor_only: true
    responses:
      - "A black-winged griffin folds into a dive. Up here, predators have gravity on their side and you do not."
```

## Redferne's Residence — the waiting paladin

```yaml
area: redferne
entries:
  - vnum: 7900
    actor: the Grand Knight of paladins
    keywords: [paladin, knight, honor, help, quest, redferne, grand]
    responses:
      - "The Grand Knight says 'I have waited a long while for someone worth helping. Prove you are not the usual disappointment.'"
      - "The Grand Knight says 'Honor is not a discount on danger. It is the reason you walk toward it anyway.'"
```

## Wyvern's Tower — rangers, centaurs, gargoyles

```yaml
area: wyvern
entries:
  - vnum: 1708
    actor: Lord of the Rangers
    keywords: [ranger, tower, wyvern, track, hunt, forest]
    responses:
      - "The Lord of the Rangers says 'Tracking is listening to what cowards call silence.'"
      - "The Lord of the Rangers says 'The wyvern at the top is old. Old things in towers did not get old by being careless.'"

  - vnums: [1707, 1706, 1704]
    actor: rangers_and_hunters
    keywords: [ranger, trapper, centaur, gargoyle, tower, hunt, track]
    responses:
      - "The ranger leader says 'The tower watches the valley. The valley watches back.'"
      - "The hunter rests over a fresh kill and says 'Quiet feet, full belly, long life. Loud feet, this conversation, short one.'"

  - vnum: 1605
    actor: gargoyle shaman
    keywords: [gargoyle, shaman, tower, golem, stone]
    responses:
      - "The gargoyle shaman says 'Stone remembers the names of those who chip it.'"

  - vnums: [1716, 1714, 1715, 1710, 1711, 1712]
    actor: centaur_village
    keywords: [centaur, chief, village, elder, beast, herd]
    responses:
      - "The centaur chief says 'Two legs hurry. Four legs arrive.'"
      - "The elder centaur says 'The minotaurs took the inner tower. We took the wisdom to stay out of it.'"

  - vnum: 1702
    actor: trapper
    keywords: [trap, pelt, displacer, beast, hunter, sell]
    responses:
      - "The trapper says 'If the beast is where you aim, it's not a displacer.'"

  - vnum: 1720
    actor: a displacer beast
    keywords: [displacer, beast, graze, illusion, miss]
    responses:
      - "The displacer beast grazes a step to the left of where it appears to be. So will your sword, if you swing where your eyes tell you."
```

## Dragon Tower / Draconia

```yaml
area: draconia
entries:
  - vnum: 2203
    actor: a powerful mage
    keywords: [book, dragon, tiamat, tower, magic, hydra, crypt, study]
    responses:
      - "The powerful mage says 'A dragon with one head is a problem. A dragon with five is a committee.'"
      - "The powerful mage says 'If the final chamber feels empty, blame the gods who write resets.'"

  - vnum: 2202
    actor: the dragon master
    keywords: [master, crypt, dragon, lord, draconia, tower]
    flavor_only: true   # aggressive crypt-lord
    responses:
      - "The lord of the crypt says 'You found my dragons. Now find a way back out past them.'"

  - vnum: 2205
    actor: the Draconian King
    keywords: [king, queen, dragon, tiamat, tower, greatness]
    responses:
      - "The Draconian King says 'Greatness is simple. Stand above everyone and call the stairs tradition.'"

  - vnum: 2243
    actor: the Draconian Queen
    keywords: [queen, tiamat, dragon, king, tower]
    responses:
      - "The Draconian Queen says 'The King ponders his greatness. I ponder who cleans up after it.'"
      - "The Draconian Queen says 'Five heads make five opinions. None of them are fond of visitors.'"

  - vnum: 2200
    actor: the dragon hatchling
    keywords: [bone, dragon, tiamat, mother, tower, baby]
    responses:
      - "The dragon hatchling gnaws a bone and mumbles 'Five-head scary. Bone good.'"

  - vnum: 2227
    actor: A human slave
    keywords: [escape, tiamat, dragon, king, queen, help]
    responses:
      - "The human slave whispers 'The dragons know the doors. We know the corners where they do not look.'"

  - vnum: 2242
    actor: the draconian fool
    keywords: [fool, tiamat, dragon, king, joke]
    responses:
      - "The draconian fool says 'A five-headed dragon still has only one stomach. Terrible design, really.'"

  - vnum: 2206
    actor: a concubine
    keywords: [king, court, secret, dragon, rest]
    responses:
      - "The concubine says quietly 'The King talks in his sleep. What he says would interest his Queen. It interests everyone, really.'"
```

## Drow City

```yaml
area: drow
entries:
  - vnum: 5100
    actor: goblin slave
    keywords: [escape, matron, drow, yochlol, house, scout, mist, help]
    responses:
      - "The goblin slave whispers 'Drow houses climb on knives. The first house just has sharper stairs.'"
      - "The goblin slave whispers 'If the mist starts speaking, stop being where you are. The yochlol wears mist like you wear skin.'"
      - "The goblin slave whispers 'Priestess outranks warrior, matron outranks priestess, and the goddess outranks pity. There is no pity here.'"
```

## High Tower of Sorcery — robes, golems, and a great many cats

A huge area built around the three orders of magic (black robes for evil, white for good, red/neutral for balance) plus golems, scribes, apprentices, a jail, a kitchen, and an absurd number of cats. Most apprentices and golems are hostile guardians; the staff (cooks, librarians, scribes, masters, the prisoner) are your talkers.

```yaml
area: hitower
entries:
  - vnums: [1356, 1359, 1362, 1364]
    actor: tower_masters
    keywords: [tower, sorcery, robe, black, white, neutral, balance, master, order, magic]
    responses:
      - "The master of Neutrality says 'Black robes plot, white robes preach, and I keep the scales from tipping into either kind of tedium.'"
      - "The master of the black robes says 'Power has no morals, only a price. The white robes pretend otherwise and pay anyway.'"
      - "The master of goodness frowns and says 'Goodness is not weakness. It is restraint, which is the harder spell.'"
      - "The Grand Mistress of Magic says 'You walked into the Tower without an appointment. The Tower keeps records of how that goes.'"

  - vnums: [1305, 1306, 1307, 1308]
    actor: tower_staff
    keywords: [drink, bar, food, cook, eat, gold, pot, ezmerelda, strick]
    responses:
      - "Strick levitates a drink to you and says 'In a tower full of wizards, the bartender is the only one who never runs out of patience or beer.'"
      - "Tatorious counts his coins and says 'Magic pays poorly and tips worse. Buy something.'"
      - "Ezmerelda stirs the pot and says 'Don't ask what's in it. In this tower, that question has gotten people turned into ingredients.'"

  - vnums: [1353, 1331, 1330, 1332, 1333]
    actor: tower_scribes_and_librarian
    keywords: [book, library, scroll, scribe, translate, knowledge, key, parchment]
    responses:
      - "The Librarian, jolted awake, says 'Quiet. Books here remember who mishandled them, and books here can hold a grudge with teeth.'"
      - "The master scribe says 'I copy spells all day. The trick is not reading them aloud. The last assistant read one aloud.'"

  - vnum: 1310
    actor: the Jailor
    keywords: [jail, prisoner, key, cell, escape, lock]
    responses:
      - "The Jailor yawns and says 'The prisoner has been chained so long he's started giving directions. Don't trust a man who'd say anything for a key.'"

  - vnum: 1309
    actor: The prisoner
    keywords: [escape, prisoner, key, jail, help, master, way]
    flavor_only: true   # he's aggressive/wimpy; treat as desperate, not trustworthy
    responses:
      - "The chained prisoner rasps 'Free me and I'll tell you everything. I'll tell you anything. That's rather the problem with men in chains.'"

  - vnums: [1325, 1329, 1365, 1314, 1304]
    actor: tower_wizards
    keywords: [wizard, mage, spell, study, sleep, speech, magic, learn]
    responses:
      - "The aged wizard, half-asleep, says 'Magic is preparation followed by public embarrassment. Mostly the second part.'"
      - "The wandering wizard mumbles 'If a book screams, close it. If it whispers, close it faster.'"

  - vnums: [1320, 1322, 1324, 1326, 1315, 1316, 1301]
    actor: tower_students
    keywords: [student, apprentice, learn, practice, invisible, lost, study]
    responses:
      - "The student of spells says 'I'm trying to master invisibility. You can tell by how clearly you can still see me.'"
      - "The lost adventurer says 'I came in for the loot. I stayed for the not finding the exit. Learn from me. Leave.'"

  - vnums: [1355, 1358, 1361, 1363, 1319]
    actor: tower_cats
    keywords: [cat, kitten, calico, purr, familiar]
    responses:
      - "The cat regards you with the flat contempt of something that has watched archmages die and was unimpressed by all of them."
      - "The small kitten meows in terror. Even the kitten knows this tower is a bad place to be small in."

  - actor_group: tower_golems
    vnums: [1303, 1342, 1343, 1346, 1347, 1339, 1344, 1345]
    keywords: [golem, stone, guard, stairs, made, clay, diamond]
    flavor_only: true
    responses:
      - "The golem does not answer. It was built to guard, not to chat, and it is very good at exactly one of those."
```

## The Keep of Mahn-Tor — minotaur stronghold

A minotaur grand master and his eight class-masters (warrior, anti-paladin, paladin, archmage, cleric, druid, ranger, thief), ringed by minotaur guards, an ogre warren, and frozen wilds. Most guards are hostile; the grand master, butler, harem girl, and the wilder spirits (treant, willow) are the talkers.

```yaml
area: mahntor
entries:
  - vnum: 2333
    actor: Mahn-Tor
    keywords: [mahntor, keep, minotaur, master, ring, sword, weapon, grand]
    responses:
      - "Mahn-Tor looks down at you and says 'My masters each perfected one art. You arrived having perfected none. Bold.'"
      - "Mahn-Tor says 'The Keep keeps its best steel near its worst tempers. Mind which you reach for.'"

  - vnums: [2325, 2326, 2327, 2328, 2329, 2330, 2331, 2332]
    actor: minotaur_masters
    keywords: [warrior, paladin, mage, cleric, druid, ranger, thief, master, train, class]
    flavor_only: true   # mostly aggressive masters; ominous barks only
    responses:
      - "The minotaur master practices a killing form and does not look up. Here, each master is a class taken to its lethal conclusion."

  - vnum: 2323
    actor: the Minotaur Butler
    keywords: [butler, keep, master, order, serve, mahntor, guest]
    responses:
      - "The Minotaur Butler stands rigid and says 'Guests of the Keep are announced. Intruders are also announced, just more loudly and by the guards.'"

  - vnum: 2324
    actor: the Minotaur Ring-Keeper
    keywords: [ring, keeper, keep, treasure, guard, post]
    responses:
      - "The minotaur ring-keeper says 'A named weapon is still just a weapon until someone bleeds on it. The Keep has bled on many.'"

  - vnums: [2301, 2304]
    actor: the wilds
    keywords: [tree, treant, willow, forest, weep, nature, old]
    responses:
      - "The Old Treant creaks 'I stood before the Keep and I will stand after it. Minotaurs are a recent inconvenience.'"
      - "The Weeping Willow sheds its leaves like grief and says nothing you can use, only something you'll remember."

  - vnum: 2322
    actor: the Harem Girl
    keywords: [keep, help, escape, minotaur, mahntor, secret]
    responses:
      - "The harem girl cowers and whispers 'The guards rotate at the gate, not the inner halls. The inner halls never rest. Neither should you.'"
```

## Moria — the deep dark, crowded and rude

Tolkien-flavored: orcs, trolls, kobolds, a veteran warrior who "could be your father," sneaky thieves, mages, and a menagerie of colored bats (blue=lightning, red=fire, green=stink, white=cold, black=acid). Almost everything is hostile; play this as ambiance and warnings rather than help.

```yaml
area: moria
entries:
  - actor_group: moria_voice
    keywords: [moria, mine, balrog, dwarf, bridge, dark, drum, deep]
    flavor_only: true
    responses:
      - "$n says 'In Moria, the dark is not empty. It is crowded and rude.'"
      - "$n says 'If you hear drums, do not ask whether they are friendly.'"

  - vnums: [4150, 4151, 4152, 4153, 4154]
    actor: moria_bats
    keywords: [bat, blue, red, green, white, black, cave, fly]
    flavor_only: true
    responses:
      - "The bats here are sorted by misery: lightning, fire, stink, cold, acid. Whichever one finds you, it brings its color with it."

  - vnums: [4050, 4051]
    actor: moria_warriors
    keywords: [warrior, veteran, scar, father, fight]
    flavor_only: true
    responses:
      - "The veteran warrior has more scars than you have plans. In Moria, that is the usual exchange rate."
```

## Little Haven / Glass Fort — the Snow Queen's hold

A frozen fort: the Snow Queen, her white/blue dragons, frost giants, a polar bear, gnome guards, a master herbalist, a busy gnome, and a thief skulking the shadows.

```yaml
area: haven
entries:
  - vnum: 1809
    actor: the master herbal
    keywords: [haven, glass, fort, herb, mint, heal, queen, snow, cold]
    responses:
      - "The master herbal says, playing with her mints, 'Cold preserves everything, including grudges. The Snow Queen has had centuries to chill hers.'"
      - "The master herbal says 'Haven is named by optimists. Bring something warm and something sharp.'"

  - vnums: [1801, 1802]
    actor: haven_staff
    keywords: [gnome, busy, guard, fort, glass, work, cold]
    responses:
      - "The busy gnome shuffles past and says 'No time, no time, the Queen's ice doesn't shape itself and the dragons don't feed themselves.'"

  - vnum: 1810
    actor: the thief
    keywords: [thief, shadow, steal, fort, glass, sneak]
    responses:
      - "The thief slips between shadows and says 'Glass breaks, forts fall, and people like me are always already inside by the time you notice the cold.'"

  - vnum: 1806
    actor: the Snow Queen
    keywords: [queen, snow, ice, cold, haven, invite]
    flavor_only: true
    responses:
      - "The Snow Queen says 'I did not invite you. Few leave to complain about the oversight.'"
```

## Olympus — three pantheons, one crowded mountain

The roster mixes Greek gods with Odin (Norse) and Ra (Egyptian), which is a gift: have the displaced gods be quietly insulted about being filed under "Olympus." Keep each god's lore accurate — Prometheus and his fire, Hephaestus thrown from the mountain, Hera's jealousy, Poseidon and the sea.

```yaml
area: olympus
entries:
  - vnum: 901
    actor: Zeus
    keywords: [zeus, olympus, god, throne, lightning, king, mercy]
    responses:
      - "Zeus says 'Mortals ask for mercy when they mean exception.'"
      - "Zeus says 'I rule the sky, the storm, and the long list of mortals who tested both.'"

  - vnum: 921
    actor: Hera
    keywords: [hera, queen, marriage, zeus, jealous, olympus]
    responses:
      - "Hera says 'My husband is the king of the gods and the king of excuses. I keep a list of both.'"
      - "Hera says 'Be loyal. I have made eternities miserable over far less than disloyalty.'"

  - vnum: 904
    actor: Poseidon
    keywords: [poseidon, sea, water, boat, ocean, storm, trident]
    responses:
      - "Poseidon says 'Carry a boat or learn humility. The sea accepts both.'"
      - "Poseidon says 'I shake the earth and I drown the proud. Frequently in that order.'"

  - vnum: 905
    actor: Apollo
    keywords: [apollo, music, song, sun, prophecy, stars, light]
    responses:
      - "Apollo plays a tune and says 'I am music, light, and prophecy. The prophecy part is why I rarely look surprised.'"

  - vnum: 903
    actor: Hermes
    keywords: [hermes, message, travel, speed, route, fast, road]
    responses:
      - "Hermes says 'Fast is not the same as safe, but it is much funnier.'"
      - "Hermes says 'I carry messages between gods who could smite the messenger. You see why I keep moving.'"

  - vnum: 907
    actor: Athena
    keywords: [athena, wisdom, strategy, war, battle, plan, owl]
    responses:
      - "Athena says 'Ares loves war. I love winning it. These are very different hobbies.'"
      - "Athena says 'Wisdom is knowing the fight you can skip. Most heroes skip the wisdom instead.'"

  - vnum: 908
    actor: Hephaestus
    keywords: [forge, weapon, smith, fire, armour, armor, hephaestus, craft]
    responses:
      - "Hephaestus says 'A weapon is honest. The hand holding it is the liar.'"
      - "Hephaestus says 'They threw me off this mountain once. Now they line up for the weapons I forge below it. Gods are like that.'"

  - vnum: 902
    actor: Prometheus
    keywords: [prometheus, fire, match, gift, mortal, chain, punish]
    responses:
      - "Prometheus flicks a match and says 'I gave your kind fire. They gave me a cliff, a chain, and a very punctual eagle. Worth it.'"
      - "Prometheus says 'Everything you forge, cook, and burn down traces back to one stolen spark. You're welcome.'"

  - vnum: 914
    actor: Odin
    keywords: [odin, norse, valhalla, raven, eye, wisdom, north]
    responses:
      - "Odin says 'I gave an eye for wisdom and crossed half a world for this mountain. They filed me under the wrong gods. I am resting on it.'"
      - "Odin says 'My ravens see all of it. They tell me most of it. I pretend to be surprised by the rest.'"

  - vnum: 919
    actor: Ra the Sun God
    keywords: [ra, sun, egypt, egyptian, boat, sky, light]
    responses:
      - "Ra the Sun God says 'I sail the sun across the sky each day and fight the serpent each night. And yet here I sit, mislabeled, on a Greek hill.'"

  - vnum: 916
    actor: the Alchemist
    keywords: [alchemy, potion, elixir, metal, secret, gold, mix]
    responses:
      - "The Alchemist says 'The secret is measurement. The explosion is what happens when students improvise.'"

  - vnums: [915, 917, 918]
    actor: olympus_merchants
    keywords: [buy, sell, weapon, armour, armor, grocer, food, train]
    responses:
      - "Julius the Green Grocer says 'Even heroes need vegetables. Especially heroes. Have you smelled a hero?'"
      - "Clive the Weapons Master says 'I practice with bow and axe so you don't have to practice with regret.'"

  - vnums: [909, 911, 912, 925, 923]
    actor: olympus_servants
    keywords: [serve, music, cook, citizen, apprentice, help, grape]
    responses:
      - "The apprentice musician says 'Apollo says practice makes perfect. He has not heard me practice.'"
      - "A Golden Maiden says 'I assist the smith. Mostly I hand him things and dodge the sparks. Eternity is sparks.'"

  - vnums: [924, 922]
    actor: olympus_mounts
    keywords: [pegasus, roc, horse, wing, fly, mount]
    responses:
      - "The pegasus tolerates your gaze the way the truly magnificent tolerate tourists."
```

## Galaxy — the night sky, made of myths with tempers

Constellations and the zodiac, mostly accurate to their myths. Best hooks: Orion and Scorpio never share the sky; Polaris is the fixed point everything wheels around; Andromeda was chained for her mother Cassiopeia's vanity; the Pleiades are seven sisters with one lost; Cancer and Leo were both hurled at the sky during Hercules' labours.

```yaml
area: galaxy
entries:
  - vnum: 9332
    actor: Polaris
    keywords: [polaris, star, universe, north, direction, lost, navigate]
    responses:
      - "Polaris says 'All roads pretend to wander. The true ones still point somewhere.'"
      - "Polaris says 'Everything in this sky wheels around me. Lose your way, find me, and the rest of the heavens becomes a map.'"

  - vnums: [9330, 9331]
    actor: Cassiopeia and Cepheus
    keywords: [queen, king, universe, throne, cassiopeia, cepheus, beauty, andromeda]
    responses:
      - "Cassiopeia says 'I called myself the most beautiful in the heavens. The gods set me in the sky to spin head-down forever. I still think I was right.'"
      - "Cepheus says 'My wife's vanity chained our daughter to a rock. Ruling the universe and ruling one's household are, it turns out, different crowns.'"

  - vnum: 9308
    actor: poor Andromeda
    keywords: [andromeda, chain, rescue, draco, help, cetus, monster]
    flavor_only: true   # aggro-all in the data; use as a tragic bark, not a quest
    responses:
      - "Andromeda says 'Chained to the rock for my mother's pride, left for the sea-beast. Heroes always ask about the chains after they finish admiring them.'"

  - vnum: 9311
    actor: Orion
    keywords: [orion, scorpio, hunter, draco, hunt, sky]
    responses:
      - "Orion says 'I hunt Scorpio across the whole sky and never catch him. When he rises, I set. The gods arranged it so neither of us ever wins.'"
      - "Orion says 'Scorpio hides in the obvious place: where everyone knows not to stand.'"

  - vnum: 9323
    actor: Scorpio
    keywords: [scorpio, orion, sting, tail, scorpion]
    flavor_only: true   # aggressive
    responses:
      - "Scorpio raises his tail and says 'I killed the great hunter once. The sky keeps us apart so I can't do it twice. Pity.'"

  - vnums: [9319, 9320]
    actor: cancer_and_leo
    keywords: [cancer, leo, crab, lion, hercules, labour, treasure]
    responses:
      - "Cancer the crab says 'I pinched Hercules during his labours and he crushed me for it. The gods gave me a constellation. Worst severance ever.'"
      - "Leo the lion roars 'Hercules strangled the lion of Nemea with his bare hands. They hung the lion in the stars. They did not hang the hands.'"

  - vnum: 9317
    actor: Gemini
    keywords: [gemini, brother, twin, lost, castor, pollux]
    responses:
      - "Gemini says 'I am looking for my brother. One of us was born to die and one to live forever, and we could not bear to be split. Have you seen him?'"

  - vnum: 9318
    actor: the evil Gemini
    keywords: [gemini, twin, hide, evil, brother, trick]
    flavor_only: true
    responses:
      - "The hiding twin grins 'One twin smiles. The other smiles with teeth. That is how you tell us apart, if you live long enough to.'"

  - vnum: 9326
    actor: Aquarius
    keywords: [aquarius, vessel, water, pour, cup]
    responses:
      - "Aquarius holds the vessel close and says 'Everyone wants what I'm pouring. No one asks what happens when the cup runs dry. It does not run dry. That is the trouble.'"

  - vnum: 9327
    actor: Pisces the mermaid
    keywords: [pisces, fish, help, vessel, water, mermaid]
    responses:
      - "Pisces the mermaid says 'The fish is jealous, the water is old, and the vessel never belonged to mortals. Help me before the jealous half notices you.'"

  - vnum: 9322
    actor: Libra
    keywords: [libra, sin, judge, balance, scales, justice]
    responses:
      - "Libra says 'The scales do not care why you sinned. They merely enjoy the weight.'"

  - vnum: 9321
    actor: Virgo
    keywords: [virgo, harvest, maiden, justice, wink]
    responses:
      - "Virgo winks and says 'I am the last of the gods to leave the mortal world when it turned cruel. You can see why I'm in no hurry to go back.'"

  - vnums: [9315, 9316, 9324, 9325]
    actor: zodiac_guards
    keywords: [aries, taurus, sagittarius, capricorn, ram, bull, archer, guard]
    responses:
      - "Aries welcomes you with the wary warmth of a ram who has been a guard too long."
      - "Sagittarius the centaur draws his bow and says 'I aim only at what I mean to hit. It saves so much apologizing.'"

  - vnum: 9309
    actor: the mighty Hercules
    keywords: [hercules, labour, strong, hero, god, work]
    responses:
      - "Hercules wipes his brow and says 'Twelve labours they gave me. I am up here doing extra ones. Heroism is mostly unpaid overtime.'"

  - vnum: 9312
    actor: the Pleiades
    keywords: [pleiades, sisters, seven, lost, weep, star]
    responses:
      - "The Pleiades weep and say 'We were seven sisters. Look closely and you will count only six clearly. The seventh is why we are still weeping.'"

  - vnum: 9310
    actor: the wild Pegasus
    keywords: [pegasus, horse, wing, fly, wild, mount]
    responses:
      - "The wild Pegasus grazes among the stars. Born from a monster's blood, tamed by exactly one hero, and not interested in being your second."

  - vnums: [9305, 9306, 9303, 9304, 9301]
    actor: stellar_phenomena
    keywords: [star, dwarf, supergiant, comet, nebula, mudder, lost]
    responses:
      - "The red supergiant rumbles 'I will collapse one day into something small and furious. Stars and adventurers end much the same way.'"
      - "A poor mudder, lost in the stars, looks at you with the universal expression of someone who took a wrong exit several areas ago."
```

## The Shire — halflings, a Keeper, and a lost youth

Tolkien-flavored, but keep every line original — no quoted text. Threads to lean on: the shiriffs hunting a lost halfling youth, the Keeper hoarding his "precious" ring, the Thain's authority, pipeweed, an Elven Wizard busy with fireworks, and a dwarven prince waiting for his king.

```yaml
area: shire
entries:
  - vnum: 1101
    actor: Keeper of the Ring
    keywords: [ring, precious, treasure, keeper, give, want]
    responses:
      - "The Keeper says 'No. Whatever you are about to ask, no.'"
      - "The Keeper clutches the ring and says 'Mine. It was always mine. It will be mine after you, too. Especially after you.'"

  - vnum: 1124
    actor: a local gossip
    keywords: [gossip, rumor, ring, keeper, youth, shiriff, news, latest]
    responses:
      - "The local gossip says 'Have you heard? The Keeper guards that treasure far too jealously for a peaceful soul. Peaceful souls don't hiss.'"
      - "The local gossip says 'The lost youth was seen where respectable folk pretend not to look. Which is exactly where you'll have to.'"

  - vnums: [1110, 1111, 1132]
    actor: shiriffs
    keywords: [shiriff, law, youth, trouble, ring, keeper, lost, missing]
    responses:
      - "The shiriff says 'We keep order by noticing what everyone else calls none of our business.'"
      - "The shiriff says 'A halfling youth's gone missing. Help us find him before the things in the dark do.'"

  - vnum: 1112
    actor: the Thain
    keywords: [thain, leader, shire, law, respect, order]
    responses:
      - "The Thain says 'The Shire runs on good manners and the quiet certainty that someone is watching. I am the someone.'"

  - vnum: 1102
    actor: Farmer Gamgee
    keywords: [farm, gamgee, harvest, crop, garden, weather]
    responses:
      - "Farmer Gamgee sniffs the air and says 'Harvest's near, or trouble is. Round here they tend to arrive together.'"

  - vnum: 1100
    actor: the Elven Wizard
    keywords: [wizard, elf, firework, magic, festival, elven]
    responses:
      - "The Elven Wizard says, not looking up from his fireworks, 'Magic should sometimes only delight. Rarely. Then back to the business of saving fools.'"

  - vnum: 1117
    actor: the dwarven prince
    keywords: [dwarf, prince, king, wait, mountain, return]
    responses:
      - "The dwarven prince says 'I sit and I wait for my king to return. A dwarf's patience is measured in mountains, so settle in or move along.'"

  - vnum: 1123
    actor: a seasoned adventurer
    keywords: [adventure, fame, treasure, death, ring, tale, story]
    responses:
      - "The seasoned adventurer says 'Fame is just survival told by someone else. I survived enough to bore you for hours. Buy me an ale and find out.'"

  - vnums: [1120, 1118, 1119, 1121, 1126]
    actor: shire_tradesfolk
    keywords: [weapon, armor, armour, gear, forge, shop, train, buy, blacksmith]
    responses:
      - "The blacksmith says 'Small folk live longer when their steel is not small.'"
      - "The battle master says 'My trainees scream their death-cries at straw dummies all day. Out there, the dummies scream back.'"

  - vnums: [1113, 1130, 1131]
    actor: shire_innkeepers
    keywords: [rest, food, drink, room, rumor, inn, pipeweed]
    responses:
      - "The Innkeeper says 'Rest here if you can. If you cannot, pretend you did and leave quietly.'"
      - "The receptionist signs a form and says 'Pipeweed's extra. Everything's extra. Comfort in the Shire is a luxury good now.'"
```

## Ultima — Britannia, its heroes, and its three great evils

This area is dense with real Ultima lore, so make the NPCs reference the right things:

- **The Avatar** embodies the Eight Virtues (Honesty, Compassion, Valor, Justice, Sacrifice, Honor, Spirituality, Humility), which derive from the three principles Truth, Love, and Courage.
- **Lord British** rules Britannia; **Blackthorn** is the regent who turned tyrant; **the Time Lord** guides the Avatar and is tied to the moongates.
- **The eight companions** each personify a virtue-city: Iolo (bard), Shamino (ranger), Dupre (paladin), Jaana (druid), Mariah (mage), Katrina (shepherd), Geoffrey (fighter), Julia (tinker); **Gwenno** is Iolo's wife and fellow bard; **Sentri** guards Lord British.
- **The Triad of Evil** is Mondain (the corrupted gem of immortality), Minax (his apprentice and lover), and Exodus (their machine-child, who ruled an island fortress).
- **The Guardian** is the great red-faced enemy of later days; **Batlin** leads the Guardian's cult, the Fellowship.
- **Draxthanum**, King of the Gargoyles, leads a people who once saw the Avatar as their False Prophet; both races revere the **Codex of Ultimate Wisdom**.
- **Mr. Smithy / Smith the talking horse** is the famous clue-giving easter egg.
- **Empath Abbey** (Compassion, makes wine) and **Serpent Hold** (Courage) and the city of **Yew** (Justice, its Court and prison) all appear.

```yaml
area: ultima
entries:
  - vnum: 2464
    actor: Lord British
    keywords: [british, avatar, heal, castle, virtue, lord, britannia, rule]
    responses:
      - "Lord British says 'Virtue is admirable. Preparation is survivable. The Avatar, bless him, keeps relearning the second one.'"
      - "Lord British says 'I have ruled Britannia through three great evils and one bad regent. Mind your manners; I have outlasted worse than you.'"

  - vnum: 2463
    actor: The Avatar
    keywords: [avatar, virtue, british, quest, shrine, truth, love, courage, codex]
    responses:
      - "The Avatar says 'Eight virtues, three principles, one long road. Truth, Love, and Courage; everything else is just where you stand on them.'"
      - "The Avatar says 'A quest is a straight road only after the bard edits it. Mine had moongates, dungeons, and a great deal of falling.'"
      - "The Avatar says 'Honesty, Compassion, Valor, Justice, Sacrifice, Honor, Spirituality, Humility. Skip even one and the road notices.'"

  - vnum: 2455
    actor: the Timelord
    keywords: [time, timelord, future, past, moongate, gate, codex]
    responses:
      - "The Timelord says 'The future is full of corpses who thought they had another round.'"
      - "The Timelord says 'The moongates open by the turning of the moons, not your impatience. Learn the cycle or wait out the wrong one.'"

  - vnum: 2457
    actor: Blackthorn
    keywords: [blackthorn, throne, regent, tyrant, law, rule, british]
    flavor_only: true   # aggressive
    responses:
      - "Blackthorn says 'I ruled in Lord British's absence and made the Virtues into laws with teeth. They called it tyranny. I called it follow-through.'"

  - vnums: [2460, 2459, 2458]
    actor: the_triad_of_evil
    keywords: [mondain, minax, exodus, gem, immortal, triad, evil, fortress]
    flavor_only: true   # all aggressive bosses; see Boss Taunts
    responses:
      - "Mondain grips his shattered gem and says 'I made myself deathless once. Death has been filing an appeal ever since.'"
      - "Minax says 'Mondain was my master and my love. His ruin was my inheritance, and I spend it freely on people like you.'"
      - "Exodus hums in its island fortress 'Not god, not demon. A machine, born of a sorcerer and a sorceress. The worst of both, calculated.'"

  - vnum: 2456
    actor: The Guardian
    keywords: [guardian, red, face, fellowship, enemy, evil]
    flavor_only: true   # aggressive
    responses:
      - "The Guardian, a vast red face, says 'I am patient where your gods are not. I do not invade. I am invited. Ask the Fellowship.'"

  - vnum: 2462
    actor: Batlin
    keywords: [batlin, fellowship, join, cult, guardian]
    responses:
      - "Batlin smiles and says 'Join the Fellowship. Strive for Unity. The words are warm. Do not ask too closely what they keep warm.'"

  - vnum: 2448
    actor: Draxthanum the Gargoyle
    keywords: [gargoyle, king, shrine, virtue, stone, codex, prophet, draxthanum]
    responses:
      - "Draxthanum, King of Gargoyles, says 'Humans call what they do virtue. Gargoyles prefer results. We have our own three principles and our own dead from your False Prophet.'"
      - "Draxthanum says 'The Codex of Ultimate Wisdom was sacred to my people before it was sacred to yours. We remember which of us bled for it.'"

  - vnum: 2409
    actor: Nystul the Librarian
    keywords: [book, library, spell, scroll, knowledge, nystul, codex]
    responses:
      - "Nystul says 'Knowledge is power. Unfortunately, so is a sword, and swords are faster.'"
      - "Nystul says 'I keep Lord British's scrolls. The Codex holds the answers; I merely hold the index, and the index is exhausting enough.'"

  - vnum: 2417
    actor: Chuckles the Jester
    keywords: [joke, riddle, chuckles, fool, clue, jester]
    responses:
      - "Chuckles says 'A clue in a jester's mouth is still a clue. Regrettably for everyone's dignity, including mine.'"
      - "Chuckles juggles and says 'Lord British keeps a fool to speak the truths a crown cannot. Want a truth? It'll cost you a laugh.'"

  - vnum: 2413
    actor: Mr. Smithy the talking horse
    keywords: [horse, smithy, smith, talk, clue, hint, hay]
    responses:
      - "Mr. Smithy the talking horse chews his hay and says 'A talking horse who gives good advice, and still nobody listens. Britannia in one stable, friend.'"
      - "Mr. Smithy says 'Want the famous hint? Mind thy virtues, watch thy back, and never trust a chest in a room full of clever-looking junk.'"

  - vnums: [2416, 2419, 2414]
    actor: ultima_bards
    keywords: [bard, song, iolo, gwenno, music, tale, mandolin]
    responses:
      - "Iolo the Bard smiles over his bottle and says 'I sing the Avatar's deeds and crossbow the rest. A bard needs both a tune and a trigger out here.'"
      - "Gwenno the Bard says 'Iolo is my husband and my favorite verse. I was taken once, by gargoyles; I came back sharper than the rescue.'"

  - vnums: [2454, 2446, 2426, 2410, 2405, 2436, 2441, 2461]
    actor: ultima_companions
    keywords: [shamino, dupre, jaana, mariah, katrina, geoffrey, julia, sentri, companion, virtue]
    responses:
      - "Shamino the Ranger says 'Each of us carries a virtue like a pack. Mine is the quiet kind; I walk point so the louder ones can argue behind me.'"
      - "Dupre the Paladin consults his maps and says 'Honor first, strategy second, drinking a respectable third. In that order I have survived everything Britannia threw.'"
      - "Mariah the Mage says, searching the stacks, 'Honesty is my virtue and my burden. Ask me a true question and brace for a true, lengthy answer.'"
      - "Katrina the Shepherd says 'My city drowned in its own pride. I tend sheep now and humility keeps me, of the eight, the most alive.'"

  - vnums: [2442, 2437, 2425, 2429, 2431, 2430, 2418]
    actor: ultima_townsfolk
    keywords: [abbey, empath, serpent, yew, judge, court, wine, pub, citizen, town]
    responses:
      - "The Lady of Empath Abbey says 'We brew the wine of Compassion in Yew's shadow. Drink slowly; even kindness has a cellar that bites.'"
      - "The Judge says 'This is the Court of Yew, where Justice is weighed. Lie to me and the scales will weigh you instead.'"
      - "Lord Simon of Serpent Hold says 'Courage is the virtue we keep here. The hold is named for the serpent because both are easy to wake and hard to survive.'"

  - vnums: [2402, 2405, 2403, 2440]
    actor: ultima_humble_folk
    keywords: [shepherd, sheep, girl, tinker, village, help]
    responses:
      - "The shepherd says 'I look after sheep and stay out of the Avatar's stories. It is, statistically, the safest job in Britannia.'"
      - "A tinker fiddles with a device and says 'Sacrifice is the virtue of Minoc. We give until it hurts, then we invoice. Tinkers have to eat.'"
```

## Clan Headquarters

These NPCs should know only their own clan, never enemy-HQ secrets. Keep them smug, factional, and unreliable. The HQ rooms block recall and refuse portals, so any "leaving" advice should point at walking out. Only the non-combatant support roles (trainers, practice-masters, quartermasters, scribes) talk; the named guardians are aggressive.

```yaml
area: cithdeux
entries:
  - vnums: [9607, 9608, 9609, 9604]
    actor: Cith Deux noncombatants
    keywords: [cith, peaceman, clout, martina, jaraxle, clan, dan, quest, lead, train]
    responses:
      - "Dan says 'Leadership is easy. Everyone just ignores you from a better angle.'"
      - "Jaraxle says 'I am a lesson in what not to do, which is still teaching.'"
      - "Martina says 'If a dragon-spooker girl is involved, the day is already ruined.'"
      - "Clout says 'Questing builds character. So does being broken. I recommend the first.'"
      - "Dan says 'No portals in or out of here. The clan likes its doorways earned on foot.'"

area: divergent
entries:
  - vnums: [9706, 9707, 9708, 9709]
    actor: Divergent support NPCs
    keywords: [divergent, cove, seeress, bard, scribe, clan, xorke, atsu, leinenna, train]
    responses:
      - "Xorke sings 'Distant lands are always better before you arrive.'"
      - "Atsu says 'Ink lasts longer than memory and complains less.'"
      - "Leinenna says 'Watch your mouth. Then watch everyone else's hands.'"
      - "The Seeress of Cove mumbles 'The path is clear, which is how you know it lies.'"

area: renegades
entries:
  - vnums: [9951, 9955, 9958, 9959]
    actor: Renegades support NPCs
    keywords: [renegade, money, q, lupin, malucifer, clan, valek, feed, train]
    responses:
      - "Valek says 'Money talks. Mine screams. Feed me and we'll get along.'"
      - "Q says 'Might be here. Might not. Might be your problem.'"
      - "Lupin says 'Everything has a price. Some prices have legs.'"
      - "Malucifer says 'Confusion is a strategy if you survive it. I'm still checking which half applies to me.'"

area: teikoku
entries:
  - vnums: [9803, 9805, 9808]
    actor: Teikoku support NPCs
    keywords: [teikoku, tournament, master, gundam, wufei, clan, balcony, train, fight]
    responses:
      - "Master Camptella says 'A quiet balcony sees more blood than the arena floor.'"
      - "Master Takumba says 'Competition is politeness before violence.'"
      - "Wufei says 'A machine is only as honorable as the fool inside it. I have piloted both ends of that sentence.'"

area: zrollers
entries:
  - vnums: [9907, 9906]
    actor: Z-Rollers support NPCs
    keywords: [zroller, gold, leprechaun, mitsurugi, clan, note, train, practice]
    responses:
      - "The leprechaun says 'Touch the gold and I discover courage.'"
      - "Mitsurugi says 'The notes are red because subtlety died poor.'"

area: malokteri
entries:
  - vnums: [9506, 9507, 9508, 9509]
    actor: Malokteri support NPCs
    keywords: [malokteri, clan, azag, kubabbar, maklu, sut, train, practice, offer, pocket]
    responses:
      - "Azag coughs fire and says 'Practice here where the fire is friendly. It is the only thing in this hall that is.'"
      - "Kubabbar waves a fist and says 'Empty your pockets for training and I'll fill your head with something worth the trade.'"
      - "Maklu says 'I have much to offer and very little of it free. That is also the clan motto, more or less.'"
      - "Sut Resi paces and mutters 'A plan, a plan, almost a plan. Come back when one of us has finished a thought.'"
```

## The Arena — trash talk and old grudges

A roast-gallery of arena fixtures who exist mostly to insult. They're non-hostile fixtures, so let them needle the player about PK, dying, and ego. Keep it cocky and funny, never genuinely mean-spirited toward the real person.

```yaml
area: arena
entries:
  - vnums: [90, 91, 92, 93]
    actor: arena_fixtures
    keywords: [arena, pk, fight, duel, rec, win, lose, columbo, palon, fistan, mdk, kill]
    responses:
      - "Columbo says 'Big talk, small health bar. I have seen your type spectate from the floor before.'"
      - "Palon says 'Dumbass extraordinaire, at your service. Takes one to roast one, friend.'"
      - "Fistan whines 'I swear I'll be good if they let me back in. Meanwhile, you, fresh meat, are very much allowed in. Suspicious.'"
      - "Mdk says 'The arena teaches one lesson: fair fights are a story losers tell. Pick your spots or become someone's spot.'"
      - "Columbo says 'You yelled. In an arena. To the people who watch others die for sport. Bold opening.'"
```

## Limbo — the void at the edge of everything

Mostly aggressive shadow-guardians and a gatekeeper who keeps "unwanted trash" out. Puff the Fractal Dragon is the calm, cosmic exception and makes a wonderful source of cryptic flavor about the void and the staging-ground nature of this place.

```yaml
area: limbo
entries:
  - vnum: 1
    actor: Puff
    keywords: [puff, dragon, void, limbo, reality, fractal, meaning, lost]
    responses:
      - "Puff the Fractal Dragon says, contemplating a higher reality, 'You see a room. I see the same pattern, smaller, all the way down. We are both correct and only one of us is worried.'"
      - "Puff says 'This is the edge of the made world. Things stage here before they enter. Try not to be one of the things.'"
      - "Puff says 'Loss, like the void, is mostly a matter of scale. Zoom out far enough and even dying is just a tidy little fold.'"

  - actor_group: limbo_shadows
    vnums: [10, 11, 12, 13]
    keywords: [shadow, soldier, void, limbo, dark, guard]
    flavor_only: true
    responses:
      - "The shadow does not speak. It searches the void for targets, and the void has a short list, and you just put yourself on it."

  - vnum: 3
    actor: the guardian vampire
    keywords: [vampire, guardian, light, dark, limbo]
    flavor_only: true
    responses:
      - "The guardian vampire shields his face from your light and says 'Bring a brighter flame or a quieter death. I am equipped for either.'"
```

## The Rats Lair — the immortals' joke-temple

Absurd by design: the mortal manifestations of the MUD's overlords (Hatchet, Icaza, Frag, Kahn, Furey) hold court alongside their pointedly "cute" murder-pets. Use sparingly; reinforce the silliness, don't hand out real progression.

```yaml
area: rats
entries:
  - vnums: [3815, 3816]
    actor: Hatchet and Icaza manifestations
    keywords: [hatchet, icaza, overlord, goddess, rat, lair, worship, god]
    responses:
      - "The manifestation of Overlord Hatchet whispers 'Worship is optional. Consequences are not.'"
      - "The manifestation of Goddess Icaza whispers back something that makes Hatchet laugh, which is somehow worse than the silence."

  - vnums: [3802, 3803]
    actor: the cute murder-pets
    keywords: [snake, kitten, snigger, sammy, cute, pet, bite]
    responses:
      - "Snigger, the cute man-eating snake, smiles with too many teeth for an animal that 'just wants pets.'"
      - "Sammy, the flesh-clawing kitten, mews adorably. The adorable part is the bait. The flesh-clawing part is the point."

  - vnum: 3808
    actor: wererat
    keywords: [rat, business, tunnel, snake, kitten, deal]
    responses:
      - "The wererat says 'Rat business is private, profitable, and smells better than hero business.'"

  - vnums: [3813, 3814]
    actor: rats_slaves
    keywords: [slave, escape, dig, tunnel, help, rat]
    responses:
      - "The human slave shivers and whispers 'The rats run this whole warren. Dig where they aren't looking, or don't dig at all.'"
```

## Pikachu's Vacation — legally-distinct adorable creatures

A parody zone of high-level "cute" critters. Keep every line original parody: marketably adorable, mechanically unhelpful, and always one lawyer away from a cease-and-desist. Reproduce nothing.

```yaml
area: vacation
entries:
  - actor_group: vacation_creatures
    vnums: [6600, 6601, 6602, 6603, 6604, 6605, 6606, 6607]
    keywords: [vacation, cute, beach, creature, catch, pocket, monster, evolve]
    responses:
      - "$n does something marketably adorable and mechanically unhelpful, then waits for applause it has legally trademarked."
      - "$n stares at you like a lawsuit waiting to happen. Best not to say its name too loudly or too completely."
      - "$n is level ninety and weighs as much as a feather and a legal threat combined. Cute does not mean harmless here."
      - "$n waddles, floats, or flops with the serene confidence of a creature that knows it sells more plush toys than you ever will."
```

## Apocalypse Headquarters — apoc

The deepest no-recall sink in the game: nearly every room here refuses recall, and portals/gates won't carry you out of the clan's heart. The only NPCs worth talking to are the prisoners (who know things) and the rank-and-file guards (who don't, but enjoy saying so). Larsen, Tarrant, Kyriel, Dorin, and Ingen are the marquee killers — they appear in the Boss Taunts section, not here, because they will not be chatting.

```yaml
area: apoc
entries:
  - vnum: 806
    actor: A Prisoner
    keywords: [escape, larsen, tarrant, kyriel, dorin, ingen, dungeon, guard, shadow, recall, out]
    responses:
      - "The prisoner whispers 'Ingen answers to shadows, not names. Say shadow if you must die loudly.'"
      - "The prisoner whispers 'This place eats recall whole. Don't reach for it — walk, and walk fast.'"
      - "The prisoner whispers 'Dorin duels because murder sounded too honest to him.'"
      - "The prisoner whispers 'Tarrant smiles before he kills. Kyriel doesn't bother. Larsen owns them both.'"
      - "The prisoner whispers 'There's no gate out of a clan's heart. The only exit is the door you came in. Find it on foot.'"

  - vnum: 804
    actor: An Apocalypse Guard
    keywords: [larsen, apocalypse, guard, dungeon, tower, prisoner, recall, out, help]
    responses:
      - "The guard says 'Orders are simple. Keep prisoners in, keep idiots moving toward the deep end.'"
      - "The guard says 'If you hear laughing in the hall, it's already too late to ask whose.'"
      - "The guard says 'Recall? In here? That's adorable. Try the stairs like the rest of the corpses.'"
      - "The guard says 'You walked into Apocalypse on purpose. I'm not even going to bother lying to you about your odds.'"

  - actor_group: apoc_lowranks
    vnums: [800, 801, 802, 803, 805]
    keywords: [larsen, clan, apocalypse, danger, leave, recall]
    responses:
      - "$n grunts 'Nobody important is named here. The important ones name themselves, usually right before they kill you.'"
      - "$n says 'No recall, no gate, no mercy. We keep this place tidy in exactly that order.'"
      - "$n says 'You can leave the way you came. You probably won't, but you can.'"
```

# Extra Character Flavor (merge-by-vnum addendum)

These add **more lines to characters that already have an entry above.** The engine should pool a vnum's base `responses` with any `extra_responses` here, then pick at weighted random — so this section deepens existing NPCs without touching their blocks. Entries accept either `vnum:` (one mob) or `vnums:` (a group, whose extra lines pool into every member). Add to it freely; it's the per-character equivalent of the Universal Chatter Pool. All original writing, lore-flavored, reproduce nothing.

```yaml
area: _extra_flavor
merge_by_vnum:

  # --- Olympus: the gods get pettier the more you talk ---
  - vnum: 901   # Zeus
    extra_responses:
      - "Zeus says 'Lightning is a gift I give the worthy and a bill I send everyone else.'"
      - "Zeus says 'Yes, the affairs are real. No, we will not be discussing them. Hera is RIGHT there.'"
      - "Zeus says 'I run this mountain. Poseidon thinks he runs the sea. We let him think things.'"
  - vnum: 921   # Hera
    extra_responses:
      - "Hera says 'I am goddess of marriage, which is the longest war anyone in this family has survived.'"
      - "Hera says 'Whatever Zeus told you, halve the heroism and double the apologies.'"
  - vnum: 904   # Poseidon
    extra_responses:
      - "Poseidon says 'Drowning is the most honest thing the sea does. Everything else is foreplay.'"
      - "Poseidon says 'My brother got the sky, I got the sea, and the third one got the basement. I came out fine.'"
  - vnum: 905   # Apollo
    extra_responses:
      - "Apollo says 'Music, prophecy, plague, and the sun. I contain multitudes. Most of them have a fever.'"
      - "Apollo says 'I could foretell your death, but you'd find it on your own soon enough out here.'"
  - vnum: 907   # Athena
    extra_responses:
      - "Athena says 'Wisdom is knowing which fights to win. You're currently failing the first half.'"
      - "Athena says 'I was born from a headache and I've been the smartest person in every room since.'"
  - vnum: 908   # Hephaestus
    extra_responses:
      - "Hephaestus says 'They threw me off the mountain for being ugly. Now they all wear my work. Funny, that.'"
      - "Hephaestus says 'Bring me a real forge problem and I'll respect you. Bring me small talk and I'll keep hammering.'"
  - vnum: 914   # Odin (annoyed at being filed under Olympus)
    extra_responses:
      - "Odin says 'I gave an EYE for wisdom. The Greeks gave a goblet of wine and a nap. We are not the same.'"
      - "Odin says 'Ravens, gallows, runes, war. Then the filing system puts me in OLYMPUS. The indignity.'"
  - vnum: 919   # Ra (also misfiled)
    extra_responses:
      - "Ra says 'I sail the sun across the sky each day and fight a serpent each night. And I'm shelved under GREEK myth. Marvelous.'"
      - "Ra says 'In my pantheon the dead get weighed against a feather. Yours would not enjoy the result.'"

  # --- Ultima: the cast that built half this MUD's heart ---
  - vnum: 2464  # Lord British
    extra_responses:
      - "Lord British says 'I have ruled Britannia through three ages of evil and one very persistent red face. Welcome.'"
      - "Lord British says 'The Eight Virtues are not suggestions. Ask the Avatar. Ask anyone who skipped one.'"
      - "Lord British says 'Mind the ankh, honor the codex, and for pity's sake don't trust a Fellowship recruiter.'"
  - vnum: 2463  # The Avatar
    extra_responses:
      - "The Avatar says 'Truth, Love, and Courage. Everything else is just the three of them wearing hats.'"
      - "The Avatar says 'I've been every class and walked every dungeon. The trick was never the sword. It was the codex.'"
  - vnum: 2455  # the Timelord
    extra_responses:
      - "the Timelord says 'I have seen your ending. Several of them. You die better in some than others.'"
      - "the Timelord says 'Time is a moongate you cannot quite aim. I aim it anyway. That's the whole job.'"

  # --- Shire: Tolkien-flavored, never Tolkien-quoted ---
  - vnum: 1101  # Keeper of the Ring (Gollum-coded)
    extra_responses:
      - "The Keeper clutches a small bright thing and hisses 'Mine. Found it. Keeps it. You wants it, but no.'"
      - "The Keeper says 'Nice hobbitses don't ask about the precious. You're not nice, is you. No, no.'"
  - vnum: 1112  # the Thain
    extra_responses:
      - "The Thain says 'I govern a land of farmers and second breakfasts. It is the finest job in the world and I will not apologize.'"
      - "The Thain says 'Adventurers come through, eat us out of pantry and patience, and leave. You're right on schedule.'"
  - vnum: 1102  # Farmer Gamgee
    extra_responses:
      - "Farmer Gamgee says 'A garden's a kind of courage you can eat. Don't trample mine learning that lesson.'"
      - "Farmer Gamgee says 'Folk go off to be heroes. I stayed to grow taters. Guess which of us still has all his fingers.'"

  # --- Wyvern / Rangers ---
  - vnum: 1708  # Lord of the Rangers
    extra_responses:
      - "The Lord of the Rangers says 'A ranger fears nothing in the wild because a ranger IS the thing in the wild to fear.'"
      - "The Lord of the Rangers says 'Track quietly, strike once, apologize never. That's the whole curriculum.'"

  # --- Draconia ---
  - vnum: 2205  # the Draconian King
    extra_responses:
      - "The Draconian King says 'My court is scales and fire and exquisite manners. Forget the last one and you'll meet the first two.'"
  - vnum: 2243  # the Draconian Queen
    extra_responses:
      - "The Draconian Queen says 'The King roars. I plan. Guess which of us the kingdom should actually fear.'"
  - vnum: 2200  # the dragon hatchling
    extra_responses:
      - "The hatchling chirps, sets a curtain on fire entirely by accident, and looks immensely proud of itself."

  # --- Galaxy: constellations with grudges ---
  - vnum: 9332  # Polaris
    extra_responses:
      - "Polaris says 'Every lost soul in history has looked up and found me. You're not even slightly special, but you're welcome.'"
  - vnum: 9311  # Orion
    extra_responses:
      - "Orion says 'I hunt across the whole winter sky, and I still never have to share it with that scorpion. Restraining order. Cosmic.'"
  - vnum: 9309  # Hercules
    extra_responses:
      - "Hercules says 'Twelve labours. TWELVE. And the bards only ever ask about the lion. There were ELEVEN others, people.'"

  # --- Mud School / hub flavor ---
  - vnum: 9402  # Foreman Floyd (Mob Factory)
    extra_responses:
      - "Foreman Floyd says 'Quota's three hundred monsters a shift. You heroes kill 'em faster than we can stamp 'em out. Job security, I guess.'"
      - "Foreman Floyd says 'Every goblin you've ever slain rolled off MY line. A little respect for the craftsmanship wouldn't kill you.'"
  - vnum: 3600  # Henry the Gardener (Graveyard)
    extra_responses:
      - "Henry the Gardener says 'I keep the graves tidy. Business is good. Business is ALWAYS good, thanks to people like you.'"

  # --- Limbo ---
  - vnum: 1     # Puff
    extra_responses:
      - "Puff drifts through the static between worlds, ancient and faintly amused, and regards you as one regards weather."
      - "Puff says nothing you can repeat, but you leave the conversation certain the universe is older and stranger than your loot suggests."

  # --- Dwarven Kingdom ---
  - vnum: 6508  # dwarven mine leader
    extra_responses:
      - "The mine leader says 'We dig deep, we dig greedy, and we dig until something digs back. Then we dig faster.'"
      - "The mine leader says 'Gold's down there. So's whatever the gold was buried to keep company. Choose your shovel accordingly.'"
  - vnum: 6509  # dwarven doctor
    extra_responses:
      - "The dwarven doctor says 'Lost an arm? Walk it off. Lost a leg? That's trickier, but still: walk it off.'"
      - "The dwarven doctor says 'Half my patients are crush injuries and the other half are bar fights about crush injuries.'"

  # --- Great Eastern Desert nomads ---
  - vnum: 5006  # nomad leader
    extra_responses:
      - "The nomad leader says 'The desert keeps no roads and forgives no maps. We don't get lost. We get RELOCATED.'"
      - "The nomad leader says 'Water is wealth here. You're carrying steel and gold and not one full skin. Tourists.'"
  - vnum: 5007  # nomad commander
    extra_responses:
      - "The nomad commander says 'Sandstorm's coming. You'll know because the sky turns the color of a bad decision.'"

  # --- High Tower of Sorcery ---
  - vnum: 1364  # The Grand Mistress
    extra_responses:
      - "The Grand Mistress says 'Black robes plot, white robes preach, neutral robes do the actual work. I sign all three paychecks.'"
      - "The Grand Mistress says 'You came to a tower full of wizards to ask ME a question. The arrogance is almost magely. Almost.'"
  - vnum: 1359  # master of the black robes
    extra_responses:
      - "The master of the black robes says 'Vile deeds don't plot themselves. Well. A few of mine do now. I'm very good.'"
  - vnum: 1305  # Strick the bartender
    extra_responses:
      - "Strick levitates a drink past your ear without looking. 'House rule: no fireballs at the bar. Learned that one expensively.'"
      - "Strick says 'Wizards are my worst customers. They argue the bill into a paradox and vanish. Tip in coin, not theory.'"
  - vnum: 1307  # Ezmerelda the cook
    extra_responses:
      - "Ezmerelda stirs a pot of something that stirs back. 'Don't ask what's in it. Asking is how the LAST cook ended up in it.'"
  - vnum: 1310  # the Jailor
    extra_responses:
      - "The Jailor says 'I hold the keys to cells holding things that eat keys. It's a delicate arrangement. Don't jostle it.'"

  # --- Mahn-Tor ---
  - vnum: 2333  # Mahn-Tor
    extra_responses:
      - "Mahn-Tor says 'A labyrinth is just a building that takes its privacy seriously. Mine takes it VERY seriously.'"
  - vnum: 2323  # the Minotaur Butler
    extra_responses:
      - "The Minotaur Butler says 'Master is not receiving guests. Master is receiving prey. The distinction is the whole tour, sir.'"
      - "The Minotaur Butler dusts a suit of armor that, on closer inspection, still has its previous occupant."

  # --- Haven / Glass Fort ---
  - vnum: 1809  # the master herbal
    extra_responses:
      - "The master herbal says 'Every cure here is also a poison at a different dose. So is the tea. So am I, frankly.'"

  # --- New Ofcol / Golden Citadel ---
  - vnum: 601   # Marshall Diana
    extra_responses:
      - "Marshall Diana says 'I keep the Citadel's peace. Peace is just violence that's been told to wait its turn.'"
  - vnum: 629   # Chaplain Jerrold
    extra_responses:
      - "Chaplain Jerrold says 'I bless the soldiers before battle and bury the ones the blessing didn't take. Busy schedule, the chaplaincy.'"

  # --- Plains / Valley / Redferne ---
  - vnum: 301   # Sorbus the Hermit
    extra_responses:
      - "Sorbus the Hermit says 'I came to the wilds for solitude and got an endless parade of heroes asking for directions. Hermitting's broken.'"
  - vnum: 7810  # an old hermit (Valley of the Elves)
    extra_responses:
      - "The old hermit says 'The elves here don't like outsiders. I'm an outsider who never left. We have a complicated arrangement and a lot of silence.'"
  - vnum: 7900  # the Grand Knight of paladins
    extra_responses:
      - "The Grand Knight says 'Honor is a heavy armor. Heavier than the steel. Most who try it on are crushed before the first charge.'"

  # --- Olympus minor gods ---
  - vnum: 903   # Hermes
    extra_responses:
      - "Hermes says 'Messages, commerce, thieves, and travelers. I'm the patron of everyone in a hurry, which out here is everyone about to die.'"
      - "Hermes is already three sentences into a different errand before you finish reading this one."
  - vnum: 902   # Prometheus
    extra_responses:
      - "Prometheus toys with a match and a grin. 'I gave mortals fire once. Look how that's going. Look how it's ALL going.'"

  # --- Galaxy ---
  - vnum: 9330  # Cassiopeia
    extra_responses:
      - "Cassiopeia says 'I was placed in the heavens for my beauty. Upside down, half the year, as a punishment for it. The gods are critics.'"
  - vnum: 9308  # poor Andromeda
    extra_responses:
      - "Andromeda rattles her chains. 'Chained to a rock as monster-bait, then immortalized in the sky STILL chained to the rock. Read the small print on heroics.'"
  - vnum: 9312  # the Pleiades
    extra_responses:
      - "The Pleiades murmur together. 'Seven sisters. Count us. Go on. Count again. Yes. That's the whole sad song right there.'"
  - vnum: 9310  # the wild Pegasus
    extra_responses:
      - "The wild Pegasus grazes, glances at your saddlebags, and makes it abundantly clear it is nobody's mount today."

  # --- Ultima cast ---
  - vnum: 2409  # Nystul the Librarian
    extra_responses:
      - "Nystul says 'The Codex of Ultimate Wisdom is in here somewhere. Probably. I reshelve by mood, not by Dewey.'"
  - vnum: 2417  # Chuckles the Jester
    extra_responses:
      - "Chuckles juggles, tumbles, and tells a joke so old Lord British groans before the punchline. 'TOUGH room. Castle full of 'em.'"
  - vnum: 2413  # Mr. Smithy the talking horse
    extra_responses:
      - "Mr. Smithy says 'Yes. A talking horse. We've both made our peace with it. Now are you riding or not?'"
  - vnum: 2454  # Shamino the Ranger
    extra_responses:
      - "Shamino says 'I've walked every wood in Britannia at the Avatar's side. The trick is loyalty. The other trick is good boots.'"
  - vnum: 2446  # Dupre the Paladin
    extra_responses:
      - "Dupre consults his maps. 'Honor, ale, and a good fight. In the right order that's a paladin. In the wrong order that's a Tuesday.'"

  # --- Gnome Village ---
  - vnum: 1505  # chief gnome
    extra_responses:
      - "The chief gnome says 'Recall doesn't work in half my village. The scientist swears it's a feature. The scientist also swears the explosions are features.'"
  - vnum: 1503  # gnome scientist
    extra_responses:
      - "The gnome scientist says 'It is NOT going to explode. Probably. The last six were learning experiences and I have LEARNED.'"

  # --- Sewer ---
  - vnum: 7042  # the guardian naga
    extra_responses:
      - "The guardian naga looks kindly at you. 'Most things down here want to eat you. I merely want you to leave politely. We're practically friends.'"

  # --- Mud School ---
  - vnum: 3719  # the priest of Hatchet
    extra_responses:
      - "The priest of Hatchet says 'Overlord Hatchet built this school to teach you to survive him. Read the irony slowly. It's the only lesson that sticks.'"


  # =========================================================
  #  BATCH 3 — joke zones & clan fixtures
  #  Note: merge entries accept either `vnum:` (single) or `vnums:` (group).
  #  A group's extra_responses are pooled into every member's base entry.
  # =========================================================

  # --- Arena: the roast fixtures (non-aggressive; keep it PG-13 trash talk) ---
  - vnum: 90    # Columbo
    extra_responses:
      - "Columbo says 'Biggest mouth on Chaosium, and I've got the duels lost to prove every word of it. Step up, I'll narrate your defeat AND mine.'"
      - "Columbo says 'I talk a huge game. The game is mostly talking. It's a complete game, technically.'"
  - vnum: 91    # Palon
    extra_responses:
      - "Palon says 'They call me a dumbass extraordinaire. The EXTRAORDINAIRE part is mine. I earned that. The rest is just jealousy.'"
      - "Palon walks confidently into the arena wall, nods like that was the plan, and turns around to try the door."
  - vnum: 92    # Fistan
    extra_responses:
      - "Fistan says 'I'll be good, I SWEAR, just let me back in the clan, I've changed, I've grown, I've — okay you walked off. Standard.'"
      - "Fistan says 'Every champion's got a comeback story. Mine's just the comeback part. On loop. Forever. Please.'"
  - vnum: 93    # Mdk
    extra_responses:
      - "Mdk says 'Official ambassador of pure Chaos to Chaosium, and let me tell you, the paperwork makes NO sense and I love it.'"
      - "Mdk says 'One minute I'm your best friend, next minute I'm your worst matchup. That's not a flaw, that's RANGE.'"

  # --- Vacation: the legally-distinct adorable creatures (group) ---
  - vnums: [6600, 6601, 6602, 6603, 6604, 6605, 6606, 6607]
    extra_responses:
      - "$n strikes a pose so marketable you can almost hear the cash register. It is level ninety. It would like you to forget that immediately."
      - "$n emits a single syllable of its own name, then freezes, suddenly aware of how much trouble the full name would cause. Legally. For everyone."
      - "$n does a little dance. The dance is free. Everything the dance is selling is very much not."

  # --- Teikoku support staff (group; the aggressive Masters are Boss Taunt material) ---
  - vnums: [9803, 9805, 9808]
    extra_responses:
      - "$n says 'The Masters do the killing. We do the filing, the laundry, and the convincing-ourselves-this-is-a-good-career.'"
      - "$n says 'You can't portal into the Masters' wing. You have to walk. Slowly. Past all of them. Enjoy.'"

  # --- Z-Rollers support staff (group) ---
  - vnums: [9907, 9906]
    extra_responses:
      - "$n says 'Rebound and Ricochet are off being terrifying. We hold the clipboard and the false hope. Both heavy.'"
      - "$n says 'Everything in here bounces back twice as hard. Including the regret. ESPECIALLY the regret.'"

  # --- Malokteri support staff (group) ---
  - vnums: [9506, 9507, 9508, 9509]
    extra_responses:
      - "$n says 'Azag, Kubabbar, Maklu, Sut Resi — say the names quietly. They have opinions about volume, and reach.'"
      - "$n says 'No gate in, no gate out. The clan likes its visitors committed. To the walk, at least.'"

```

# Boss Taunts (opt-in barks — NOT hints)

Everything below is **disabled by default.** These mobs are aggressive (or boss-grade sentinels) and the selection loop in *Suggested Code Behavior* skips them on purpose: a creature trying to kill you should not pause to give navigation advice. A designer who wants atmosphere can enable a single bark per boss by **explicitly overriding** the `actor_not_aggressive` guard for that vnum and setting `flavor_only: true`.

Rules for this whole section:

- These are **flavor barks, never hints.** They reveal nothing actionable — no target keywords, no no-recall warnings, no exact commands. That is what the prisoners and guards are for.
- One bark, then a long cooldown. A boss that monologues every round is a comedy, not a threat.
- Fire on **aggro/room-enter or first player `say`**, not on every keyword. Treat the keyword list as "any of these, occasionally," not a menu.
- Keep them in voice: short, mean, a little funny, a little doomed. Reproduce nothing from the source franchises — these are original lines about characters, not quotes of them.

```yaml
# boss_taunts — ALL ENTRIES flavor_only: true, disabled until designer override
area: _global_bosses
entries:

  # --- Apocalypse HQ: the clan's killers ---
  - vnum: 800
    actor: Larsen the Cruel
    flavor_only: true
    keywords: [larsen, apocalypse, mercy, leave]
    responses:
      - "Larsen the Cruel says 'You found the front door. I admire that. I'll mourn it too, a little.'"
      - "Larsen the Cruel says 'Everyone here works for me. Even the part of you that's already deciding to run.'"
  - vnum: 801
    actor: Tarrant the Shitter
    flavor_only: true
    keywords: [tarrant, smile, apocalypse]
    responses:
      - "Tarrant smiles the way a closing door smiles. 'Stay. Briefly. Loudly.'"
  - vnum: 802
    actor: Kyriel the Mage
    flavor_only: true
    keywords: [kyriel, mage, magic, apocalypse]
    responses:
      - "Kyriel doesn't smile, doesn't threaten, doesn't blink. The spell is already half-cast before you finish reading this."
  - vnum: 803
    actor: Dorin the Assassin
    flavor_only: true
    keywords: [dorin, assassin, duel, apocalypse]
    responses:
      - "Dorin says 'I duel because murder felt too honest. Don't worry — I'll be honest with you.'"
  - vnum: 805
    actor: Ingen Jegger
    flavor_only: true
    keywords: [ingen, shadow, hunt, apocalypse]
    responses:
      - "Ingen Jegger steps out of a shadow you'd have sworn was empty. He answers to those, not to names."

  # --- Ultima: the Triad of Evil, the Guardian, the fallen regent ---
  - vnum: 2460
    actor: Mondain
    flavor_only: true
    keywords: [mondain, gem, triad, evil, sosaria]
    responses:
      - "Mondain grips his gem of immortality. 'The Avatar broke this once. You are not the Avatar.'"
  - vnum: 2459
    actor: Minax
    flavor_only: true
    keywords: [minax, triad, evil, mondain, time]
    responses:
      - "Minax says 'My master fell. I learned from it. The lesson was: kill the hero earlier.'"
  - vnum: 2458
    actor: Exodus
    flavor_only: true
    keywords: [exodus, triad, evil, island, machine]
    responses:
      - "Exodus is neither man nor daemon but the ugly math of both, and it has already counted you out."
  - vnum: 2456
    actor: The Guardian
    flavor_only: true
    keywords: [guardian, red, face, britannia]
    responses:
      - "The Guardian fills the room with a great red face and a voice like a landslide. 'Welcome, Avatar. I have been so looking forward to this.'"
  - vnum: 2457
    actor: Blackthorn
    flavor_only: true
    keywords: [blackthorn, throne, virtue, regent]
    responses:
      - "Blackthorn says 'I ruled in his absence and they called it tyranny. I call it attendance.'"

  # --- Marquee dragons and monsters ---
  - vnum: 2220
    actor: Tiamat
    flavor_only: true
    keywords: [tiamat, dragon, heads, queen]
    responses:
      - "Tiamat regards you with five heads and zero patience. One of them is already deciding which way you'll fall."
  - vnum: 5109
    actor: the yochlol
    flavor_only: true
    keywords: [yochlol, lolth, spider, drow, demon]
    responses:
      - "The yochlol boils between spider and woman and ooze, all of them displeased to share a room with prey."
  - vnum: 1806
    actor: the Snow Queen
    flavor_only: true
    keywords: [queen, snow, cold, ice, invite]
    responses:
      - "The Snow Queen says 'You were not invited. The cold, however, always is.'"
  - vnum: 305
    actor: Shudde-M'ell
    flavor_only: true
    keywords: [shudde, worm, gharne, treasure, burrow]
    responses:
      - "Shudde-M'ell coils over its hoard, a giant worm older than the words for it, and the ground complains under its weight."
  - vnum: 2103
    actor: the gang leader
    flavor_only: true
    keywords: [gang, leader, boss, hideout, turf]
    responses:
      - "The gang leader says 'You found my hideout. Bad news: finding it was the easy part, and you've already done it.'"

  # --- Limbo: the swordsman who shouldn't be chatted with ---
  - vnum: 2
    actor: the Hitokiri Battousai
    flavor_only: true
    keywords: [battousai, manslayer, sword, limbo, blade]
    responses:
      - "The manslayer doesn't speak. The blade clears the sheath before the syllable would have landed, and that is his whole answer."
```

# Easter Eggs (rare, opt-in, low-weight)

The flavor pools above are the everyday voice of the world. This section is the **rare** layer — longer, weirder, more memorable lines that should feel like a *discovery*, not a default. All original writing; any wink at an outside reference is a wink only, never a quote.

Two firing modes:

```yaml
easter_egg_config:
  # Mode 1: secret trigger words. These ALWAYS fire when a non-hostile NPC hears them.
  #         The discovery is knowing the word exists at all. Tier 0, pure flavor.
  # Mode 2: rare_pool. Longer lines layered UNDERNEATH every normal pool at low weight.
  #         ~96% of the time the player gets the normal area/role/universal line;
  #         ~4% of the time a non-hostile NPC reaches for one of these instead.
  rare_pool_chance: 0.04
  rare_pool_cooldown_seconds: 600   # so a "discovery" stays rare even under spam
  applies_to: non_hostile_only
```

## Secret trigger words

```yaml
area: _easter_eggs
secret_triggers:

  - phrase: [xyzzy, plugh]
    responses:
      - "$n perks up. 'Now THAT'S an old word. Older than this realm. A hollow voice says \"fool\" and nothing else happens. It never does.'"

  - phrase: [marco]
    responses:
      - "$n sighs, glances around, and mutters '...polo,' against every instinct of dignity it possesses."

  - phrase: [knock knock]
    responses:
      - "$n says 'Who's there. No — don't. I've heard them all. I've heard them DURING sieges. Just tell me what you want.'"

  - phrase: [meaning of life, the answer, why are we here]
    responses:
      - "$n thinks a long while. 'I asked an immortal that once. They said the answer was a number, laughed, and rebooted the world. So. Inconclusive.'"

  - phrase: [are you real, am i real, is this real]
    responses:
      - "$n looks at you very steadily. 'You're asking a man made of words whether HE'S real. Friend, we are both in deeper than you think. Have an ale.'"

  - phrase: [open sesame, abracadabra, alakazam]
    responses:
      - "$n waits. Nothing opens. 'Magic words work for wizards. For you they're just talking. Loudly. At a door.'"

  - phrase: [up up down down, konami, cheat code]
    responses:
      - "$n says 'There's no secret code. There WAS, once, an immortal swore. Then they patched it and the immortal vanished. Coincidence, surely.'"

  - phrase: [press f, rip, gg]
    responses:
      - "$n removes its hat for exactly one respectful second, then puts it back. 'Someone, somewhere, just died doing something stupid. We honor them by not doing the same. We will, though.'"

  - phrase: [hello world]
    responses:
      - "$n says 'Funny thing to say. That's the first thing every newborn world hears, they say. The coders whisper it before the rest of us exist.'"

  - phrase: [the cake, is the cake a lie]
    responses:
      - "$n says 'There is no cake. There has never been cake. Whoever promised you cake was running a dungeon, not a bakery. Run.'"

  - phrase: [sudo, root, admin]
    responses:
      - "$n laughs once, sharp. 'Oh, you want PERMISSIONS. Sweetheart, the only one with root access here lives above the sky and does not take requests.'"
```

## Rare pool (longer flavor, fires ~4%)

```yaml
area: _easter_eggs
rare_pool:
  responses:
    - "$n stops, looks at you a beat too long, and says 'You know, I've stood in this exact spot for what feels like years. Heroes come, heroes go, the seasons never quite change. Sometimes I wonder if any of us ever really leave the room we were made in.' Then it shrugs and the moment's gone."
    - "$n leans in conspiratorially. 'Between us? I don't think the gods of this place are as in charge as they let on. Half the disasters round here have the distinct smell of someone fixing a problem and making three new ones. Don't quote me. Especially not to an immortal.'"
    - "$n says 'I'll tell you the real secret of surviving out here, since you asked nicely and I'm bored. It isn't the sword. It isn't the spell. It's leaving. Knowing the exact moment to stop being brave and start being elsewhere. Nobody ever writes songs about that one.'"
    - "$n squints at your gear, your scars, and the particular way you carry yourself. 'You've died before. Don't bother denying it — everyone worth talking to has. You come back a little more careful and a little more reckless at the same time. It's the strangest thing to watch, over and over, from a fixed point.'"
    - "$n says 'There's a room somewhere in this realm with no exits and no monster and nothing in it but a single chair. I've never found it. I just know it's there, the way you know a missing tooth. If you find it, sit down a minute. Somebody built it for a reason and then forgot why.'"
    - "$n watches a leaf, or a spark, or a bit of nothing drift past. 'Every name in this world means something to somebody who isn't here anymore. The ones who made us tucked little jokes and grudges and love letters into every signpost. You're walking through someone's diary and calling it a dungeon crawl.'"
    - "$n says 'Want to know what's on the other side of the sky? Same thing that's underneath the floor. The edge of what got built. Don't go looking. The brave ones who reach the edge come back quiet, and the quiet ones don't come back at all.'"
    - "$n grins. 'You ever notice the monsters respawn but the dead heroes mostly don't? Tells you who the management actually values. We're the permanent cast. You lot are seasonal labor with very good swords.'"
```

## Signature eggs (per-vnum, rare)

A handful of named characters get one unique rare line. Same low-weight firing as the rare pool, merged by vnum.

```yaml
area: _easter_eggs_signature
signature_eggs:
  - vnum: 1     # Puff
    responses:
      - "Puff uncoils across the static between worlds and, for one impossible moment, the gaps in reality line up into something almost like a sentence: that this place was loved into being, frays a little more each year, and is held together mostly by the fact that people keep coming back. Then it's just a fractal dragon again, and you're just standing there."
  - vnum: 2464  # Lord British
    responses:
      - "Lord British studies you with the patience of a man who has been king across more ages than he cares to total. 'They say a true ruler outlives every threat to his realm. I have outlived three. I have also outlived nearly everyone who ever loved me for reasons other than the crown. Choose your immortality carefully, adventurer. It is lonelier than the songs admit.'"
  - vnum: 3011  # the vortex guardian (Midgaard, lvl 100 sentinel)
    responses:
      - "The vortex guardian's voice arrives from a direction that isn't one. 'I guard the seam where this world was stitched to the next. Do you understand how thin it is here? One bad pull and the whole tapestry runs. So I stand. Forever. And you wanted to ask me for directions. Go on, then. Ask. It's the most normal thing that's happened to me in an age.'"
  - vnum: 9402  # Foreman Floyd (Mob Factory) — fourth wall finale
    responses:
      - "Foreman Floyd sets down his clipboard. 'You really want the truth, kid? Every goblin, every kobold, every nameless thing you've ever put a sword through — it clocked out of THIS factory. Had a number. Had a shift. We stamp 'em, we ship 'em, you slay 'em, and the line never stops. Don't feel bad. They don't. They're built not to. ...Most of 'em.' He picks the clipboard back up. 'Anyway. Quota.'"
```


## Per-area secret words

Each iconic zone gets its own local secret trigger — same always-fires rule as the global secret words, but it only works **inside that area**, so every zone hides one small reward for the curious. Tier 0, original writing, never a hint.

```yaml
area: _area_eggs
area_secret_triggers:

  - area: midgaard
    phrase: [fido, puff the dog, good boy]
    responses:
      - "$n says 'You're trying to befriend the temple dog. Everyone tries. Fido has buried better heroes than you in that flowerbed. Good luck.'"

  - area: shire
    phrase: [second breakfast, elevenses, supper]
    responses:
      - "$n brightens enormously. 'NOW you're speaking sense. Breakfast, second breakfast, elevenses, luncheon — a proper day has more meals than a hero has lives. Sit. Eat. Stop adventuring for once.'"

  - area: olympus
    phrase: [ambrosia, nectar, feast]
    responses:
      - "$n says 'Ambrosia is for the deathless. One bite and you'd either ascend or burst, and the smart money is on burst. The cheese plate, however, is mortal-safe. Mostly.'"

  - area: galaxy
    phrase: [wish, shooting star, make a wish]
    responses:
      - "$n says 'Wish on a star out here and the star wishes right back. Be sure you want what you ask for — half these constellations are someone's wish that came true the wrong way.'"

  - area: ultima
    phrase: [virtue, codex, ankh]
    responses:
      - "$n traces an ankh in the air. 'Truth without Love is cruelty. Love without Courage is empty. Courage without the other two is just a loud way to die. The Codex says it prettier. It usually does.'"

  - area: mahntor
    phrase: [thread, string, which way out]
    responses:
      - "$n says 'Smart ones bring a thread to find the way back out of the maze. The maze knows this. The maze has eaten a great deal of thread, and the people holding it.'"

  - area: moria
    phrase: [deep, dig too greedily, dwarves]
    responses:
      - "$n says 'The old dwarves dug deep here. Then they dug deeper. Then they dug into something that had been perfectly happy being left alone. The moral is in the silence. Listen to it.'"

  - area: hitower
    phrase: [spellbook, forbidden, top floor]
    responses:
      - "$n says 'The good spellbooks are on the top floor. So is the Grand Mistress, the worst of the wards, and the reason the LAST curious visitor is now a decorative scorch mark. Browse responsibly.'"

  - area: gnome
    phrase: [explode, kaboom, big red button]
    responses:
      - "$n says 'There's a big red button in the lab. The scientist says it's perfectly safe. The scientist also no longer has eyebrows. Connect those two facts at your leisure.'"

  - area: vacation
    phrase: [gotta catch, catch them all, pocket]
    responses:
      - "$n freezes mid-pose. 'We do not say the whole phrase here. Lawyers, friend. Terrible, tireless lawyers. Smile for the camera and DON'T finish that sentence.'"

  - area: arena
    phrase: [rematch, again, run it back]
    responses:
      - "$n says 'Rematch? That's the sweetest word in the arena. It means somebody learned NOTHING and brought gold anyway. Step up. We're delighted to host the lesson twice.'"
```

# Keyword Alias Table

Use these alias groups globally so designers don't duplicate every phrasing on every NPC. An entry's `keywords:` list may name an alias group (e.g. `[heal, travel]`) and the matcher expands it. Player text is normalized (lowercased, punctuation stripped, plurals folded) before matching.

```yaml
aliases:
  # --- core utility ---
  heal:      [heal, healing, cure, hp, hurt, wounded, poison, blind, curse, disease, rez, resurrect]
  travel:    [travel, path, road, route, exit, way, lost, where, recall, portal, gate, return, home]
  gear:      [gear, equipment, weapon, weapons, armor, armour, hitroll, damroll, ac, saves, wear, wield]
  training:  [train, training, practice, skill, spell, learn, master, research, level, advance]
  danger:    [danger, boss, monster, guard, warning, death, kill, deadly, careful, trap]
  lore:      [story, history, rumor, gossip, legend, song, lore, tale, who, what]
  commerce:  [buy, sell, list, value, shop, wares, gold, coin, price, trade, deal]
  # --- movement / world-fact aliases (tie to Verified World Facts block) ---
  flight:    [fly, flight, air, float, levitate, sky, wings]
  water:     [boat, ship, swim, water, river, sea, ocean, raft, sail]
  norecall:  [recall, norecall, stuck, trapped, cant recall, no recall, walk out]
  # --- class aliases (Static Chaos custom classes) ---
  class_sorcerer: [sorcerer, chant, magic, mystic, black, white, neutral, shaman, robe]
  class_patryn:   [patryn, rune, runeweave, tattoo, ward, sigil, sartan]
  class_saiyan:   [saiyan, ki, rage, kamehameha, kiwave, kiwall, transform, super]
  class_fist:     [fist, combo, ki, stance, phoenixaura, martial, monk]
  class_mazoku:   [mazoku, demon, essence, reform, morph, charge, dark]
```

# Suggested Code Behavior

## Response selection

```pseudo
on_speech(speaker, command, text, room):
    if command not in ["say", "yell"]: return
    normalized = normalize(text)               # lowercase, strip punctuation, fold plurals
    listeners = room.people if command == "say"
                else same_area_nonhostile_npcs(room.area)

    responders = 0
    for npc in listeners:
        if npc.is_fighting: continue           # busy dying or killing
        if npc.is_aggressive and not npc.flavor_only_override: continue
        if cooldown_active(npc, speaker): continue

        entry = lookup_dialogue_entry(npc.vnum, npc.area, npc.role)
        if not entry: continue

        matched = find_keyword_group(entry, normalized)   # expands alias groups
        if not matched: continue

        response = weighted_random(entry.responses[matched])
        npc.say(substitute_tokens(response, npc, speaker))  # $n, $N, etc.
        set_cooldown(npc, speaker)             # per-player, per-NPC

        responders += 1
        if command == "say": break             # one speaker answers a say
        if responders >= YELL_REPLY_CAP: break  # cap area-wide yell replies (e.g. 3)
```

## Avoid over-hinting

Use tiers so most NPCs stay flavorful and only a few are spoilery:

```yaml
hint_tier:
  0: flavor only            # mood, jokes, in-character noise
  1: general area warning   # "this place hates recall"
  2: vague nav/combat hint  # "the soft target is the small ugly word, not the fancy name"
  3: exact keyword/command  # "curse blocks recall; flee first"
```

Most NPCs should sit at tier 0–2. Reserve tier 3 for the tutorial (Mud School), prisoners/slaves, shopkeepers, and the handful of spots where the original Static Chaos keywords are genuinely misleading.

## Suggested exact-command hints

Worth allowing because they cut frustration without spoiling exploration. All verified against the current codebase behavior:

```yaml
- trigger: [potion, quaff]
  line: "Potions are quaffed. Pills are eaten. Scrolls are recited. Wands are zapped. Staves are brandished."
- trigger: [recall, curse, norecall]
  line: "Curse blocks recall, and so do no-recall rooms. If you're fighting, flee first — recall won't fire mid-swing."
- trigger: [target, keyword, name]
  line: "Target keywords aren't always the fancy name you see. Try the small ugly words underneath it."
- trigger: [gear, stat]
  line: "Gear moves hp, mana, move, AC, hitroll, damroll, and saves. Training handles the rest."
- trigger: [flight, air]
  line: "Some rooms are open air — no floor, no mercy. You need to fly, or you fall."
- trigger: [water, boat]
  line: "Deep water won't let you swim it. Bring a boat, or bring wings."
```

# Implementation Priority

1. Midgaard, Mud School, and the core tutorial hints — first contact matters most.
2. Prisoners and slaves who reveal local target keywords or no-recall danger.
3. Area rulers and guardians: flavor plus vague warnings.
4. Shopkeepers, trainers, and practice NPCs.
5. Joke-area flavor (Mob Factory, Vacation, Rat Lair, Galaxy).
6. `yell`-based area-wide responses — turn these on only after same-room `say` triggers are stable, and keep the reply cap low.
7. Boss Taunts — last, optional, and only if a designer deliberately wants the atmosphere.

# QA Checklist

- NPCs never answer while fighting.
- Aggressive mobs stay silent unless a designer explicitly set `flavor_only` + the override.
- `yell` never spams the whole area — the reply cap holds.
- Response cooldown is per-player **and** per-NPC, so two players can't desync it and the same NPC can't be milked.
- Alias expansion works: an NPC keyed to `[heal]` answers "cure," "wounded," "poison," etc.
- Hint lines never reveal inaccessible or unimplemented content unless that's intentionally diagnostic.
- Boss barks reveal **no** actionable info — they're tier 0 only.
- World-fact hints match the Verified World Facts block: recall room 3001, fly-required air rooms, boat/flight water rooms, and the no-recall zone list.
- Every tier-3 command hint is correct in the current codebase.
- Lines fit the tone: short, rude, funny, ominous, or practical — and reproduce nothing from any source franchise.
- The Limbo caricature mob (vnum 20) has no dialogue and is not referenced anywhere.
- Easter eggs stay rare: secret trigger words fire only on their exact phrase, and the rare pool respects its low chance and long cooldown so "discoveries" don't become spam.
- Eggs are non-hostile only and tier 0 — no egg reveals a hint, target keyword, or exact command.
- Per-area secret words only fire inside their own area; a Shire secret says nothing in Olympus.
