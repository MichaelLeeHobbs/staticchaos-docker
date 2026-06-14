# Mobile Suits — Pilot's Guide
> Climb into a towering Gundam-style mecha, load it for war, and trade your body for tons of armor and heavy weapons.

## Overview

**Mobile suits** are a system layered on top of your class: *any* character can climb into one and become a walking war machine. While you're piloting, you stop being a Saiyan or a Sorcerer for combat purposes and instead fight as the **suit** — with its own armor, weapons, ammunition, fuel, and a separate set of **pilot skills**.

Most mobile suits belong to the **clans** — each clan keeps its fighting suits in a **hangar** inside its headquarters, and the planet defensors lock you out of any hangar that isn't your clan's. But you don't need a clan to *start*: a public **trainer Leo** (with a stack of ammo) waits in the **Temple of Midgaard** for anyone to climb into — see *Getting started* below. The bigger suits are what you grow into as a clan member.

A suit is **temporary power**. You `mount` to climb in, `leave` to climb back out (which parks the suit in the hangar again), and if you take enough punishment the suit is **wrecked** and throws you out half-dead. Nothing about a suit is permanent except the **pilot skills** you build — those stay with your character even after you leave the suit.

## How piloting works

Five things drive a suit; keep them straight:

**Armor — your damage buffer.**
Every suit has an armor rating that does two jobs. First, it **soaks a percentage of every hit** before anything else (a heavy suit can shrug off most of the damage thrown at it; soak is capped at 75% so nothing is ever truly invulnerable). Second, it gives you an **armor pool** that absorbs the damage that gets through. When that pool is emptied, a **random part of the suit is damaged**, the pool refills, and *you personally* take a jolt of damage in the cockpit. Lose all twelve parts and the suit explodes.

**Condition — the twelve parts.**
As the pool breaks, parts get knocked out one at a time (torso, head, camera, legs, arms, guidance, radar, propulsion…). A `status` check shows your overall condition. Damage doesn't reduce your stats much on its own, but two things care about it: a heavily-damaged suit can't **cloak**, and a totally-wrecked suit (all twelve parts) **explodes**.

**Ammunition — four kinds.**
Suit weapons don't run on mana — they run on physical ammo: **bullets**, **shells**, **plasma**, and **beam** energy, plus **missiles**. A freshly-parked suit usually sits with **empty magazines**, so before you go to war you `load` munitions into it. Each weapon draws from its own ammo type and has its own cost per shot.

**Fuel — for flying.**
`fly` lets a suit move between rooms and chase or flee, but it burns **fuel**. Run dry and you're grounded.

**Pilot skills — how good you are in the cockpit.**
Four skills decide what you can pilot and how hard you hit: **Aim** (gunnery), **Weapon** (beam-melee), **Combat** (punching), and **Reflex** (dodging/parrying). They go up as you *use* them in battle (see below), and they gate which suits you're allowed to mount.

## Pilot skills & climbing the ladder

You raise the four pilot skills by **fighting in a suit against monsters** (not other players):

| Skill | Raised by | Used for |
|---|---|---|
| **Aim** | `fire`-ing ranged weapons at enemies | All gun/beam/missile weapons |
| **Weapon** | `slash`-ing with a beam melee weapon | Beam sabres, swords, scythes, etc. |
| **Combat** | `punch`-ing | Unarmed/brawling strikes |
| **Reflex** | successfully dodging/parrying incoming hits | Avoiding damage |

Each suit has a **requisite** — the pilot-skill level you need in **all four** skills before you're allowed to climb in. So a well-rounded pilot who has trained gunnery *and* melee *and* dodging gets access to the bigger suits; a one-trick pilot stays in the small ones. (Skill gain is also paced by your overall character development, so growing your character helps your piloting cap rise too.)

> **Tip:** start in the **Leo** — the no-requisite trainer that's free in the Temple of Midgaard — and grind your four skills up in it before stepping into a clan Gundam. You keep the skills when you swap suits.

## The suit roster

Suits roughly tier by their **requisite**. Higher tiers hit harder and soak more, at the cost of needing a more skilled pilot.

| Suit | Requisite | Toughness (melee soak) | Notes |
|---|---:|---:|---|
| **Leo** | 0 | 29% | The trainer — **free in the Temple of Midgaard**. Anyone can pilot it; build your skills here. |
| **Aries** | 100 | 21% | Light, fast, fragile; carries missiles. |
| **Taurus** | 400 | 36% | Beam-rifle skirmisher. |
| **Virgo** | 750 | 43% | Defensor shields + beam cannon. |
| **Mercurius** | 1000 | 50% | Shield-heavy defensive suit. |
| **Vayeate** | 1000 | 50% | Beam-cannon glass cannon. |
| **Tallgeese** | 1200 | 43% | Blisteringly fast all-rounder; a great stepping stone to the Gundams. |
| **Wing Gundam** | 1400 | 50% | Buster rifle; balanced Gundam. |
| **Deathscythe** | 1400 | 43% | Beam scythe + **cloak**; the ambusher. |
| **Heavyarms** | 1400 | 57% | Gatling, miniguns, and a huge **16-missile** salvo. |
| **Sandrock** | 1400 | 71% | The tank — heaviest armor of the fielded suits; heat shotels. |
| **Shenlong** | 1400 | 43% | Dragon-fang melee bruiser, highest melee bonuses. |
| **Wing Zero** | 2000 | 57% | Elite. Twin buster rifle. Not in normal circulation. |
| **Epyon** | 2000 | 57% | Elite melee monster (beam glaive + heat rod). Not in normal circulation. |
| **M1A1 Tank** | 2000 | 75% | Heaviest armor in the game; never released. |

Which suits your clan actually fields varies by faction — **check your clan hangar** to see what's parked there. The two-thousand-requisite suits (Wing Zero, Epyon, M1A1) are elite/unreleased and aren't part of normal play.

## Weapons & firing

Suits carry weapons as **equipment bits** the suit is built with. You pick one to be your active weapon with `ready`, then attack:

| Command | What it does |
|---|---|
| `ready <weapon>` | Make a weapon active (the one `fire`/`slash` will use). |
| `fire [target/direction]` | Fire your readied ranged weapon. Some weapons hit one target; others (cannons, vulcans, Doberguns) shoot **down a hallway**, hitting rooms in a direction. Costs ammo. |
| `slash [target]` | Strike with a readied **beam melee** weapon (sabre, sword, scythe, glaive…). Costs a little beam/plasma; big damage multipliers. |
| `punch [target]` | Unarmed melee. No ammo cost — your fallback when you're dry. |
| `salvo [direction]` | Launch your missiles — count depends on the suit (Heavyarms fires the most). Spans rooms. |
| `shell [direction]` | Dobergun artillery: lob shells up to **50 rooms** away. |
| `load <munition>` | Load ammo (bullets/shells/plasma/beam/missiles) into the matching magazine. |
| `status` | Show armor, fuel, every ammo count, your readied weapon, and your pilot skills + suit condition. |

**Ranged vs. melee:** gunnery (`fire`) raises **Aim** and lets you fight at range and across rooms; beam melee (`slash`) raises **Weapon** and hits hardest up close. Most pilots train both so they qualify for the bigger suits.

## Taking damage, repair & destruction

- **Soak first.** Your armor rating absorbs a percentage of every hit (up to 75%). The rest chips your armor pool.
- **Part damage.** When the pool empties, a random part breaks, the pool refills, and you take a burst of cockpit damage. Expect to lose a chunk of your own HP over a long fight even inside a healthy suit — bring a real health bar.
- **Field repairs.** Carry a **field repair kit** and `install` it to refill your armor pool and patch **one** damaged part. Install other **accessories** the same way to add equipment your suit is built to accept.
- **Destruction.** Lose all twelve parts and the suit explodes — you're thrown clear but left at a sliver of HP. The suit is gone (not re-parkable); you'll need another from the hangar.
- **Weaknesses.** Suits take **double damage from lightning**, and the **wind** school of magic also hits them double. A caster who leans into those elements is the natural counter to an armored column — and the reason pilots don't rule unopposed.

## Defensive systems

- **Shields / defensors** (Virgo, Mercurius, and others): burn **plasma** to deflect a share of incoming bullets and shells.
- **Cloak** (Deathscythe): hides you from the room — devastating for ambushes — but **fails once your suit is badly damaged**, so cloak early.
- **Dodge & parry:** fast suits (high speed + Reflex) dodge shots and parry melee. You can only parry if you've got a melee weapon equipped; successful dodges/parries raise your **Reflex**.

## Core commands

| Command | What it does |
|---|---|
| `mount <suit>` | Climb into a suit (in your clan hangar, if you meet its requisite). |
| `leave` | Climb out and park the suit (hangars hold up to four). |
| `fly <direction>` | Move/chase/flee using fuel. |
| `ready <weapon>` | Set your active weapon. |
| `fire` / `slash` / `punch` | Attack (ranged / beam-melee / unarmed). |
| `salvo` / `shell` | Missiles / long-range artillery. |
| `load <munition>` | Reload an ammo type. |
| `install <item>` | Install a field repair kit or accessory. |
| `status` | Full readout: armor, fuel, ammo, weapon, condition, pilot skills. |

## Getting started

> Your first suit is waiting — a trainer **Leo** and a stack of **105mm bullets** sit in the **[Temple of Midgaard](#/map?room=3001)**, the room you recall to. No clan required to pilot the Leo.

1. **Recall to the Temple.** Type `recall` (or you'll already start there). A Leo mobile suit and 105mm bullets are on the floor.
2. **Grab ammo, then mount.** `get bullets`, then `mount leo` — you don't pick the suit up; `mount` climbs into the one standing in the room.
3. **Load and ready.** `load bullets` adds 100 rounds; then `ready machinegun` so `fire` has something to shoot.
4. **Train your four skills.** `punch` costs no ammo and raises **Combat**; `fire` at monsters raises **Aim**; getting hit (and dodging) raises **Reflex**; a beam-melee `slash` raises **Weapon**. Watch them climb on `status`, and grab a fresh stack of bullets from the Temple whenever you run dry.
5. **Join a clan and step up.** The real suits — Tallgeese and the Gundams — live in your clan's hangar (find it on the **[World Map](#/map)**). Once all four pilot skills clear a suit's requisite, `mount` up there.
6. **Stay topped off.** Keep ammo loaded, fuel up before flying, and carry a field repair kit for long fights.

## Tips

- **You keep your pilot skills forever** — time spent grinding a Leo is never wasted.
- **Bring HP.** The cockpit takes damage every time your armor pool breaks; a suit doesn't make you immortal.
- **Mind the counters.** A lightning/wind caster eats suits alive — don't assume armor wins every fight.
- **Cloak early** (Deathscythe) — once you're beaten up it stops working.
- **`status` constantly.** It's your fuel gauge, ammo count, damage report, and skill tracker in one.
