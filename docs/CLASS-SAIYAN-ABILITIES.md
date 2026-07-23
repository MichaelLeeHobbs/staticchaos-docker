# Saiyan — Ability Reference (Balance / Mechanics)

> **Audience:** balance review and maintainers. This is the exhaustive, numbers-first reference
> for **every** Saiyan ability, extracted directly from the server source. For the new-player
> narrative version, see **[Saiyan — Player Guide](CLASS-SAIYAN.md)** (do not balance-check against
> that doc; this one is authoritative).

**Last verified against source:** commit `f0e24e1` (Ki Wave chant-interrupt section, #122; rest verified at `d4de47d`)
**Primary sources:** `src/classes/saiyan.c`, `src/core/fight.c`, `src/core/update.c`, `src/core/const.c`, `src/include/merc.h`, `src/core/interp.c`

## How to read this

- **Each ability is cited `file:function`** so every number can be checked against the C. If a number
  here disagrees with the code, the **code wins** — fix this doc.
- **Cost / threshold:** Saiyans spend a self-generated **fighting power** pool (`S_POWER`), *not* mana.
  Most checks use `<=` (so you need **strictly more** than the listed number); a few use `<` (you need
  **at least** the number). The exact comparison is given per ability because it matters at the margin.
- **Formulas are quoted verbatim** from the source. `dice(n, size)` rolls `n` dice of `1..size`
  (mean ≈ `n*(size+1)/2`). `body` = `ch->pcdata->body`, `spirit` = `ch->pcdata->spirit`.
- **WAIT_STATE(ch, n)** is lag in pulses (`PULSE_VIOLENCE`), the standard combat-round unit.
- **Damage types** route through `damage()` in `src/core/fight.c`; `DAM_KIFLAME` and the melee `F_UPPERCUT`
  type have Saiyan-specific resist/soak handling, documented under *Passive defenses* below.
- **⚠️ Needs verification** items are collected at the bottom — don't trust those numbers blindly.

## Class basics

Source: `src/core/const.c` `class_table[CLASS_SAIYAN]`, `src/include/merc.h` (struct `class_type`).

| Field | Value | Notes |
|---|---|---|
| `who_name` | `Saiyan` | |
| `attr_prime` | `APPLY_WIS` | Prime attribute = **Wisdom** |
| First weapon | `OBJ_VNUM_SCHOOL_MACE` | Newbie weapon |
| Guild room | `3003` | Learn techniques here |
| Max trainable Body | `100` | Highest Body in the game; fuels power-ups + melee |
| Max trainable Mind | `75` | |
| Max trainable Spirit | `90` | Fuels **all** Ki technique damage |
| Max trainable Will | `80` | |
| HP gained per level | `10` | |
| Gains mana | `FALSE` | Not a mana class — resource is fighting power |

**Power slots** (`src/include/merc.h`, indices into `ch->pcdata->powers[]`): `S_POWER`/`S_POWER_MAX` (fighting
power pool), `S_STRENGTH`/`_MAX`, `S_SPEED`/`_MAX`, `S_AEGIS`/`_MAX` (defensive Ki), and `S_TECH`
(bitfield of learned techniques). Techniques are bits in `S_TECH` (`S_KIBOLT=1`, `S_KIWAVE=2`,
`S_KIBLAST=4`, … `S_KIAIHOU=262144`).

---

## 1. Power management

### Rage — `rage`
*Source: `src/classes/saiyan.c:do_rage` · `interp.c`: POS_FIGHTING, level 2*

Core resource generator. Converts stamina (`move`) into fighting power and simultaneously buffs
strength/speed/aegis. No learning required (not a technique).

- **Gain (power):** `gain = dice(body, spirit/2) + S_POWER_MAX/100`
- **Gain (str/speed/aegis):** `gain2 = isquare(gain)` (integer square root of the power gain), applied
  to `S_SPEED`, `S_STRENGTH`, and `S_AEGIS` (each capped at its `_MAX`).
- **Power added:** `S_POWER += gain` (capped at `S_POWER_MAX`).
- **Stamina cost:** `move -= UMIN(gain/10, 1000)`.
- **Fails if** already at `S_POWER_MAX` ("already at full power"), or if `move <= UMIN(gain/10, 1000)`
  ("too exhausted").
- **Lag:** `WAIT_STATE(ch, 4)`.

### Focus — `focus <strength|speed|aegis> <amount>`
*Source: `src/classes/saiyan.c:do_focus` · POS_FIGHTING, level 2*

Redirects banked fighting power into a single attribute on demand (trades offense reserves for a
tougher/faster fighter). Not a technique.

- **Effect:** `S_<attr> += amount` (capped at `S_<attr>_MAX`), then `S_POWER -= amount`.
- **Targets:** `strength`, `speed`, or `aegis` (matched by `str_prefix`).
- **Validation:** `amount` must be `> 0` and `< S_POWER` (must have more than you spend).
- **Lag:** `WAIT_STATE(ch, 4)`.

### Technique (learn / list) — `technique [<name>|cost|learned]`
*Source: `src/classes/saiyan.c:do_technique` · POS_STANDING, level 2*

The learning command. Techniques are bought **permanently** with **primal** (advancement currency)
and stored as bits in `S_TECH`. `technique` with no arg lists them; `technique cost` prints prices;
`technique learned` prints an owned/not grid.

**Technique costs in primal** (verbatim from `do_technique`):

| Technique | Command keyword | Primal cost | `S_TECH` bit |
|---|---|---|---|
| Ki Blast | `kiblast` | 25 | `S_KIBLAST` |
| Ki Wave | `kiwave` | 50 | `S_KIWAVE` |
| Ki Bomb | `kibomb` | 50 | `S_KIBOMB` |
| Ki Bolt | `kibolt` | 50 | `S_KIBOLT` |
| Flight | `flight` | 50 | `S_FLIGHT` |
| Hawk Eyes | `hawkeyes` | 50 | `S_HAWKEYES` |
| Ki Wall | `kiwall` | 75 | `S_KIWALL` |
| Battlesense | `battlesense` | 75 | `S_BATTLESENSE` |
| Kikouhou | `kikouhou` | 100 | `S_KIKOUHOU` |
| Solar Flare | `solarflare` | 100 | `S_SOLARFIST` |
| Zanzouken | `zanzouken` | 100 | `S_ZANZOUKEN` |
| Ki Sense | `kisense` | 100 | `S_KISENSE` |
| Masenkou Ha | `masenkouha` | 125 | `S_MASENKOUHA` |
| Shunkan Idou | `shunkanidou` | 125 | `S_KIMOVE` |
| Ryuken | `ryuken` | 150 | `S_RYUKEN` |
| Kamehameha | `kamehameha` | 200 | `S_KAMEHAMEHA` |
| Kaiouken | `kaiouken` | 200 | `S_KAIOUKEN` |
| Kiaihou | `kiaihou` | 200 | `S_KIAIHOU` |

- **Buy logic:** fails if `primal < cost` or technique already owned; otherwise `primal -= cost` and
  `SET_BIT(S_TECH, skill)`.
- **⚠️ Note:** the keyword to *learn* Masenkou Ha is `masenkouha` (the menu/list text shows
  `Masenkou`), and Kaiouken is keyed off `S_KAIOUKEN`. **Zanzouken and Kiaihou are learnable here but
  have no active command** — they are passive (see *Passive techniques* below).

---

## 2. Offensive techniques (Ki attacks)

All energy attacks deal `DAM_KIFLAME` unless noted. Damage scales off **Spirit**. Per-target power
thresholds use the exact comparison shown.

### Ki Blast — `kiblast [target]`
*Source: `src/classes/saiyan.c:do_kiblast` · POS_FIGHTING*

- **Damage:** `dice(7, spirit*5)` — `DAM_KIFLAME`.
- **Requires:** `S_POWER > 500` (check is `<= 500`). **Cost:** `S_POWER -= 500`.
- **Targeting:** uses `ch->fighting` if no arg; else `get_char_room`.
- **Lag:** `WAIT_STATE(ch, 5)` (4 on a bad target).

### Ki Bolt — `kibolt [target]`
*Source: `src/classes/saiyan.c:do_kibolt` · POS_FIGHTING*

- **Damage:** `dice(spirit/7, spirit*5)` — `DAM_KIFLAME`.
- **Requires:** `S_POWER > 2000` (check `<= 2000`). **Cost:** `S_POWER -= 2000` (deducted *before* the hit).
- **Lag:** `WAIT_STATE(ch, 8)`.

### Ki Wave — `kiwave [target]`
*Source: `src/classes/saiyan.c:do_kiwave` · POS_FIGHTING*

Damage **plus** a major cross-class disruption suite.

- **Damage:** `dice(5, spirit*7)` — `DAM_KIFLAME`.
- **Requires:** `S_POWER > 2000` (check `<= 2000`). **Cost:** `S_POWER -= 2000`.
- **Lag:** `WAIT_STATE(ch, 8)`.
- **Cross-class effects (PvP only):**
  - **vs Fist:** if `number_percent() > victim body/2`, drains `F_KI -= dice(2,3)+1` and clears
    `NEW_FIGUREEIGHT` (breaks the Figure-Eight rhythm).
  - **vs Saiyan:** strips the target's `gsn_kiwall` if active ("collapses your wall of Ki").
  - **vs Sorcerer:** chance to interrupt the active chant. Base interrupt chance **66%**; reduced
    `-15` if `AFF_VAS_GLUUDO`, `-10` if `AFF_HOLY_RESIST`, and `-10` if `gsn_defense` is up (Defense
    is consumed — stripped — in the process); clamped to `URANGE(50, chance, 85)`, so avoidance is
    capped at **50%** (#122). On interrupt, `lose_chant(victim)`.
  - **vs Mazoku:** if charging (`M_CTYPE != 0`), resets `M_CTYPE = 0` and `M_CTIME = 0` (loses the charge).

### Ki Bomb — `kibomb`
*Source: `src/classes/saiyan.c:do_kibomb` · POS_FIGHTING*

Room-wide AoE; no target argument.

- **Damage (each victim):** `dice(10, spirit*4)` — `DAM_KIFLAME`, to every char in the room that is
  **not** `ch` and **not** in `ch`'s group (`is_same_group`).
- **Requires:** `S_POWER > 2000` (check `<= 2000`). **Cost:** `S_POWER -= 2000`.
- **Lag:** `WAIT_STATE(ch, 8)`.

### Kikouhou — `kikouhou <target>`
*Source: `src/classes/saiyan.c:do_kikouhou` · POS_FIGHTING*

Long-range (area-wide) bolt that also pins a long fight timer.

- **Damage:** `dice(spirit/10, spirit*10)` — `DAM_KIFLAME`.
- **Requires:** `S_POWER > 5000` (check `<= 5000`). **Cost:** `S_POWER -= 5000`.
- **Range:** `get_char_area` (anywhere in the current area).
- **Restrictions:** can't target NPCs `level > 95` ("unable to target them"); can't interfere in a
  PvP fight where the victim is fighting another player.
- **Side effects:** sets `extras[TIMER] = 16` if currently lower (locks you into a long combat timer);
  if the victim was fighting `ch` (and not a fireback player), `stop_fighting(victim)`.
- **Lag:** `WAIT_STATE(ch, 20)`.

### Masenkou Ha — `masenkouha [target]`
*Source: `src/classes/saiyan.c:do_masenkouha` (charge) → `src/core/update.c` (resolve) · POS_FIGHTING*

One-tick charged beam. The command commits & spends power; damage lands the **next** game tick.

- **Charge:** sets `NEW_MASENKOUHA`, stores target, `do_say "Masenkou.."`, `S_POWER -= 5000`,
  `WAIT_STATE(ch, 9)`.
- **Requires:** `S_POWER > 5000` (check `<= 5000`).
- **Resolve (next tick, `update.c`):** `do_yell "Ha!!"`, damage `dice(spirit/10, spirit*10)` —
  `DAM_KIFLAME`. Resolves against the target in-room, else in-area; if the target is gone, the bolt
  fires harmlessly into the air.
- **Interruptible:** the charge state can be cleared before resolution (you are committed during the lag window).

### Kamehameha — `kamehameha [target]`
*Source: `src/classes/saiyan.c:do_kamehameha` (charge) → `src/core/update.c` (resolve) · POS_FIGHTING*

The ultimate. **Two-tick** charge: "Kame.. Hame.. Ha!!"

- **Charge:** sets `NEW_KAME_1`, stores target, `do_say "Kame.."`, `S_POWER -= 15000`,
  `WAIT_STATE(ch, 16)`.
- **Requires:** `S_POWER >= 15000` (check `< 15000`).
- **Tick 1 (`update.c`):** `NEW_KAME_1` → `do_say "Hame.."`, advances to `NEW_KAME_2`.
- **Tick 2 (`update.c`):** `NEW_KAME_2` → `do_yell "Ha!!"`, damage `spirit*105 + dice(15, spirit)` —
  `DAM_KIFLAME`. Same in-room → in-area → whiff-into-air targeting as Masenkou Ha.
- **⚠️ Note:** the damage formula in `saiyan.c` is **commented out / superseded**; the live number is
  the `update.c` value above (`spirit*105 + dice(15, spirit)`), *not* the old
  `spirit*115 + dice(10, spirit)` in the comment.

### Ryuken (Dragon uppercut) — `ryuken`
*Source: `src/classes/saiyan.c:do_ryuken` · POS_FIGHTING*

A heavy two-part melee + Ki strike **only while fighting**, and the Saiyan's premier defense-stripper.

- **Part 1 damage:** `dice(body, body*3/2)` — typed `F_UPPERCUT` (melee uppercut type; see Passive
  defenses for ward interactions on `F_UPPERCUT`).
- **Part 2 damage** (if still fighting after part 1): `dice(spirit, spirit*5/4)` — `DAM_KIFLAME`.
- **Setup bonus (Issue #4):** if the victim is in a "major setup/defense state" *before* stripping,
  **+10%** to **both** damage rolls (`dam += dam*10/100`). Setup states detected: any of
  `gsn_defense`, `gsn_kiwall`, `gsn_spirit_ward`, `gsn_earth_ward`, `gsn_flame_ward`, `gsn_wind_ward`,
  `gsn_water_ward`, `gsn_negative_ward`; or Fist `NEW_FIGUREEIGHT`; or Mazoku charging (`M_CTYPE != 0`);
  or Patryn Air/Fire/Negative block; or Sorcerer mid-chant (`chant != NULL`).
- **Requires:** `S_POWER >= 10000` (check `< 10000`). **Cost:** `S_POWER -= 10000`.
- **Self-gate:** fails if `ch->hit < ch->max_hit/4` ("too weakened"). Anti-cheese: refuses if the
  victim is a player below 1/4 HP while `ch` is above 1/2 HP ("no more lame run").
- **Cross-class strips (the headline interaction):**
  - **vs Fist:** clears `NEW_FIGUREEIGHT` and zeroes `F_KI`.
  - **vs Saiyan:** strips `gsn_kiwall`.
  - **vs Sorcerer:** shatters `gsn_defense` (still takes damage); if no Defense, interrupts the chant
    (`lose_chant`).
  - **vs Patryn:** strips one active block in priority **Air > Fire > Negative**
    (`patryn_active_block`); if none, drains an active ward by **500** duration
    (`dec_duration(victim, ward, 500)` via `patryn_active_ward`).
  - **vs Mazoku:** scatters a charge (`M_CTYPE = 0`, `M_CTIME = 0`).
  - Any successful strip sets `broke_major`.
- **Lag:** `WAIT_STATE(ch, broke_major ? 26 : 30)` — breaking a major state *reduces* your lag.

### Solar Flare — `solarflare <target>`
*Source: `src/classes/saiyan.c:do_solarflare` · POS_FIGHTING*

A blind. Not a damage move — it casts the `blind` spell.

- **Effect:** calls `skill_table[skill_lookup("blind")].spell_fun(sn, lev, ch, victim)` with
  `lev = spirit/2`.
- **Requires:** `S_POWER >= 2500` (check `< 2500`). **Cost:** `S_POWER -= 2500`.
- **Restrictions:** caster must be `level >= 2`; can't be used on mortals (`victim level < 2`).
- **Side effects:** if not already fighting, sets mutual fighting; `timer_check(ch, victim)`.
- **Save:** governed by the `blind` spell's own save (not in this function).
- **Lag:** `WAIT_STATE(ch, 8)`.

### Big Bang Attack — `bigbang [target]`
*Source: `src/classes/saiyan.c:do_bigbang` · `interp.c`: POS_DEAD, level `L_SUP` (immortal/admin)*

Effectively an admin-tier nuke (command is gated to `L_SUP`, not normal play).

- **Damage:** `level * body * spirit + dice(100, 100)` — `DAM_KIFLAME`.
- **Requires:** `S_POWER >= 250000` (check `< 250000`). **Cost:** `S_POWER -= 250000`.
- **Side effects:** `do_yell "BIG BANG ATTACK!!"`, applies `WAIT_STATE(victim, 24)` to the target.
- **Lag (self):** `WAIT_STATE(ch, 8)`.

---

## 3. Defensive techniques & buffs

### Ki Wall — `kiwall`
*Source: `src/classes/saiyan.c:do_kiwall` · POS_FIGHTING; resist handled in `src/core/fight.c`*

A short-duration AC buff and damage-soak (`gsn_kiwall`).

- **AC modifier:** `APPLY_AC` of `-1 * S_AEGIS / 50` (more aegis = better AC).
- **Duration:** `dice(2,4)` ticks normally; `dice(2,2)` if `ch->name == "Piccolo"`; **`dice(1,3)` if
  re-cast while already up** (refresh penalty — the old one is stripped first).
- **Requires:** `S_POWER > 2500` (check `<= 2500`). **Cost:** `S_POWER -= 2500`.
- **Damage soak (`fight.c`):** while `gsn_kiwall` is affected, incoming non-`DAM_CERULEAN` damage is
  `dam /= 3` (`F_DEATHTOUCH` only `dam /= 2`); on certain spell paths a separate `dam /= 2` applies.
- **Countered by:** another Saiyan's **Ki Wave** (strips it) and **Ryuken** (strips it).
- **Lag:** `WAIT_STATE(ch, 8)`.

### Kaiouken — `kaiouken`
*Source: `src/classes/saiyan.c:do_kaiouken` (apply) + `src/core/fight.c` (effects) · POS_FIGHTING*

A short, intense self-buff (`gsn_kaiouken`, `duration = 1`) that supercharges melee and evasion but
burns stamina.

- **Requires:** `S_POWER > 20000` (check `<= 20000`). **Cost:** `S_POWER -= 20000`. **Lag:** `WAIT_STATE(ch, 12)`.
- **Combat effects while affected (`fight.c`):**
  - **Melee bonus:** in addition to the standard `dam += dam * S_STRENGTH / 175`, adds
    `dam += dam * S_STRENGTH_MAX / 1000`.
  - **Extra attacks / evasion:** improves dodge/parry math (e.g. the parry-against branch
    `return FALSE` while Kaiouken, doubled dodge chance `chance *= 2`, etc.).
  - **vs Fist:** each landed melee hit drains the victim's `F_KI--` (if `F_KI > 0` and not in Figure-Eight).
  - **vs windy shield:** reduces the shield's deflect chance by `num/4`.
  - **Cost over time:** each tick consumes `kai = dice(10,20)` move; if `move < kai`, the affect is
    stripped ("Exhausted, you cease using the Kaiouken attack").

### Ki Wall vs incoming Ki, aegis interactions
See **Passive defenses** below — the `S_AEGIS` pool is the Saiyan's main innate damage reduction and
is consumed as it absorbs.

---

## 4. Passive techniques (learned but no command)

These are bought via `technique` but have **no active command**; they alter combat math when the bit
is set or trigger automatically.

### Zanzouken (after-image / extra attacks) — passive
*Source: `src/core/fight.c` (number-of-attacks calc), bit `S_ZANZOUKEN`*

- **Effect:** in the Saiyan attack-count routine, `if IS_SET(S_TECH, S_ZANZOUKEN) atk += 2` — **+2
  attacks per round**. (Also in that routine: `S_SPEED > 50` → `atk++`; `S_SPEED >= 150` →
  `atk += S_SPEED/150`.)
- No power cost, no command — purely a learned passive.

### Kiaihou (auto aegis refocus) — passive
*Source: `src/core/fight.c` (~line 195), bits `S_KIAIHOU` / `NEW_KIAIHOU`*

Auto-restores aegis when struck, if learned and the Saiyan is `level >= 2`.

- **Trigger:** when the Saiyan is the victim and `NEW_KIAIHOU` is set, on being hit it tries to refill
  aegis: `i = S_AEGIS_MAX - S_AEGIS`; if `S_POWER <= i*2` it's "too tired" (no refill); else
  `S_POWER -= i*2` and `S_AEGIS = S_AEGIS_MAX`.
- **Arming:** if the bit `S_KIAIHOU` is learned but `NEW_KIAIHOU` isn't yet set, the first qualifying
  hit sets `NEW_KIAIHOU` (arms it); the refocus then fires on subsequent hits.
- No command, no separate lag.

---

## 5. Utility

### Flight — `flight`
*Source: `src/classes/saiyan.c:do_flight` · POS_STANDING*

- **Effect:** casts `fly` at `level = spirit` (`skill_table[fly].spell_fun`).
- **Requires:** `S_POWER >= 250` (check `< 250`). **Cost:** `S_POWER -= 250`. **Lag:** `WAIT_STATE(ch, 12)`.

### Shunkan Idou (instant transmission) — `shunkanidou <target>`
*Source: `src/classes/saiyan.c:do_shunkanidou` · POS_STANDING*

World-wide teleport to a target's room.

- **Requires:** `S_POWER > 10000` (check `<= 10000`). **Cost:** `S_POWER -= 10000`. **Lag:** `WAIT_STATE(ch, 12)`.
- **Restrictions:** target must exist via `get_char_world`; **NPC-only** ("Their Ki is shielded" for
  players); target must have `level <= spirit/2`, not be fighting, and be in a room; blocked while
  `NEW_RETIRED`, in a `ROOM_HQ`, or with a fight timer (`extras[TIMER] > 0`).
- **Landing safety:** if the target's room vnum is in protected ranges (`8900–8999`, `5300–5399`,
  `8400–8599`, `8100–8299`), you are redirected to vnum `5148` instead.

### Ki Sense — `kisense <target>`
*Source: `src/classes/saiyan.c:do_kisense` · POS_STANDING*

World-wide locate: prints the target's general area. No power cost.

- **Effect:** `get_char_world`, then prints "You locate `<name>` in the vicinity of `<area>`".
- **Anti-scry (vs Patryn, graded by Fire runes on TORSO; requires victim `level >= 2` and `P_DEFENSES`):**
  - **5+ Fire runes** *or* `gsn_spirit_ward`: fully blocked ("Their Ki is shrouded").
  - **3–4 Fire runes:** location denied ("the trail is clouded").
  - **1–2 Fire runes:** warning glow to the Patryn only; sense still succeeds.
- **Lag:** `WAIT_STATE(ch, 8)`.

### Battlesense — `battlesense <target>`
*Source: `src/classes/saiyan.c:do_battlesense` · POS_FIGHTING*

In-room read: HP / mana / move / class, plus class-specific intel. No power cost.

- **Effect:** prints victim HP, mana, moves, and class (`who_name`).
- **Restriction:** on players, only works while you are fighting (`ch->fighting != NULL` and
  `position == POS_FIGHTING`), else "Their Ki is shielded".
- **Class intel:** for a **Saiyan** target, reveals exact `S_POWER`; for **Fist**, a qualitative
  `F_KI` readout (charging / focused / bursting); for **Mazoku**, a qualitative `M_NIHILISM` readout.

### Hawk Eyes — `hawkeyes`
*Source: `src/classes/saiyan.c:do_hawkeyes` · POS_STANDING*

Toggle true sight (`PLR_TRUESIGHT`).

- **Turn on:** `S_POWER -= 500`, set `PLR_TRUESIGHT`. **Requires:** `S_POWER > 500` (check `<= 500`).
- **Turn off:** if already on, removes the bit for **free** (no cost, no power needed).
- **Lag:** none.

---

## 6. Passive defenses & damage scaling (always-on, `src/core/fight.c`)

These are not commands but are essential for balance — they govern how much damage a Saiyan deals and
absorbs based on the `S_STRENGTH`, `S_SPEED`, and `S_AEGIS` pools.

- **Melee damage bonus** (`level >= 2`): `dam += dam * S_STRENGTH / 175` (plus the Kaiouken bonus above).
- **Barehand damage:** with `S_AEGIS > 0`, unarmed hits use `number_range(weapon/8+20, weapon/5+45)`
  (a Saiyan with aegis up hits hard barehanded).
- **Extra attacks from speed:** `S_SPEED > 50` → +1 attack; `S_SPEED >= 150` → `+ S_SPEED/150`
  (Zanzouken adds +2 on top).
- **`DAM_KIFLAME` resist:** vs a Saiyan victim with `S_AEGIS > 0`: `dam -= dam * S_AEGIS / 2100`.
- **General toughness (`level >= 2`, Saiyan victim):**
  - Mystickal damage (`dt` in `1400..1479`): `dam -= dam * S_AEGIS / 3000`.
  - With `S_AEGIS > 0`: `dam -= dam * (S_AEGIS + number_range(1,300)) / 1500`, **and aegis is consumed**:
    `S_AEGIS = UMAX(S_AEGIS_MAX/2, S_AEGIS - isquare(dam))` (floor at half-max).
  - `gsn_kiwall` soak stacks on top (see Ki Wall).
  - `DAM_OBSIDIAN` *drains* strength/speed/aegis by `dam/5` each (floored at half-max).
- **Weapon disarm immunity:** a Saiyan with `S_AEGIS >= 250` and no wielded weapon can still parry
  (barehand parry allowed).
- **On near-death (`fight.c` ~2490):** when reduced to `move = 1`, a Saiyan's pools reset to
  `S_POWER = 5` and strength/speed/aegis to half their `_MAX`.

---

## ⚠️ Needs verification

- **Kamehameha live damage.** Confirmed the live value is the `update.c` `NEW_KAME_2` formula
  `spirit*105 + dice(15, spirit)`; the `saiyan.c` block contains an **older, commented-out** formula
  (`spirit*115 + dice(10, spirit)`). Double-check no other path re-applies Kamehameha damage.
- **Ki Wall soak ordering vs aegis consumption.** Ki Wall's `dam /= 3` and the aegis
  `dam -= dam*(S_AEGIS+rand)/1500` both apply in the same `fight.c` victim block; the exact ordering
  (and whether aegis is consumed when Ki Wall is also up) should be confirmed against the live order
  of operations in `damage()`.
- **`F_UPPERCUT` ward interactions for Ryuken part 1.** Ryuken's first hit is typed `F_UPPERCUT`,
  which shares ward-mitigation code with Fist uppercuts (e.g. `gsn_earth_ward` reduces it). The exact
  mitigation magnitude vs Saiyan Ryuken specifically was not fully traced.
- **Kaiouken evasion magnitudes.** The dodge/parry `*= 2` and `return FALSE` branches are confirmed
  present, but the net hit/dodge percentages depend on surrounding `chance` math not fully reproduced here.
- **`isquare()` semantics.** Assumed integer square root; confirm in `src/*.c` if a precise rage/aegis
  curve is needed.
