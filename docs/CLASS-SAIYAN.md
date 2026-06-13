# Saiyan — Player Guide
> Power up, then unleash. The harder you train, the harder you hit.

## Overview
Saiyans are the proud warrior race of Static Chaos — melee bruisers who turn raw rage into devastating energy attacks. You fight in close, but your signature moves are blasts of **Ki** that scale off how much **fighting power** you've built up. A fresh Saiyan starts with the highest **Body** in the game (100) and very strong **Spirit** (90), which together drive almost everything you do: Body fuels your power-ups and your physical strikes, while Spirit fuels the damage of your energy techniques.

You are **not** a mana user. Your spells-equivalent is a personal **power pool** that you charge yourself and spend on techniques. No item refills it — only you do, by raging.

- **Prime stat:** Wisdom
- **Starting stats:** Body 100, Mind 75, Spirit 90, Will 80
- **Guild room:** vnum 3003
- **Resource:** Fighting power (Ki), self-generated — not mana

## How your power works
Everything keys off one number: your **fighting power** (your Ki pool).

- **You build power with `rage`.** Each rage converts stamina (move) into a chunk of fighting power, and simultaneously boosts your inner **strength**, **speed**, and **aegis** (defensive Ki). The bigger your Body and Spirit, the bigger each gain. Rage costs move points, so you can't power up forever without resting — if you're too exhausted, it fails.
- **You spend power on techniques.** Every energy attack and utility move drains a fixed amount of fighting power. Small moves like Ki Blast cost a few hundred; the big guns (Kamehameha, Kaiouken) cost tens of thousands. So a fight is a rhythm: rage to bank power, then dump it into the right attack.
- **`focus` lets you redirect banked power** into your strength, speed, or aegis on demand, trading offensive reserves for a tougher, faster fighter.
- **Techniques must be learned first.** You don't get them automatically — you buy each one permanently with **primal** (your advancement currency) at your guild using the `technique` command. Once learned, it's yours forever; you just need enough fighting power on hand to fire it.

Your stats grow as you advance, and higher Body/Spirit directly increase both how fast you bank power and how hard your techniques land.

## Core commands

| Command | What it does |
|---|---|
| `rage` | Power up: spends stamina to raise your fighting power and boost strength, speed, and aegis. Your core resource generator. |
| `focus <ability> <amount>` | Channel banked fighting power into `strength`, `speed`, or `aegis`. |
| `technique` | View, price (`technique cost`), check (`technique learned`), and learn techniques with primal (e.g. `technique kibolt`). |
| `kiblast [target]` | Cheap energy bolt (needs 500 power). Good spam attack. |
| `kiwave [target]` | Wave of Ki; also disrupts certain enemy classes (breaks Fist rhythm, Sorcerer casting, Mazoku charge, other Saiyans' Ki walls). |
| `kibolt [target]` | Concentrated high-damage bolt (needs 2000 power). |
| `kibomb` | Room-wide explosion hitting everyone not in your group (needs 2000 power). |
| `kiwall` | Raises a defensive wall of Ki that improves your AC for a short time (needs 2500 power). |
| `kikouhou <target>` | Long-range bolt that can hit anywhere in the area; sets a long fight timer (needs 5000 power). |
| `masenkouha [target]` | Charged beam — say it, then it fires next tick for heavy Spirit-based damage (needs 5000 power). |
| `kamehameha [target]` | Your ultimate beam. Charges over multiple ticks ("Kame.. Hame.. Ha!!") then detonates (needs 15000 power). |
| `kaiouken` | Self-buff: bursts into crimson flame, sharpening your combat for a short time (needs 20000 power). |
| `ryuken` | Dragon uppercut combo — a huge two-part melee + Ki strike while fighting (needs 10000 power). |
| `solarflare <target>` | Flash of light that blinds your target. Strong opener/control. |
| `flight` | Lift off and fly (needs 250 power). |
| `shunkanidou <target>` | Instant teleport to a target's location anywhere in the world (needs 10000 power; can't use from an HQ or with a fight timer). |
| `kisense <target>` | Sense a target's Ki and locate their general area, anywhere in the world. |
| `battlesense <target>` | Read a target in your room: HP, mana, moves, class, and (for Saiyans) their current fighting power. |
| `hawkeyes` | Toggle enhanced vision (true sight) on/off (needs 500 power to turn on). |

## Getting started
1. **Find your guild** (room 3003) and check what's available with `technique` — then `technique cost` to see prices in primal.
2. **Learn cheap, high-value moves first.** Great early picks: **Ki Blast** (25 primal, your bread-and-butter attack), **Flight** (50), **Hawk Eyes** (50, cheap true sight), **Battlesense** (75), and **Solar Flare** (100, a blind that wins fights). Save up for **Ki Bolt** and **Ki Wave** as your damage backbone.
3. **Learn the rhythm:** `rage` a few times to bank power (watch your move/stamina), then attack and spend that power on techniques. Re-rage between fights.
4. **Use `focus`** when you need to be tankier or faster right now — pour power into `aegis` for survivability or `strength`/`speed` for the brawl.
5. **Work toward the big three:** Ki Bolt/Ki Wave for sustained damage, then Masenkouha and eventually Kamehameha as your finishers.

## Strengths & weaknesses
**Strengths**
- Top-tier Body and strong Spirit — hits hard with both fists and Ki.
- Self-sufficient resource: you generate your own power with `rage`, no mana to run dry.
- Excellent toolkit: ranged blasts, an area bomb, a defensive wall, a blind, true sight, flight, world-wide tracking (Ki Sense) and teleport (Shunkan Idou).
- Strong class-disruption: Ki Wave staggers and interrupts several enemy classes.

**Weaknesses**
- **Ramp-up dependent.** Your best attacks need huge banked power (Kamehameha needs 15,000; Kaiouken 20,000). Caught with an empty pool, you're far weaker.
- **`rage` burns stamina**, so you can be powered down by exhaustion if you don't manage move points.
- **Charge moves leave you exposed.** Masenkouha and Kamehameha take ticks to go off ("Kame.. Hame.. Ha!!") — interruptible windows where you're committed.
- Big techniques carry long lag (wait states), so mis-timing them is costly.

## Tips
- **Bank before the fight.** Walk into a duel already raged up so you can open with a real technique instead of scrambling for power.
- **Open with Solar Flare.** A blinded opponent is a much easier kill — it's cheap and brutal.
- **Use Battlesense on other Saiyans** to read their exact fighting power, then strike when they're drained.
- **Ki Wave is more than damage** — time it to break a Fist's rhythm, interrupt a Sorcerer's chant, kill a Mazoku's charge, or collapse another Saiyan's Ki wall.
- **Don't start a Kamehameha in the open** against someone who can punish the charge; save it for when you've controlled the fight (e.g. after a Solar Flare blind).
- **Keep an eye on stamina** — if `rage` says you're too exhausted, rest before you bank more power.
- **Shunkan Idou + Ki Sense** make you a great hunter: locate, then teleport in.

## Gear
Remember the golden rule of Static Chaos: **gear does not give you your class power.** Armor and weapons only supply HP, mana, move, AC, hitroll, damroll, and saves — your Saiyan strength comes entirely from **training** (raging, focusing, and the techniques you buy with primal). Gear keeps you alive and swinging; *you* bring the power. For how to pick and optimize equipment stats, see the **Gearing Guide**.
