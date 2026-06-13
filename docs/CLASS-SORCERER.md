# Sorcerer — Player Guide
> Weave chants, master the schools of magic, and rain elemental ruin from a distance.

## Overview

The Sorcerer (also called a chanter) is Static Chaos's dedicated spellcaster. Instead of trading blows, you **chant** spells: you speak the words of power, the magic gathers, and then it unleashes. Some chants fire instantly; the big, multi-line incantations take a moment to build before they land.

Your strength comes from study, not from your sword. You **research** seven schools of magic, **specialize** into one of three magical paths, and spend a hard-won currency called **primal** to push your schools ever higher. A maxed-out specialist drops devastation that few classes can match — but a Sorcerer caught with depleted reserves or pinned in melee is fragile.

Your governing stat is **Will**, which is your highest attribute. Will fuels how much magic you can hold and how reliably your chants overcome an enemy's resistance.

## How your power works

There are three things to keep straight: **Mystic**, **primal**, and your **school ranks**.

**Mystic — your spellcasting fuel.**
Every chant costs Mystic equal to its rank (a rank-40 chant costs 40 Mystic). Mystic is an ammo pool: you must be able to pay a chant's full cost up front, or you simply can't cast it ("Your Mystic is too depleted to weave that chant."). Your maximum Mystic equals your Will.

Crucially, **a chant always lands at the full power of your school rank** — it is *not* weakened by how much Mystic you have left. As long as you can afford the cost, an expensive chant hits just as hard on your last cast as your first. Run dry, and you simply can't cast until you recover.

Mystic regenerates on its own over time, faster the higher your Will. To recover much faster, use **concentrate** (see commands): while concentrating you pour ordinary **mana** into your Mystic pool each tick. This is how you top off between fights. If your mana runs out, concentration drops automatically.

**Primal — your progression currency.**
Primal is earned by slaying worthy foes. You spend it on **research** to raise your school ranks and on **specialize** to change your path. Higher ranks cost more primal each time, so primal is always in demand.

**School ranks — your actual spell power.**
Each of the seven schools (Black, Earth, Wind, Fire, Water, Astral, White) has a rank. A school's rank determines which chants in that school you can cast and how hard they hit. Raise ranks with `research`.

There is a catch: a school is at **full strength only if it's part of your specialty.** Schools outside your specialty are **capped at rank 44** — the points above the cap aren't destroyed, they just go dormant. If you later specialize back into that school, they return at full strength. Your specialty schools can climb all the way to **rank 50**.

## Specializations (Black / White / Shamanism)

Every Sorcerer picks one of three paths with the `specialize` command. Your first choice is free; **changing it later costs 250 primal** (your old ranks survive the switch, just dormant while out of spec).

- **Black magic** — *the damage and drain path.* Black is raw destruction: shadow dragons, negative-energy bolts that heal you for the damage dealt, and lightning pentagrams. Its capstone is the **Laguna Blade** at Black rank 50 — a short-lived blade of black energy that hits enormously hard but drains Mystic every moment you hold it, and prevents you from weaving other complex chants. When your Mystic runs dry, the blade shatters. The signature **Dragon Slave** (rank 50) is also Black.
- **White magic** — *the sustain and anti-magic path.* White keeps you and your group alive and dismantles enemy magic: **Recovery** (heal-over-time), **Dicleary** (heal + refresh), **Holy Resist** (a sturdy protective barrier), **Resurrection**, and the all-important **Flow Break** — which strips an enemy's buffs, collapses holy-resist barriers, shatters a Laguna Blade or Recovery, and can disrupt an enemy Sorcerer mid-chant.
- **Shamanism** — *the elemental path, and the broadest of the three.* Specializing in shamanism makes **all four elemental schools — Earth, Wind, Fire, and Water — count as full-strength specialty schools** (each able to reach rank 50). This gives you by far the widest spellbook: fire lances and infernos, ice lances and crystals, lightning, wind blasts that disrupt foes, earthquakes, and the ultimate **Ra-Tilt** (Astral rank 50). The trade-off is breadth over a single specialized peak.

## Core commands

| Command | What it does |
|---|---|
| `chant <spell> [target]` | Cast a chant. With no target in combat, it hits whatever you're fighting. |
| `chant list` | List every chant you can currently cast, with its school, required rank, and how many lines it takes. |
| `chant info <spell>` | Show full details for one chant: school, rank/Mystic cost, target type, length, and your effective rank in that school. |
| `research` | List the seven schools you can study. |
| `research list` | Show your rank, cap, and the primal cost of the next rank in every school. |
| `research <school>` | Spend primal to raise that school by one rank (Black, Earth, Wind, Fire, Water, Astral, White). |
| `specialize` | Show your current specialization. |
| `specialize <path>` | Choose or change your path: `black magic`, `white magic`, or `shamanism`. First pick free; later changes cost 250 primal. |
| `concentrate` | Toggle deep focus to rapidly refill Mystic by burning mana each tick. Toggle off to stop. |
| `prepare <school>` | Set a school so you can quickly cast its strongest prepared attack; `prepare none` to clear. |

**Typing spell names:** spell names are matched as a single word. A two-word chant like "rubyeye blade" should be typed with a hyphen — `chant rubyeye-blade` — so the game doesn't mistake the second word for a target. Prefixes and partial names work as long as they're unambiguous; if they aren't, the game lists the candidates for you.

## Getting started

1. **Pick a path early.** Run `specialize` to see your options, then commit — for example, `specialize shamanism`. Your first choice is free, so choose the playstyle you want (burst damage, support, or elemental variety).
2. **See what you can cast.** `chant list` shows everything currently available to you. Start with the low-rank chants in your school(s).
3. **Learn your spells.** Use `chant info <spell>` to check a chant's cost, target type, and how many lines it takes before it lands.
4. **Fight and earn primal.** Kill worthy enemies to bank primal.
5. **Pour primal into your schools.** Use `research list` to see costs, then `research <school>` to climb. Focus your specialty schools first — they go to 50 and stay at full power.
6. **Manage your Mystic.** Between fights, `concentrate` to refill fast (it eats mana). In a fight, watch your pool — if it's too low to afford a chant, you can't cast it.

## Strengths & weaknesses

**Strengths**
- Enormous ranged and area damage, especially from a maxed specialty.
- Chant power scales with mastery, so spells stay strong even as your Mystic drains.
- Three very different playstyles, plus the ability to respec without losing your hard-earned ranks.
- White's Flow Break is premier anti-magic; Black self-heals through Hell Blast; Shamanism brings unmatched spell variety.
- Reliable spell landing once specialized — a specialist's chants are far harder to resist.

**Weaknesses**
- Casting is gated by Mystic. Tapped out means you can't chant at all until you recover.
- Big multi-line chants take time to build, and you can only hold one complex chant at a time.
- A gathering chant can be disrupted (e.g. enemy Flow Break or Diem Wing), wasting your effort.
- Fragile in melee — you want range, not a brawl.
- The Laguna Blade locks out other complex magic and can be shattered by disarm, Flow Break, or simply running your Mystic dry.

## Tips

- **Keep Mystic topped off.** Get in the habit of `concentrate` between fights. Don't open a fight near empty.
- **Match cost to your pool.** Your max Mystic is your Will. Plan your opener so you can afford your biggest chants when they matter.
- **Don't over-research off-spec schools** past rank 44 unless you intend to specialize into them — those points sit dormant.
- **Shamanism wants spread, specialists want depth.** As a shaman, raise all four elements; as Black/White, drive one school to 50.
- **Use Flow Break (White) against other casters** to shatter barriers, kill an enemy Laguna Blade, and interrupt their chant.
- **Laguna Blade is a finisher, not an opener** — it drains Mystic every moment and blocks your other complex chants, and shatters when your Mystic runs out. Bring it out when you can close.
- **Type multi-word chants with a hyphen** to avoid the "spell names are one word" prompt.

## Gear

A core truth in Static Chaos: **gear never grants class power.** Equipment only provides HP, mana, movement, AC (armor), hitroll, damroll, and saving throws. Your *real* power — your school ranks, your Mystic pool, your chant damage — comes entirely from training with primal. No item makes your chants hit harder.

So gear your Sorcerer for survival and fuel, not for melee stats:

- **HP** — you're squishy; raw survivability buys you time to chant.
- **AC** — fewer melee hits landing on you means fewer interrupted chants.
- **Save vs. spell** — soaks enemy magic, vital against other casters.
- **Mana** — this is your secondary fuel: `concentrate` converts mana into Mystic, so more mana means faster, longer recovery.

**Skip hitroll and damroll.** They improve weapon swings, which do almost nothing for a chanter — your damage is in your chants, and your chants don't care about your damroll. Spend those gear slots on HP, AC, saves, and mana instead. See the **Gearing Guide** for the strongest obtainable pieces.
