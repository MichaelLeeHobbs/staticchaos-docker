# Patryn — Player Guide
> Rune mages who tattoo the elements into their skin and weave raw magic on the fly.

## Overview

The Patryn is Static Chaos's rune mage. Where other classes hit harder or move faster, the Patryn *combines* — pairing a primary elemental rune with a secondary "intent" rune to weave dozens of different effects on the spot: lightning bolts, healing, room-clearing fire, mana drains, curses, wards, and protective shields. Almost nothing comes pre-loaded; your power is whatever rune knowledge you have **trained** and **learned**.

Patryns use **DEX** as their prime stat, and their signature attribute is **mind** — it scales the strength and duration of nearly everything they weave. They have strong all-around magic and excellent defensive tools, but they are not a class you can play on autopilot. You build the Patryn you want by choosing which runes to master.

## How your power works

Three things drive a Patryn, and all three are earned, not bought from gear:

1. **Learned runes** — You must `learn` each rune before you can use it. There are six **primary** runes (air, earth, fire, water, energy, negative) and eight **secondary** runes (life, death, creation, destruction, protection, transformation, movement, abjuration). Learning costs *primal*, and each new rune costs a little more than the last.
2. **Rune understanding (training)** — Use `runetrain` to deepen your mastery of the six primary elements. Higher understanding means stronger weaves of that element. Training costs **experience**, gets steeply more expensive as you climb, and tops out at full mastery.
3. **Mind** — Your `mind` score multiplies weave damage, healing, ward duration, and curse length. A high-mind Patryn weaves longer-lasting, harder-hitting magic.

**Weaving** is the heart of the class. You `runeweave` a primary rune + a secondary rune together; the combination determines the spell. You must have *both* runes learned, and each weave costs **mana**. For example, air + death throws a lightning bolt, fire + destruction engulfs the whole room in flame, energy + life heals, and any element + abjuration raises an elemental ward around you.

**Tattoos** are a separate, permanent layer. You can `tattoo` the four physical runes (earth, air, fire, water) onto five body parts (torso, arms, legs). Tattooing costs primal *and* a large amount of mana, and each body part holds a limited number of runes. Use `runestats` to see what's inked on you, and `erase` to remove a tattoo (also costs primal and mana).

## Core commands

| Command | What it does |
|---|---|
| `learn <rune>` | Permanently learn a rune so you can weave with it (costs primal). Type `learn` alone to list all runes; `learn cost` shows the price of your next one. |
| `runetrain <element>` | Deepen your understanding of a primary element — air, earth, fire, water, energy, negative — making its weaves stronger (costs experience). Type alone to see costs. |
| `runeweave <primary> <secondary> [target]` | Weave two learned runes into a spell (costs mana). Offensive weaves hit your current fight target if you don't name one. |
| `tattoo <bodypart> <rune>` | Permanently ink earth/air/fire/water onto torso, leftarm, rightarm, leftleg, or rightleg (costs primal + mana). |
| `erase <bodypart> <rune>` | Remove a tattooed rune (costs primal + mana). |
| `runestats` | Show all the runes currently tattooed on your body. |
| `defenses raise` / `defenses lower` | Raise or lower your standing magical defenses. Type `defenses` alone to check their state. |
| `circle` | Close "the circle of your being" while resting — a focusing ritual. |

## Getting started

> Your guild is **[The Thieves Bar — room 3028](#/map?room=3028)** in Midgaard. (Open it on the World Map.)

1. **Learn your first runes.** You can begin learning and tattooing at level 2. Start with `learn` to see the list, then learn a primary element or two and a secondary intent. A solid opening pair is an offensive element (air or fire) plus **death** for direct damage, or **life**/**creation** if you want healing first.
2. **Pick a primary to train.** Run `runetrain` to see costs, then pour experience into one element you'll lean on. Concentrated training makes that element's weaves noticeably stronger.
3. **Weave it.** With both runes learned, try `runeweave air death` on a target. Watch your mana — weaves are not cheap.
4. **Set up wards and defenses.** Learn **abjuration** and pair it with an element to raise an elemental ward. Use `defenses raise` so your standing protection is up before you fight.
5. **Tattoo for the long game.** Once you have spare primal and mana, `tattoo` your body with elemental runes and check the result with `runestats`.

## Strengths & weaknesses

**Strengths**
- Enormous **flexibility** — one class covers nukes, area damage, heals, cures, buffs, curses, drains, and multiple elemental wards.
- **Top-tier mind**, so weaves scale hard as you train and level.
- Strong, layered **defenses**: stackable shields (shield, stone skin, armor, balus wall), a switchable defense stance, and elemental wards.
- Utility weaves can heal allies, restore movement, grant true sight and flight, and dispel enemy magic.

**Weaknesses**
- **Setup-heavy.** An untrained, unlearned Patryn is weak. You must invest primal *and* experience before you feel powerful.
- **Mana-hungry.** Big weaves cost hundreds of mana each, and tattooing costs even more.
- Only the four physical elements (earth/air/fire/water) can be tattooed — energy and negative cannot.
- Many of the flashiest weaves only do something against players or specific classes; against ordinary monsters some effects fizzle.

## Tips

- **Specialize, then branch.** Fully training one or two primary elements early gives you more punch per fight than spreading thin across all six.
- **Mind is king.** Anything that raises your mind raises your whole kit — ward duration, curse length, weave damage and healing all scale with it.
- **Watch your weave targeting.** Offensive weaves auto-target whatever you're fighting; self/defensive weaves default to you. Name a target explicitly when you want to weave on an ally.
- **Wards don't stack with each other.** Raising one elemental ward (via *element* + abjuration) strips your other wards — keep up the one that matters for the fight you're in.
- **Air + transformation and fire + transformation cancel each other** — you can flip between an air-block and a fire-block, but you can't hold both.
- **Energy + creation and energy + life are your panic buttons** — the more your energy understanding is trained, the bigger the heal package they deliver.
- **Keep `defenses` raised** out of combat so you're never caught flat-footed.

## Gear

Gear matters for a Patryn the same way it matters for everyone — and no more. Equipment only ever provides **HP, mana, movement, AC, hitroll, damroll, and saves**. Mana gear is especially valuable to you because weaving and tattooing burn through it fast, and a healthy mana pool lets you weave more before you run dry.

What gear **cannot** do is give you rune knowledge. No item teaches a rune, trains an element, or raises your mind-driven weave power. Those come only from `learn`, `runetrain`, and leveling. A fully geared Patryn who hasn't trained their runes is still a weak Patryn — so treat gear as fuel for the magic you've already earned, not a substitute for it. See the **Gearing Guide** for the best stat pieces.
