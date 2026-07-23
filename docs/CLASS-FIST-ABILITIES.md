# Fist — Ability Reference (Balance / Mechanics)

> **Audience:** balance review and maintainers. This is the exhaustive, numbers-first reference
> for **every** Fist ability, extracted directly from the server source. For the new-player
> narrative version, see **[Fist — Player Guide](CLASS-FIST.md)** (do not balance-check against
> that doc; this one is authoritative).

**Last verified against source:** commit `f0e24e1`
**Primary sources:** `src/classes/fist.c`, `src/core/fight.c`, `src/core/update.c`, `src/core/handler.c`, `src/core/const.c`, `src/include/merc.h`, `src/core/interp.c`

## How to read this

- **Each ability is cited `file:function`** so every number can be checked against the C. If a number
  here disagrees with the code, the **code wins** — fix this doc.
- **Resources:** the Fist is **not** a mana class. It runs on four pools:
  - **`F_KI`** (`ch->pcdata->powers[F_KI]`, 0–`F_KI_MAX`): a volatile *combat charge* built by landing
    basic strikes. It is the heart of the class — the **exact** value (10–30) selects which `combo`
    technique fires. It collapses to 0 out of combat or when overcharged past 30.
  - **`move`** (stamina): spent by `combo`, `roundhouse`, `levitate`, `phoenixaura`, `dim-mak`.
  - **`exp`**: spent permanently to `master` moves/Ki, raise `discipline`, and `bodytrain`.
  - **`primal`**: spent only by `master swap` (150).
- **Comparisons matter at the margin** — the exact `<`, `<=`, `>`, `>=` is quoted per ability.
- **Formulas are quoted verbatim** from the source. `dice(n, size)` rolls `n` dice of `1..size`
  (mean ≈ `n*(size+1)/2`). `body` = `ch->pcdata->body`, `spirit` = `ch->pcdata->spirit`,
  `mind` = `ch->pcdata->mind`, `weapon` = `ch->pcdata->weapons[0]` (the bare-hand "weapon" skill).
  `isquare(n)` = integer floor of `sqrt(n)` (`src/xrand.c:isquare`).
- **WAIT_STATE(ch, n)** is lag in pulses (`PULSE_VIOLENCE`), the standard combat-round unit.
- **Damage types** route through `damage()` in `src/core/fight.c`; the melee `F_*` types
  (`F_SHINKICK`..`F_JUMPKICK`, `F_PALMTHRUST`, `F_STOMP`, `F_DEATHTOUCH`) and `DAM_KIFLAME` have
  Fist-specific resist/soak handling, documented under *Passive defenses* below.
- **⚠️ Needs verification** items are collected at the bottom — don't trust those numbers blindly.

## Class basics

Source: `src/core/const.c` `class_table[]` (the `"Fist"` row), `src/include/merc.h` (struct `class_type`).

| Field | Value | Notes |
|---|---|---|
| `who_name` | `Fist` | |
| `attr_prime` | `APPLY_STR` | Prime attribute = **Strength** |
| First weapon | `OBJ_VNUM_SCHOOL_SWORD` | Newbie weapon |
| Guild room | `3022` | |
| Max trainable Body | `90` | Fuels combo damage + figure-eight soak |
| Max trainable Mind | `75` | |
| Max trainable Spirit | `100` | Fuels Ki-flame combos (Hadoken/Gadouken/Masher) |
| Max trainable Will | `80` | |
| HP gained per level | `15` | |
| Gains mana | `FALSE` | Not a mana class — resource is Ki + stamina |

**Power slots** (`src/include/merc.h`, indices into `ch->pcdata->powers[]`):
`F_KI`=0 / `F_KI_MAX`=1 (Ki charge + its cap), `F_LEARNED`=2 (highest mastered basic strike),
`F_TORSO`=3 / `F_ARMS`=4 / `F_HANDS`=5 / `F_LEGS`=6 (body-part training, each 0–100),
`F_DISC`=7 (discipline rank 0–10), `F_MASTER`=8 (packed base-10 digits: per-strike Ki-gain settings).

**Combat / state flags** (`ch->pcdata->actnew`): `NEW_FIGUREEIGHT`=512 (figure-eight defense),
`NEW_NOBLOCK`=1024 (Maiden-Masher whiff, can't block), `NEW_INNERFIRE`=256 (pending cleanse),
`NEW_PHOENIX_AURA`=262144 (offense aura). **Melee damage types** (`src/include/merc.h`):
`F_SHINKICK`=1300, `F_JAB`=1301, `F_SPINKICK`=1302, `F_KNEE`=1303, `F_ELBOW`=1304, `F_UPPERCUT`=1305,
`F_STOMP`=1306, `F_JUMPKICK`=1307, `F_DEATHTOUCH`=1308, `F_PALMTHRUST`=1309.

All Fist commands are `level 2` in the command table (`src/core/interp.c`); position is noted per ability.

---

## 1. Advancement & training (out-of-combat)

### Master — `master [list | <move> | ki | swap <m1> <m2>]`
*Source: `src/classes/fist.c:do_master` · `interp.c`: POS_STANDING, level 2*

The permanent unlock command for basic strikes and the Ki cap.

- **`master list`** (or no arg): prints a grid of which strikes are learned (`F_LEARNED >= F_*`), each
  strike's configured Ki-gain digit (from `F_MASTER`), and `Maximum Ki: F_KI_MAX`.
- **Learn a strike** (`shinkick`/`jab`/`spinkick`/`knee`/`elbow`/`uppercut`):
  - **Cost:** `500000 exp` each (fails if `ch->exp <= 500000`).
  - **Sequential gate:** must learn in order — fails with "must master a simpler technique first" if
    `F_LEARNED < (ability-1)` (except `shinkick`, the first). On success `F_LEARNED = ability`.
- **`master ki`:** raises the Ki cap. **Cost:** `500000 exp` (fails if `ch->exp < 500000`);
  `F_KI_MAX++`, capped at **30** ("developed your Ki to its maximum potential" at `F_KI_MAX >= 30`).
- **`master swap <m1> <m2>`:** swaps two strikes' Ki-gain digits inside `F_MASTER`.
  **Cost:** `150 primal` (fails if `primal < 150`). Internally moves the base-10 digits at positions
  `F_*-1300` (0–5) using `pow(10, move)`.

### Discipline — `discipline`
*Source: `src/classes/fist.c:do_discipline` · POS_STANDING, level 2*

Raises `F_DISC` (rank 0–10), which gates the discipline abilities and several passives.

- **Cost:** `cost = 500000 * (1 + F_DISC)` exp (fails if `ch->exp < cost`).
- **Cap:** `F_DISC >= 10` → "already mastered all your disciplines".
- On success `F_DISC++`.

**What each rank unlocks** (cross-referenced below): `>=1` eyesight; `>=2` raises bare-hand weapon-skill
training cap to 500 (`one_hit`); `>=3` raises stance training cap to 300 (`one_hit`); `>=4` levitate;
`>=5` roundhouse; `>=6` tackles fleeing NPCs (`do_flee`); `>=7` innerfire; `>=8` phoenixaura; `>=9` dim-mak.

### Bodytrain — `bodytrain <torso|hands|arms|legs>`
*Source: `src/classes/fist.c:do_bodytrain` · `interp.c`: POS_FIGHTING, level 2*

Trains a body part (each 0–100). The parts feed melee scaling and defense (see *Passive defenses*).

- **Cost (torso):** `1500 * (F_TORSO+1) * isquare(F_TORSO+1)` exp.
- **Cost (hands):** `1500 * (F_HANDS+1) * isquare(F_HANDS+1)` exp.
- **Cost (arms):** `1500 * (F_ARMS+1) * isquare(F_HANDS+1)` exp. *(Note: the `isquare` term uses
  `F_HANDS+1`, not `F_ARMS+1` — see ⚠️.)*
- **Cost (legs):** `1500 * (F_LEGS+1) * isquare(F_HANDS+1)` exp. *(Same `F_HANDS` quirk — see ⚠️.)*
- **Cap:** `powers[abil] >= 100` → "already attained peak physical condition".
- Fails if `cost > ch->exp`; on success `ch->exp -= cost` and `powers[abil]++`.

---

## 2. Basic strikes (the Ki engine)

The six `master`ed strikes are the only way to **build** `F_KI`. Each must be learned via `master`
(`F_LEARNED >= F_*`) or it prints "You have not yet mastered that attack." All six share the same
shape: roll damage, deal it, then randomly nudge Ki and gain it.

- **Common damage roll:** `dam = dice( 10-ki, weapon/4 )`, where `ki` is that strike's **configured
  Ki-gain digit** (see below). A *higher* configured Ki gain means *fewer* damage dice — the tradeoff
  is damage-per-hit vs Ki-per-hit.
- **Ki-gain digit source** (`F_MASTER` is packed base-10, one digit per strike):
  | Strike | Damage type | `ki` (gain digit) | Source line |
  |---|---|---|---|
  | `shinkick` | `F_SHINKICK` | `F_MASTER % 10` | `do_shinkick` |
  | `jab` | `F_JAB` | `(F_MASTER/10) % 10` | `do_jab` |
  | `spinkick` | `F_SPINKICK` | `(F_MASTER/100) % 10` | `do_spinkick` |
  | `knee` | `F_KNEE` | `(F_MASTER/1000) % 10` | `do_knee` |
  | `elbow` | `F_ELBOW` | `(F_MASTER/10000) % 10` | `do_elbow` |
  | `uppercut` | `F_UPPERCUT` | `F_MASTER/100000` | `do_uppercut` |
- **Ki nudge after the hit** (all six, identical): `num = number_percent()`; if `NEW_PHOENIX_AURA` is
  set, `num += F_KI` (so a high current Ki pushes `num` *up*, away from the low thresholds, suppressing
  further Ki gain). Then `if (num < 15) ki += 1; else if (num < 30) ki -= 1;`. Finally `gain_ki(ch, ki)`.
- **Lag:** `WAIT_STATE(ch, 8)` for every strike.
- **Targeting:** `shinkick`/`jab`/`spinkick`/`knee` require `ch->fighting != NULL` ("not in combat").
  `elbow` and `uppercut` additionally accept a room target when not fighting (`get_char_room`), and
  refuse player targets below level 2 ("Not on mortals.").

### Ki accounting — `gain_ki(ch, ki)`
*Source: `src/classes/fist.c:gain_ki` (helper, not a command)*

- If the passed `ki > F_KI_MAX`, the gain is **rejected** outright (`return`).
- If `F_KI + ki > F_KI_MAX`: "Your ki exceeds your training, and you lose control of it." → `F_KI = 0`.
- Otherwise `F_KI += ki`, then **overextend check:** if `F_KI > 30` → "you overextended" → `F_KI = 0`.
- Threshold flavor: crossing `F_KI >= 10` and `>= 20` prints intensifying glow messages.
- **Decay:** out of combat, a tick with `F_KI > 0` and no `NEW_PHOENIX_AURA` collapses Ki to 0
  ("The aura of Ki around you collapses." — `src/core/update.c`, the Fist regen branch).

---

## 3. Combo — `combo`
*Source: `src/classes/fist.c:do_combo` · `interp.c`: POS_FIGHTING, level 2*

The signature mechanic. The **exact** current `F_KI` value (10–30) selects which technique fires — so a
Fist charges to a precise number with basic strikes, then unleashes. **Every path ends by setting
`F_KI = 0`.**

- **Entry gates:** must be `CLASS_FIST`, `ch->fighting != NULL`, and `F_KI >= 10`
  ("Your Ki is not excited enough." if `< 10`).
- **Stamina cost / gate:** `cost = F_KI * (100 - F_DISC*8)`. If `move <= cost` → "too exhausted",
  `F_KI = 0`, `WAIT_STATE(ch, 8)`, abort. Otherwise `move -= cost`. *(At max discipline the multiplier
  is `100 - 80 = 20`, i.e. `20*F_KI` move; `F_DISC` is capped at 10 so the multiplier never goes ≤ 0.)*
- **Default tail (all cases that `break`):** `WAIT_STATE(ch, 4)` then `F_KI = 0`. Cases that `return`
  early (case 10) set their own lag.

### Combo table (by exact `F_KI`)

| `F_KI` | Name | Effect / formula | Damage type |
|---|---|---|---|
| 10 | Quivering Palm | See detail below. `WAIT_STATE(ch, 4)`, returns early. | — / drain |
| 11 | Toe-break stomp | `dam = dice(10, body)` | `F_STOMP` |
| 12 | Di-Amon Mega Rotation Death | `dam = dice(1, weapon/2)` dealt **twice** | `F_SHINKICK` ×2 |
| 13 | Hadoken | `dam = dice(body/2, weapon/3)` | `DAM_KIFLAME` |
| 14 | Figure-Eight | Sets `NEW_FIGUREEIGHT` (defensive — see Passives). No damage. | — |
| 15 | Boot to the head (jumpkick) | `dam = dice(body/2, weapon*2/5)` | `F_JUMPKICK` |
| 16 | Failed bodyslam | `unstance(ch)` (you knock *yourself* out of stance). No damage. | — |
| 17 | Balance slam | `unstance(victim)`. No damage. | — |
| 18 | Tangle | `unstance(ch)` **and** `unstance(victim)`. No damage. | — |
| 19 | Fists of Fury | `dice(6,weapon/4)`,`dice(6,weapon/3)`,`dice(6,weapon/2)` then `dice(15,weapon/2)` | `F_JAB`×3, `F_UPPERCUT` |
| 20 | Palm Thrust | Defense-shatter; see detail below. `dam = dice(body/3, weapon)` (×3 vs Fist) | `F_PALMTHRUST` |
| 21 | Stunning routine | Escalating `dice(3..14, weapon/3)` chain | `F_SHINKICK`,`F_JAB`,`F_SPINKICK`,`F_KNEE`,`F_ELBOW`,`F_UPPERCUT` |
| 22 | Hesitation (whiff) | No damage; `WAIT_STATE(ch, 8)` | — |
| 23 | Touch of Death | `dam = UMAX(victim->hit * 2/5, 2500)` | `F_DEATHTOUCH` |
| 24 | Deathtouch rebound | `dam = UMAX(ch->hit / 4, 2500)` dealt to **self** | `F_DEATHTOUCH` |
| 25 | Maiden Masher whiff | Sets `NEW_NOBLOCK`; `WAIT_STATE(ch, 8)`. No damage. | — |
| 26 | Maiden Masher | `dice(12/14/17/19, weapon/2)` spinkick/uppercut interleaved with `dice(5/7/8/20, spirit)` ki-flame | `F_SPINKICK`/`F_UPPERCUT`, `DAM_KIFLAME` |
| 27 | Gadouken | `dam = dice(1, hitroll * body / 3)` | `DAM_KIFLAME` |
| 28 | Shinkyuu Hadoken | `dice(22, body/2 + weapon/4)`, `dice(22, body + weapon/4)`, `dice(22, body + weapon/2)` | `DAM_KIFLAME` ×3 |
| 29 | Blaze buff | Self-buff (golden/dark/judicators); see detail below. No damage. | — |
| 30 | Instant Hell Murder | `level >= 20` only; loops random `dice(10..16, 500)` strikes until `victim->position <= POS_STUNNED` | `F_JAB`/`F_ELBOW`/`F_KNEE`/`F_UPPERCUT` |

> **Note:** values **outside 10–30** fall through `default:` (no effect) but still hit the default
> tail (`WAIT_STATE 4`, `F_KI = 0`). Since the entry gate already requires `F_KI >= 10` and `gain_ki`
> collapses anything `> 30`, the live window is 10–30.

### Combo 10 — Quivering Palm (detail)
*`do_combo` case 10*

- **vs non-Fist victim:** "twitches violently", `victim->mana = UMAX(1, mana - (1500 + dice(5,100)))`.
  - **vs Saiyan:** `F_KI = victim S_POWER / 1000`; `victim S_POWER = UMAX(1, S_POWER - dice(10,500))`.
    If the Saiyan is mid-charge (`NEW_KAME_1`/`NEW_KAME_2`/`NEW_MASENKOUHA`), the charge is **absorbed
    and converted to self-healing**: clears the charge bit and `ch->hit = UMIN(max_hit, hit + dam)`
    where `dam = spirit*10 + dice(10, spirit)` for Kame, or `spirit*5 + dice(8, spirit)` for Masenkouha.
  - **vs other non-Fist:** `F_KI = 0` (no extra effect beyond the mana burn).
- **vs Fist victim:** if their `F_KI <= 0`, "nothing happens" (`F_KI = 0`); otherwise **steals Ki**:
  `ch F_KI = 9 + victim F_KI/2`, `victim F_KI /= 2`.
- **Lag:** `WAIT_STATE(ch, 4)`, returns (skips default tail).

### Combo 20 — Palm Thrust (detail)
*`do_combo` case 20*

- **vs NPC:** "nothing happens" (no damage).
- **vs player:** only fires if the victim is in a major setup/defense state — any of
  `gsn_kiwall`, `NEW_FIGUREEIGHT`, `gsn_defense`, or `NEW_VAN_REHL`. If so it **strips all** of those
  present, then `dam = dice(body/3, weapon)` as `F_PALMTHRUST`, **`dam *= 3` if the victim is a Fist**.
  If the victim has none of those states: "nothing happens".

### Combo 29 — Blaze buff (detail)
*`do_combo` case 29*

First strips any existing `gsn_golden_blaze`/`gsn_judicators_ire`/`gsn_dark_blaze`, then applies one by
argument, all with `duration = spirit*10`:

| `combo` arg | Affect | AC modifier (`APPLY_AC`) |
|---|---|---|
| `holy` | `gsn_golden_blaze` | `spirit / -10` |
| `dark` | `gsn_dark_blaze` | `spirit / -10` |
| *(none / other)* | `gsn_judicators_ire` | `spirit / -2` |

These blazes proc **bonus damage on every normal punch (`TYPE_HIT`) vs an NPC** (`src/core/fight.c:damage`,
gated to `IS_CLASS(ch,CLASS_FIST) && IS_NPC(victim)`):
- `gsn_golden_blaze`: extra `400 + number_percent()` `DAM_KIFLAME`.
- `gsn_judicators_ire`: extra `400 + number_percent()` `DAM_SHOCKWAVE`.
- `gsn_dark_blaze`: 50% chance (`number_percent() < 50`) of `200 + number_percent()` `DAM_NEGATIVE`,
  and **lifesteals** the HP dealt back to `ch->hit` and `ch->move` (capped at max).

---

## 4. Discipline abilities

### Eyesight — `eyesight`
*Source: `src/classes/fist.c:do_eyesight` · POS_STANDING*

- **Gate:** `F_DISC >= 1`.
- **Effect:** toggles `PLR_TRUESIGHT` (free, no cost, no lag).

### Levitate — `levitate`
*Source: `src/classes/fist.c:do_levitate` · POS_STANDING*

- **Gate:** `F_DISC >= 4`; requires `move >= 1000` ("need 1000 move").
- **Effect:** `move -= 1000`, then casts `fly` at `lev = spirit / 3`
  (`skill_table[fly].spell_fun`). No `WAIT_STATE`.

### Roundhouse — `roundhouse`
*Source: `src/classes/fist.c:do_roundhouse` · POS_FIGHTING*

Room-wide AoE kick.

- **Gate:** `F_DISC >= 5`; requires `move >= 150` ("too tired").
- **Damage (each non-group, visible target):** `dam = body + dice(15, F_LEGS*2)` — `F_SPINKICK`.
- **Cost:** `move -= 150`. **Lag:** `WAIT_STATE(ch, 12)`.

### Innerfire — `innerfire`
*Source: `src/classes/fist.c:do_innerfire` · POS_FIGHTING*

A self-cleanse: arms now, fires on the next tick.

- **Gate:** `F_DISC >= 7`; requires `F_KI >= 1`; fails if `NEW_INNERFIRE` already set.
- **Concentration check:** if `dice(1,15) > F_KI` → lose focus, `F_KI = 0`, abort (so higher Ki = more
  reliable). On success: `SET_BIT(NEW_INNERFIRE)`, `F_KI = 0`.
- **Resolve (next tick, `src/core/update.c` Fist branch):** clears `NEW_INNERFIRE` and **removes every
  affect on the character** (`while (ch->affected) affect_remove(...)`) — strips debuffs *and* buffs.

### Phoenixaura — `phoenixaura`
*Source: `src/classes/fist.c:do_phoenixaura` (toggle) + `src/core/update.c` (upkeep) · POS_FIGHTING*

A toggled offense aura (`NEW_PHOENIX_AURA`).

- **Gate:** `F_DISC >= 8`.
- **Turn on:** requires `move >= 500` ("too exhausted"); sets the bit, `hitroll += 15`, `damroll += 15`,
  `move -= 500`.
- **Turn off (re-issue):** clears the bit, `hitroll -= 15`, `damroll -= 15`.
- **Upkeep (`src/core/update.c`, Fist branch):** each tick, if `move < 250` the aura **collapses**
  (clears bit, `hitroll -= 15`, `damroll -= 15`); otherwise `move -= 150 + dice(10,10)`.
- **Also collapses** when not `POS_FIGHTING` (`src/core/update.c` ~line 649: clears bit, removes the
  ±15 hitroll/damroll, and zeroes `F_KI`).
- **Side effect on basic strikes:** while active, the Ki-nudge adds `F_KI` to the random roll
  (`num += F_KI`), suppressing further Ki swings (see §2).

### Dim-Mak — `dim-mak [target]`
*Source: `src/classes/fist.c:do_dim_mak` · `interp.c`: POS_FIGHTING, level 2*

The Fist's premier nuke; scales off **player-kill count**, and Ki *reduces* its stamina cost.

- **Gate:** `F_DISC >= 9`.
- **Cost model:** if `F_KI > 0`, `kicost = URANGE(0, F_KI, 20)` and `movecost = 1000 - kicost*50`
  (spending up to 20 Ki cuts the move cost from 1000 down to 0); else `kicost = 0`, `movecost = 1000`.
  Fails if `move < movecost` ("too exhausted").
- **Damage:** `dam = dice(10, 200 + isquare(ch->pcdata->kills[PK]))` — `F_DEATHTOUCH`.
  - **vs Mazoku:** `dam *= 2` if `M_TRUE`; `dam *= 5` if `M_ASTRIKE` (multiplicative, so both → ×10),
    and a landed `M_ASTRIKE` strike is then stripped.
  - **vs Sorcerer:** `number_percent() < 60` → `lose_chant(victim)` (interrupts the chant).
- **Targeting:** with no argument, defaults to the current opponent (`ch->fighting`), like the other
  fist attacks; with an argument, `get_char_room`. "Poison who?" only when idle with no argument.
- **Cost applied:** `move -= movecost`, `F_KI -= kicost`. **Lag:** `WAIT_STATE(ch, 10)`.

---

## 5. Passive defenses & damage scaling (always-on, `src/core/fight.c`)

Not commands, but central to balance. These govern how much a Fist deals and absorbs.

### Offense
- **Hands melee bonus** (`one_hit`, `level >= 2`, `F_HANDS > 0`): `dam += dam * F_HANDS / 33`
  (up to ≈ +303% at 100 hands).
- **Bare-hand base damage** (`one_hit`, unarmed): Fist uses `dam = number_range(weapon/8+1, weapon/3+5)`
  (below level 2, `weapon` is first clamped to `UMIN(200, weapon)`).
- **Kick command** (`do_kick`, `level >= 2`): Fist bare-hand kick rolls
  `number_range(weapon*2, weapon*4 + F_LEGS*15)`; vs players or NPCs `level > 95`, `dam /= 15`.
  Lag = `skill_table[gsn_kick].beats` (= 8).
- **Skill-up caps from discipline** (`one_hit`): `F_DISC >= 2` raises the bare-hand weapon-skill
  training cap from 200 → 500 on `TYPE_HIT`; `F_DISC >= 3` raises the stance training cap from 200 → 300.
- **Attack count** (`calc_attacks`): the Fist has **no class-specific extra-attack bonus** — attacks
  come only from base 1 + stance + the hitroll-vs-hitroll term (`URANGE(0,(chit-vhit)/10,3)`).
- **EVAL rating** (`src/core/handler.c:eval`, caps how much hitroll/damroll counts in combat):
  `point += F_DISC/2 + Σ_{i=3..6} (powers[i]+1)/20 + max_move/1500 + max_mana/3000`.

### Defense (Fist as victim, `src/core/fight.c:damage`, `level >= 2`)
- **Torso toughness:** `dam -= dam * ( number_range(1,15) + (25 + F_TORSO/2) ) / 100`
  (a flat ~25–40%+ melee/general soak that scales with torso).
- **Figure-Eight** (`NEW_FIGUREEIGHT` set): `dam /= 3` **before** the torso reduction — a large blanket
  damage cut. It also makes the Fist immune to the Kaiouken/Ki-Wave Ki-drain (below). Figure-eight
  decays each tick with probability `100 - body/2` percent (`src/core/update.c`), so high body = it lasts longer.
- **Spell/mystic damage** (`chant_damage`): figure-eight gives `dam /= 2`; torso gives
  `dam -= dam * (dice(1,10) + F_TORSO) / 127`.
- **Block** (`check_block`): Fist-only active defense, requires `level >= 2`, no wielded weapon, not a
  mobile suit, not `NEW_NOBLOCK`. `chance = (F_ARMS/2) + (weapon/10)`, reduced by attacker class
  (Saiyan speed, Mazoku claws/spikes, Patryn earth-leg runes; **auto-fails vs a Kaiouken Saiyan**),
  then `URANGE(25, chance, 85)`. Note a Fist attacker reduces a *defender's* block by `F_ARMS*3/8`.
- **Defense barrier vs Fist melee** (`damage`): a victim affected by `gsn_defense` fully blocks any
  `F_SHINKICK..F_JUMPKICK` strike ("defense barrier blocks").
- **Tackle on flee** (`do_flee`): vs a fleeing NPC, a Fist with `F_DISC >= 6` has >50% chance to tackle
  it (cancels the flee, `WAIT_STATE` on the mob).

### Cross-class interactions where the Fist is on the receiving end
- **vs Saiyan Kaiouken / Ki-Wave Ki-drain** (`damage`, `saiyan.c`): each landed Kaiouken melee hit
  drains `F_KI--` (only if `F_KI > 0` and **not** in figure-eight); Ki-Wave drains `F_KI -= dice(2,3)+1`
  and clears `NEW_FIGUREEIGHT`. (Documented authoritatively in the Saiyan reference.)
- **vs Sorcerer, with `AFF_CHAOS_STRING` on the Fist** (`damage`): the Fist's own melee
  (`TYPE_HIT..TYPE_HIT+14` or `F_SHINKICK..F_JUMPKICK`) against a Sorcerer is **halved** (`dam /= 2`).
- **Backstabbed:** a Fist victim takes `dam *= 3` from a non-Fist backstab (`one_hit`) — Fists are
  extra-vulnerable to backstab.

---

## ⚠️ Needs verification

- **Bodytrain `isquare` quirk.** `arms`, `hands`, and `legs` all compute their cost's `isquare` term
  from `F_HANDS+1` (only `torso` uses its own part). This looks like an upstream copy-paste bug:
  training arms/legs gets cheaper/dearer as *hands* rise, not the trained part. Documented verbatim;
  confirm whether this is intended before any rebalance.
- **`F_MASTER` digit ranges.** Each strike's Ki-gain digit is one base-10 digit (0–9), but `uppercut`
  reads `F_MASTER/100000` with no `% 10`, so it captures any overflow above position 5. In normal play
  digits stay 0–9; confirm no path can push a digit ≥ 10 and corrupt the packing (and that
  `dice(10-ki, ...)` never gets a non-positive dice count, which `dice` would need to handle).
- **Combo 30 "Instant Hell Murder" loop.** The `while (victim && victim->position > POS_STUNNED)` loop
  re-enters `damage()` repeatedly with no per-iteration lag; confirm `damage()` can't free `victim`
  mid-loop in a way the `victim &&` guard misses (use-after-free risk), and that it terminates promptly.
- **Phoenix-aura `num += F_KI` intent.** Adding current `F_KI` to the strike's random roll mathematically
  *suppresses* Ki gain at high Ki (pushes `num` past the `<15`/`<30` thresholds). Confirm this is the
  intended "aura stabilizes your Ki" behavior rather than an inverted comparison.
- **`AFF_CHAOS_STRING` provenance.** Confirmed it halves the Fist's melee vs Sorcerers, but the bit is
  applied elsewhere (a Sorcerer debuff); trace its source/duration if balancing the Fist-vs-Sorcerer
  matchup.
- **Damage-mod ordering in `damage()`.** Figure-eight `dam /= 3`, torso reduction, and the various
  cross-class soaks all execute in the same victim block; the exact order (and interaction with the
  `DAM_KIFLAME` Saiyan-resist path) should be confirmed against the live order of operations.
