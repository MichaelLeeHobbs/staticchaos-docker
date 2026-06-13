# Mazoku — Player Guide
> A shapeless astral demon that sculpts its own body from raw essence — claws one moment, a charged energy blast the next.

## Overview

The Mazoku is a demon: a being of astral energy with no fixed shape. Instead of equipping a kit and swinging a weapon, you **build the body you want** out of your own substance — growing limbs, forming weapons from your arms, and shifting between a human disguise, a monstrous battle form, and a near-invisible true astral form.

You are well-rounded across the board (Body, Mind, Spirit, and Will are all strong), and your prime attribute is Intelligence. You are **not** a mana-based caster — your power comes from a resource called **essence**, spent moment-to-moment in combat, plus permanent upgrades bought with experience and a separate resource called **primal**.

The trade-off: as long as you are in any non-human form, you are made of energy, not flesh — and ordinary armor and weapons simply fall off you. A Mazoku that wants gear has to either look human or make its own **astral** equipment.

## How your power works

Three different "currencies" drive a Mazoku:

- **Essence** — your live combat fuel. Every form you take, every limb you grow, every charged attack, every reform and teleport spends essence. Watch it constantly; if you run dry mid-fight you can't shift forms or fire your big attacks.
- **Primal** — spent once, with `develop`, to permanently *learn* a technique (claws, blades, the charge ability, the bolt/blast/bomb attack modes, extra arms, wings, eyes, teleport, astral strike). Learning something with `develop` doesn't make it active — it unlocks your ability to use it.
- **Experience** — spent with `develop` to raise four core masteries:
  - **Astral** — the heart of your damage. Higher astral = bigger charged attacks and faster charge time.
  - **Matter** — your command over physical substance.
  - **Focus** — improves things like how precisely you can teleport.
  - **Nihilism** — your peace with oblivion.

The pattern is always: **`develop` to learn or improve, then the matching command to use it.** Class power is trained, never bought from a shop.

## Forms

You shift forms with the `morph` command. Each form has a purpose and an essence cost:

- **Human** (`morph human`) — you reform "looking vaguely human," gain infravision, and grow ordinary hands. This is the **only** form in which you can wear normal material gear. Use it for traveling, equipping, and blending in. While human you can't grow battle limbs.
- **Battle** (`morph battle`) — a monstrous combat body. This is your standard fighting form: it lets you form weapon-limbs and grow extra body parts, and it grants infravision.
- **True** (`morph true`) — your true astral form. You become diffuse and nearly fade from sight. Charged attacks hit substantially harder in true form, and it is the **only** form from which you can `teleport`. The catch: you're barely material, so all your normal gear drops off the moment you enter it.

Switching forms takes a moment (you're locked briefly while you reform), and entering any non-human form drops everything you're wearing that isn't an astral item.

## Core commands

| Command | What it does |
|---|---|
| `develop` | Learn a technique with primal, or raise Astral/Matter/Focus/Nihilism with experience. `develop list` shows costs. |
| `form` | Shape your arms into hands, claws, spikes, blades, or tentacles (costs essence; the weapon must be developed first). |
| `morph` | Change between Human, Battle, and True form (costs essence). |
| `grow` | Permanently add a body part — third/fourth/fifth/sixth arm, an eye, or wings (costs a large amount of essence; must be developed first). |
| `charge` | Begin building up a charged astral attack in a chosen color: obsidian, emerald, cerulean, or crimson. Must be in combat; costs essence to charge. |
| `release` | Unleash the charged energy as a `bolt`, `blast`, or `bomb` at your target. The longer it charged, the harder it hits. |
| `rake` | Slash with your claws — extra arms add extra hits (requires claws formed). |
| `gouge` | Drive your spikes into the enemy's eyes, potentially blinding them (requires spikes formed). |
| `lash` | Whip your tentacles at the target and disrupt their footing (requires tentacles formed). |
| `astrike` | Begin an astral strike, phasing in and out of the material plane against your target (must be developed; costs essence). |
| `teleport` | Will yourself across the astral plane to a target — true form only; costs essence plus health, mana, and movement. |
| `reform` | Repair the tears in your astral body, healing yourself for essence (more healing costs more essence). |
| `instantiate` | Forge an astral item — ring, amulet, shirt, cap, pants, boots, gloves, sleeves, cloak, belt, bracer, shield, or sceptre (costs a large amount of essence). |
| `imbue` | Pour essence into an astral item to raise its hitroll and damroll, up to a cap. |

## Getting started

1. **Learn the basics.** Use `develop list` to see what's available and what it costs. Spend your first primal on `develop claws` so you have a real attack, and `develop charge` plus one attack mode (`develop bolt`, `develop blast`, or `develop bomb`).
2. **Pour experience into Astral.** Your charged attacks scale off Astral mastery, and it also speeds up your charge time. `develop astral` early and often.
3. **Practice forming up.** Use `form claws` to grow weapon-limbs, then `morph battle` to enter your fighting body. Try `rake` in a fight to feel out claw attacks.
4. **Learn the charge-release rhythm.** In combat, `charge crimson` (or another color), wait for it to build, then `release bolt`. Don't release too early — a partly-charged attack hits for much less.
5. **Keep an eye on essence.** Reforming, morphing, and big attacks all drain it. Plan fights so you don't run empty.
6. **Make your own gear (later).** Once you can spare the essence, `instantiate` astral items so you can stay battle-ready outside human form, then `imbue` them to sharpen them.

## Strengths & weaknesses

**Strengths**
- Extremely versatile — you reshape yourself for the situation instead of relying on loot.
- Strong, balanced core stats with no glaring weakness.
- Heavy burst potential from charged astral attacks, especially in true form.
- Built-in self-healing (`reform`) and mobility (`teleport`) that don't depend on outside gear.
- The bomb release hits everyone in the room who isn't in your group — strong against multiple enemies.

**Weaknesses**
- **Can't wear normal gear except in human form** — and human form is your weakest combat shape. (See Gear.)
- Charged attacks need set-up time; you're committing and somewhat exposed while building energy.
- Essence-hungry — running dry leaves you unable to shift, heal, or fire your big hits.
- Big upgrades (extra limbs, astral items) demand enormous essence reserves, so they come online slowly.
- Form-shifting and big abilities lock you up briefly, so timing matters.

## Tips

- **Charge before you need it, release at full.** Damage grows the longer energy has gathered — a rushed release wastes your essence.
- **Fight in true form when you can afford it.** Charged attacks hit far harder there. Drop to battle form when you need your weapon-limbs, and human form only to gear up or travel.
- **Match your limbs to your tactic:** claws (`rake`) for raw multi-hit damage that scales with extra arms, spikes (`gouge`) to blind a dangerous target, tentacles (`lash`) to disrupt enemy footing.
- **Grow extra arms for more rake hits.** Each additional arm adds another swing — a major long-term power spike once you can afford the essence.
- **Keep `reform` in reserve.** It's your panic-heal; healing more costs more essence, so top up before you're critical.
- **Prioritize Astral mastery.** It's your single biggest damage and tempo lever.
- **Stay topped up on essence** before committing to a fight, a teleport, or a form change.

## Gear

**Read this carefully — it's the rule that catches every new Mazoku off guard.**

A Mazoku is a creature of astral energy. **You cannot wear ordinary material gear unless you are in human form.** The instant you enter battle form or true form, anything you're wearing that isn't astral simply slips off and drops to the ground — your body is no longer solid enough to hold it.

That leaves you two ways to be equipped:

1. **Stay human.** In human form you can wear normal armor and items like anyone else — but human form is your weakest fighting shape, so this is mostly for travel, town, and gearing up.
2. **Use astral items.** Forge your own equipment with `instantiate` (ring, amulet, shirt, cap, pants, boots, gloves, sleeves, cloak, belt, bracer, shield, sceptre). Astral items stay on you in *any* form, and you can sharpen them over time with `imbue`, which raises their hitroll and damroll up to a cap. This is the real endgame answer for a combat Mazoku.

A shared truth across all classes worth remembering: **gear only provides stats** — HP, mana, movement, armor class, hitroll, damroll, and saves. It does **not** grant your class abilities. Your forming, growing, charging, and astral mastery all come from training (`develop` and experience), never from an item. So while astral gear keeps your numbers competitive, your actual power as a Mazoku is something you build into your own body.
