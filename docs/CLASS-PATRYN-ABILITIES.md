# Patryn — Ability Reference (Balance / Mechanics)

> **Audience:** balance review and maintainers. This is the exhaustive, numbers-first reference
> for **every** Patryn ability, extracted directly from the server source. For the new-player
> narrative version, see **[Patryn — Player Guide](CLASS-PATRYN.md)** (do not balance-check against
> that doc; this one is authoritative).

**Last verified against source:** commit `f0e24e1` (gust chant-interrupt section, #122; rest verified at `cefb094`)
**Primary sources:** `src/classes/patryn.c`, `src/core/fight.c`, `src/classes/saiyan.c`, `src/classes/sorcerer.c`, `src/core/const.c`, `src/include/merc.h`, `src/core/interp.c`

## How to read this

- **Each ability is cited `file:function`** so every number can be checked against the C. If a number
  here disagrees with the code, the **code wins** — fix this doc.
- **Resource:** runeweaves spend **mana** (no self-generated pool). Learning runes spends **primal**;
  training primary-rune strength spends **exp**; tattooing/erasing body runes spends **primal + mana**.
- **The Patryn is unusual:** almost every "spell" is a **runeweave** — a pairing of one *primary*
  rune (air/earth/fire/water/energy/negative) with one *secondary* rune (life/death/creation/
  destruction/protection/transformation/movement/abjuration). `do_runeweave` is one big `switch`
  keyed on the integer **`spell = rune1 + rune2`** (the sum of the two rune **bit values**), so e.g.
  FIRE(4)+PROTECTION(1024) → case `1028` (balus wall). Each combo is documented with its case code.
- **Body runes vs known runes are different things.** *Knowing* a rune (`learn`) unlocks it for
  runeweaving and raises `P_LEARNED_NUM`. *Tattooing* a rune onto a body part (`tattoo`) is what
  drives the passive combat math in `fight.c` (parry, dodge, mitigation, extra attacks). Only
  **air/earth/fire/water** can be tattooed — energy/negative have no body presence.
- **Formulas are quoted verbatim** from the source. `dice(n, size)` rolls `n` dice of `1..size`
  (mean ≈ `n*(size+1)/2`). `mind` = `ch->pcdata->mind`, `will` = `ch->pcdata->will`, and
  `P_AIR..P_NEGATIVE` are the trainable primary-rune strengths (`ch->pcdata->powers[P_*]`, 0–100).
- **WAIT_STATE(ch, n)** is lag in pulses (`PULSE_VIOLENCE`), the standard combat-round unit.
- **`dec_duration(ch, gsn, n)`** drains `n` off a ward's `duration`. For Patryn wards `duration` is an
  **absorption pool** (set to `mind*15`), *not* a tick timer — it is strength, spent per blocked hit.
- **⚠️ Needs verification** items are collected at the bottom — don't trust those numbers blindly.

## Class basics

Source: `src/core/const.c` `class_table[CLASS_PATRYN]`, `src/include/merc.h` (struct `class_type`).

| Field | Value | Notes |
|---|---|---|
| `who_name` | `Patryn` | |
| `attr_prime` | `APPLY_DEX` | Prime attribute = **Dexterity** |
| First weapon | `OBJ_VNUM_SCHOOL_DAGGER` | Newbie weapon |
| Guild room | `3028` | |
| Max trainable Body | `80` | |
| Max trainable Mind | `100` | Highest Mind cap in the game; scales nearly all runeweave damage/duration |
| Max trainable Spirit | `75` | |
| Max trainable Will | `85` | `runecast` level = `will` (drives the rune-delegated spells) |
| HP gained per level | `13` | |
| Gains mana | `FALSE` | The `fMana` flag is FALSE, yet runeweaves cost mana — see ⚠️ Needs verification |

**Power slots** (`src/include/merc.h`, indices into `ch->pcdata->powers[]`): `P_LEARNED` (bitfield of known
runes), `P_LEARNED_NUM` (count of runes learned, drives learn cost), `P_AIR`/`P_EARTH`/`P_FIRE`/
`P_WATER`/`P_ENERGY`/`P_NEGATIVE` (primary-rune **strengths**, 0–100), and `P_BITS` (flag field;
the only bit is `P_DEFENSES = 1`).

---

## 1. The rune system

### Rune types (`src/include/merc.h`)

| Rune | Bit value | Kind | Trainable strength? | Tattooable? |
|---|---|---|---|---|
| `RUNE_AIR` | `1` | primary | yes (`P_AIR`) | yes |
| `RUNE_EARTH` | `2` | primary | yes (`P_EARTH`) | yes |
| `RUNE_FIRE` | `4` | primary | yes (`P_FIRE`) | yes |
| `RUNE_WATER` | `8` | primary | yes (`P_WATER`) | yes |
| `RUNE_ENERGY` | `16` | primary | yes (`P_ENERGY`) | no |
| `RUNE_NEGATIVE` | `32` | primary | yes (`P_NEGATIVE`) | no |
| `RUNE_LIFE` | `64` | secondary | no | no |
| `RUNE_DEATH` | `128` | secondary | no | no |
| `RUNE_CREATION` | `256` | secondary | no | no |
| `RUNE_DESTRUCTION` | `512` | secondary | no | no |
| `RUNE_PROTECTION` | `1024` | secondary | no | no |
| `RUNE_TRANSFORMATION` | `2048` | secondary | no | no |
| `RUNE_MOVEMENT` | `4096` | secondary | no | no |
| `RUNE_ABJURATION` | `16384` | secondary | no | no |
| `RUNE_ALL` | `8192` | sentinel | — | used by `get_runes` to count any rune |
| `RUNE_NONE` | `0` | empty slot | — | — |

Knowing a rune = `IS_SET(ch->pcdata->powers[P_LEARNED], RUNE_*)`. A runeweave requires **both** its
runes to be known (`src/classes/patryn.c:do_runeweave`).

### Body parts and rune capacity (`src/classes/patryn.c`, `src/include/merc.h`)

Tattoos live in `ch->pcdata->runes[part][slot]`. Capacity per part is `runemax[5] = { 15, 5, 5, 8, 8 }`:

| Part | Constant | Index | Max runes (`runemax`) |
|---|---|---|---|
| Torso | `TORSO` | 0 | 15 |
| Left arm | `LEFTARM` | 1 | 5 |
| Right arm | `RIGHTARM` | 2 | 5 |
| Left leg | `LEFTLEG` | 3 | 8 |
| Right leg | `RIGHTLEG` | 4 | 8 |

- **`get_runes(ch, type, part)`** (`src/classes/patryn.c`) counts runes of `type` on `part` (or any rune if
  `type == RUNE_ALL`). Result is clamped `URANGE(0, num, 15)`. Returns 0 for non-Patryn/NPC or
  `part > 4 || part < 0`.
- **`add_rune` / `remove_rune`** (`src/classes/patryn.c`) place/clear a rune in the first matching slot; no-op
  if the part is already full (`add_rune`) or the rune is absent (`remove_rune`).

**Which body runes matter where** (all in `src/core/fight.c`, see §4):

| Rune + location | Drives |
|---|---|
| Fire on **arms** | melee damage bonus |
| Earth on **arms** | parry chance |
| Air on **arms** | extra attacks per round |
| Water on **legs** | dodge chance |
| Earth on **legs** | suppresses enemy parry / dodge / block |
| Earth/Fire/Air/Water on **torso** | damage-type mitigation (see §4) |
| Air on **torso** | anti-scry vs Astral Detect / chant vision (§4) |
| Fire on **torso** | anti-scry vs Saiyan Ki Sense (§4) |

### `P_DEFENSES` — the defensive stance

Toggled by `defenses raise|lower` (`src/classes/patryn.c:do_defenses`; stored as bit `P_DEFENSES` in
`powers[P_BITS]`). While raised, it changes five things across the code:

| Effect | Magnitude | Source |
|---|---|---|
| Offensive runeweave mana cost | **+15%**, round up: `(cost*115 + 99)/100` | `src/classes/patryn.c:do_runeweave` |
| Patryn melee damage dealt | **−10%**: `dam -= dam/10` (after fire-arm boost) | `src/core/fight.c` (melee block) |
| Torso damage mitigation | **+10%**: mitigation `* 11/10` | `src/core/fight.c` (victim Patryn block) |
| Flat ward absorb | **+10%**: `wardbonus = 11` instead of `10` | `src/core/fight.c` (ward switch) |
| Anti-scry (Ki Sense / Astral Detect / locate) | **enables** it (gated on `P_DEFENSES`) | `src/classes/saiyan.c`, `src/classes/sorcerer.c` |

`defenses raise` applies `WAIT_STATE(ch, 4)`; `defenses lower` applies `WAIT_STATE(ch, 2)`.

---

## 2. Runeweave combinations

*Source: `src/classes/patryn.c:do_runeweave` (the `switch(spell)`), cost/target/wait from `cost_table[]`.*

Syntax: `runeweave <primary> <secondary> [target]`. `runeweave list` prints the table below.
`spell = rune1 + rune2`. Targeting/mana/lag come from the matching `cost_table[]` row; with no match
the weave prints "There is no such spell..yet." Common gates: caster must be Patryn `level >= 2`;
the **victim must be `level >= 2`** ("No runeweaving on mortals"); offensive weaves auto-engage
combat. `MAX_RUNESPELLS = 32`, and all 32 `cost_table` rows have a `switch` handler.

| Runes | Case | Target | Mana | Wait | Effect (summary) |
|---|---|---|---|---|---|
| air + life | `65` | offensive | 300 | 9 | Wind **curse** (pale blue mist) |
| earth + life | `66` | offensive | 300 | 9 | Earth **curse** (dull yellow mist) |
| fire + life | `68` | offensive | 300 | 9 | Flame **curse** (blood red mist) |
| water + life | `72` | offensive | 300 | 9 | Water **curse** (murky mist) |
| energy + life | `80` | defensive | 750 | 10 | Repeated `heal` (one per 33 `P_ENERGY`) |
| negative + life | `96` | offensive | 400 | 10 | HP drain (`DAM_NEGATIVE`), heals self; absorbed by Negative Block |
| air + death | `129` | offensive | 500 | 10 | Lightning bolt (`DAM_LIGHTNING`); rebounds off Air Block |
| fire + death | `132` | offensive | 500 | 10 | Flame lance (`DAM_FIRE`); rebounds off Fire Block; melts Van Rehl |
| air + creation | `257` | defensive | 750 | 10 | Grants target `NEW_BALL_LIGHTNING` (PC only) |
| water + creation | `264` | defensive | 250 | 10 | Douses `flame breath` affect |
| energy + creation | `272` | defensive | 100 | 10 | Graded cures (by `P_ENERGY`) |
| air + destruction | `513` | offensive | 750 | 9 | Gust of wind — cross-class disruption suite |
| fire + destruction | `516` | room | 400 | 10 | Room-wide fire AoE (`DAM_FIRE`) |
| energy + destruction | `528` | offensive | 600 | 10 | `dispel magic` + engages target |
| negative + destruction | `544` | offensive | 500 | 10 | Pearlescent light — scatters Mazoku charge |
| air + protection | `1025` | self | 200 | 10 | `shield` |
| earth + protection | `1026` | self | 250 | 10 | `stone skin` |
| fire + protection | `1028` | self | 750 | 8 | **Balus Wall** (`gsn_balus_wall`, dur 30) |
| water + protection | `1032` | self | 100 | 10 | `armor` |
| negative + protection | `1056` | defensive | 250 | 10 | **Negative Block** (`NEW_NEGATIVE_BLOCK`, PC only) |
| air + transformation | `2049` | defensive | 350 | 10 | **Air Block** (`NEW_AIR_BLOCK`, strips Fire Block; PC only) |
| fire + transformation | `2052` | defensive | 350 | 10 | **Fire Block** (`NEW_FIRE_BLOCK`, strips Air Block; PC only) |
| energy + transformation | `2064` | self | 400 | 10 | Toggle `PLR_TRUESIGHT` |
| negative + transformation | `2080` | offensive | 100 | 10 | Mana drain; absorbed by Negative Block |
| air + movement | `4097` | defensive | 100 | 10 | `fly` (always on self) |
| negative + movement | `4128` | offensive | 400 | 8 | Black haze — saps `move`; drains Saiyan str/speed/aegis |
| air + abjuration | `16385` | self | 750 | 8 | **Wind ward** (`gsn_wind_ward`) |
| earth + abjuration | `16386` | self | 750 | 8 | **Earth ward** (`gsn_earth_ward`) |
| fire + abjuration | `16388` | self | 750 | 8 | **Flame ward** (`gsn_flame_ward`) |
| water + abjuration | `16392` | self | 750 | 8 | **Water ward** (`gsn_water_ward`) |
| energy + abjuration | `16400` | self | 750 | 8 | **Spirit ward** (`gsn_spirit_ward`) |
| negative + abjuration | `16416` | self | 750 | 8 | **Negative ward** (`gsn_negative_ward`) |

### 2.1 Curses — `life` family (cases 65 / 66 / 68 / 72)

All four are mutually exclusive: applying one `affect_strip`s the other three (only one elemental
curse at a time). Each:

- Refuses if the victim already has that curse ("They are already so cursed.").
- **Holy Bless ward (#23):** `holy_bless_wards(...)` gives a white-spec target a 5% chance to shrug it off.
- **Duration:** `af.duration = !IS_NPC(victim) ? 3 : ch->pcdata->mind * 2/3` — capped at **3 ticks vs
  players**, Mind-scaled vs NPCs. `location = 0, modifier = 0` (the curse's combat effect lives in the
  affected target's class code, not here).

`gsn_wind_curse / gsn_earth_curse / gsn_flame_curse / gsn_water_curse`.

### 2.2 Direct damage — `death` family + negative/life (cases 96 / 129 / 132)

- **Negative + Life (`96`):** if the PC victim has `NEW_NEGATIVE_BLOCK`, the bolt is **absorbed** and
  *heals* them `victim->hit += dice(mind/8, P_NEGATIVE)` (capped at max; fails if self-cast). Otherwise
  `dam = dice(mind/4, P_NEGATIVE)`, `DAM_NEGATIVE`, and the caster heals by the HP actually drained
  (`ch->hit += (i - victim->hit)`, capped at max).
- **Air + Death (`129`):** if PC victim has `NEW_AIR_BLOCK`, **rebounds** onto the caster:
  `dam = dice(mind/4, P_AIR)`, `DAM_LIGHTNING` to self. Otherwise `dam = dice(mind/2, P_AIR)`,
  `DAM_LIGHTNING` to victim.
- **Fire + Death (`132`):** if PC victim has `NEW_FIRE_BLOCK`, **rebounds** onto the caster:
  `dam = dice(mind/4, P_AIR)` *(uses `P_AIR`, not `P_FIRE` — see ⚠️)*, `DAM_FIRE` to self. Otherwise
  `dam = dice(mind/2, P_FIRE)`, `DAM_FIRE`; also melts a Sorcerer's `NEW_VAN_REHL` icy barrier.

### 2.3 Healing / utility — `creation`/`life`/`movement` families (80, 257, 264, 272, 528, 2064, 4097)

- **Energy + Life (`80`):** loops `runecast(ch, victim, "heal")` once per 33 `P_ENERGY`
  (`for i = 0; i < P_ENERGY; i += 33`) — i.e. 1 heal at 0–32, up to ~4 heals near 100.
- **Energy + Creation (`272`):** graded by `P_ENERGY`: `<25` → `cure light`; `<50` → `cure serious`;
  `<75` → `cure critical` + `refresh`; else `heal` + `refresh` + `cure poison`.
- **Air + Creation (`257`):** PC-only; sets `NEW_BALL_LIGHTNING` on the target ("Nothing happens" on NPC).
- **Water + Creation (`264`):** strips a `flame breath` affect off the target if present.
- **Energy + Destruction (`528`):** `runecast(ch, victim, "dispel magic")`; if the victim wasn't
  fighting, sets `victim->fighting = ch`.
- **Energy + Transformation (`2064`):** toggles `PLR_TRUESIGHT` on the caster (no target).
- **Air + Movement (`4097`):** `runecast(ch, ch, "fly")` — always cast on self regardless of target.

All `runecast`-delegated spells run at level `ch->pcdata->will` and, for offensive delegated spells,
trigger a retaliatory `multi_hit` if the victim wasn't already fighting (`src/classes/patryn.c:runecast`).

### 2.4 Cross-class disruption (cases 513 / 544 / 2080 / 4128)

- **Air + Destruction (`513`) — gust of wind:** PC-target disruption (mirrors the Saiyan Ki Wave suite):
  - **vs Fist:** clears `NEW_FIGUREEIGHT`; decrements `F_KI` (if `> 0`).
  - **vs Sorcerer (only if `chant != NULL`):** rolls a resist-reduced interrupt: `chance = 75`; if
    `gsn_defense` is up it is torn away (stripped) and counts as `-10`; `-10` each for
    `AFF_HOLY_RESIST`, `AFF_VAS_GLUUDO`, and (`AFF_RAYWING` **or** `AFF_WINDY_SHIELD`); clamped
    `URANGE(50, chance, 85)`; on `number_percent() <= chance` → `lose_chant(victim)`. Defense no
    longer holds the chant outright — avoidance is capped at **50%** (#122).
  - **vs Saiyan:** strips `gsn_kiwall` if present.
- **Negative + Destruction (`544`) — pearlescent light:** only affects Mazoku; if `M_CTIME > 0`,
  resets `M_CTIME = 0` and `M_CTYPE = 0` (scatters the charge).
- **Negative + Transformation (`2080`) — mana drain:** if PC victim has `NEW_NEGATIVE_BLOCK`, it is
  **absorbed** and gives them `victim->mana += dice(mind/4, P_NEGATIVE)` (fails if self). Otherwise
  `dam = dice(mind/4, P_NEGATIVE)`, reduced by `AFF_HOLY_RESIST` (`dam -= dam * number_percent()/100`),
  drains the victim's mana (floor 1), and the caster regains `dam/2` mana.
- **Negative + Movement (`4128`) — black haze:** `dam = dice(10, 15)`; `victim->move -= dam` (floor 1),
  `ch->move += dam` (cap max). **vs Saiyan**, also drains `S_STRENGTH`, `S_SPEED`, `S_AEGIS` by `dam`
  each (floored at half their `_MAX`).

### 2.5 Self-protection — `protection` family (cases 1025 / 1026 / 1028 / 1032 / 1056)

- **Air + Protection (`1025`):** `runecast` `shield`.
- **Earth + Protection (`1026`):** `runecast` `stone skin`.
- **Fire + Protection (`1028`) — Balus Wall:** strips any existing `gsn_balus_wall`, then applies a
  fresh one: `af.type = gsn_balus_wall, duration = 30, location = APPLY_NONE`. In `fight.c` it **fully
  absorbs** the first `DAM_KIFLAME` melee hit and the first `SCHOOL_FIRE` chant (`dam = 0`, then the
  wall is stripped) — see §4.
- **Water + Protection (`1032`):** `runecast` `armor`.
- **Negative + Protection (`1056`):** PC-only ("All your base are belong to us." on NPC); sets
  `NEW_NEGATIVE_BLOCK`.

### 2.6 Blocks — `transformation` family (cases 2049 / 2052 / 1056)

The three "blocks" are mutually antagonistic flags in `actnew`, priority **Air > Fire > Negative**
(`src/classes/patryn.c:patryn_active_block`):

| Block | Flag | Bit | Set by | Strips |
|---|---|---|---|---|
| Air Block | `NEW_AIR_BLOCK` | `1048576` | case `2049` | removes `NEW_FIRE_BLOCK` first |
| Fire Block | `NEW_FIRE_BLOCK` | `524288` | case `2052` | removes `NEW_AIR_BLOCK` first |
| Negative Block | `NEW_NEGATIVE_BLOCK` | `32768` | case `1056` | (independent) |

Blocks **rebound/absorb** the matching runeweave damage (see §2.2/§2.4) and count as a "major setup
state" that a Saiyan Ryuken/Ki Wave will strip (priority Air > Fire > Negative, then ward drain).

### 2.7 Wards — `abjuration` family (cases 16385 / 16386 / 16388 / 16392 / 16400 / 16416)

Each ward is mutually exclusive: applying one `affect_strip`s **all six** wards first, then applies
the new one with `af.duration = ch->pcdata->mind * 15` (the absorption pool). Priority for
"the active ward" is **Spirit > Earth > Flame > Wind > Water > Negative**
(`src/classes/patryn.c:patryn_active_ward`).

| Weave | Ward gsn | Primarily blunts (see §4) |
|---|---|---|
| air + abjuration (`16385`) | `gsn_wind_ward` | `DAM_SHOCKWAVE` (−500) / `SCHOOL_WIND` |
| earth + abjuration (`16386`) | `gsn_earth_ward` | melee / `DAM_EMERALD` / `SCHOOL_EARTH` |
| fire + abjuration (`16388`) | `gsn_flame_ward` | `DAM_KIFLAME`/`DAM_CRIMSON` (−400) / `SCHOOL_FIRE` |
| water + abjuration (`16392`) | `gsn_water_ward` | `DAM_CERULEAN` (−400) / `SCHOOL_WATER` |
| energy + abjuration (`16400`) | `gsn_spirit_ward` | −15% to chant/astral schools; anti-scry |
| negative + abjuration (`16416`) | `gsn_negative_ward` | `F_DEATHTOUCH`/`DAM_OBSIDIAN` (−200) |

> ⚠️ **Code note:** the **wind ward (`16385`)** applies to `ch`; the other five abjuration cases call
> `affect_to_char(victim, &af)`. Because these are `TAR_CHAR_SELF`, `victim == ch`, so the result is
> the same — but the inconsistency is real (see ⚠️ Needs verification).

---

## 3. Commands

*All Patryn commands are `level 2`, Patryn-only (`src/core/interp.c`).*

| Command | Function | Position | Purpose |
|---|---|---|---|
| `runeweave <a> <b> [tgt]` | `do_runeweave` | `POS_FIGHTING` | Cast a runeweave combo (§2). `runeweave list` prints the table. |
| `runes` | `do_runes` | `POS_DEAD` | List known/unknown runes, primary strengths, and learn/train costs. |
| `runestats` | `do_runestats` | `POS_RESTING` | Show body-rune layout + defensive summary (see below). |
| `runetrain [rune]` | `do_runetrain` | `POS_STANDING` | Raise a primary rune's strength with exp. |
| `learn <rune>` | `do_learn` | `POS_STANDING` | Learn a new rune with primal. |
| `tattoo <part> <rune>` | `do_tattoo` | `POS_STANDING` | Tattoo air/earth/fire/water onto a body part. |
| `erase <part> <rune>` | `do_erase` | `POS_STANDING` | Remove a body rune. |
| `defenses [raise\|lower]` | `do_defenses` | `POS_RESTING` | Toggle the `P_DEFENSES` stance (§1). |
| `circle` | `do_circle` | `POS_RESTING` | Set `NEW_CIRCLE` (must be resting; multi-person circling unimplemented). |

### `learn` — `do_learn`

- **Cost:** `UMIN(200, 50 + P_LEARNED_NUM * 10)` **primal** (caps at 200).
- Fails if already known or insufficient primal; on success: `primal -= cost`, set rune bit in
  `P_LEARNED`, `P_LEARNED_NUM++`. Learnable: all six primaries + eight secondaries.

### `runetrain` — `do_runetrain`

- **Cost:** `150 * (strength+1) * (strength+1)` **exp** for the current `P_*` strength.
- Caps at strength `100` ("you already understand all the uses"). On success `exp -= cost`, `P_*++`.
- Only the six primaries have trainable strength; secondaries do not.

### `tattoo` — `do_tattoo`

- **Runes allowed:** earth/air/fire/water only (must be known).
- **Parts:** torso/leftarm/rightarm/leftleg/rightleg.
- **Primal cost:** `10 + get_runes(ch, RUNE_ALL, location) * 10`. **Mana cost:** `cost * 25`.
- Fails if the part is full (`get_runes(...) >= runemax[location]`). On success `add_rune`, deduct
  primal + mana, `WAIT_STATE(ch, 12)`.

### `erase` — `do_erase`

- **Cost:** flat **25 primal + 1000 mana**. Fails if the rune isn't on that part. Calls `remove_rune`.

### `runes` — `do_runes`

Prints, for each **primary**, known/unknown, strength `n/100`, and either `(mastered)`, the train cost
(`150*(strength+1)^2` exp), or the learn cost; and for each **secondary**, known/unknown + learn cost.

### `runestats` — `do_runestats`

Prints the full body-rune grid (torso 15, arms 5, legs 8 each) and a **defensive summary** (#7):
`P_DEFENSES` state; torso runes (Earth/Fire/Air/Water — "earth+fire drive damage mitigation"); arm
runes (Fire = melee damage, Earth = parry); leg runes (Earth = reduces enemy parry/dodge/block); any
active block; and active wards with remaining `duration` (printed as "strength ~N" via
`runestats_ward`, since ward duration is an absorption pool, not ticks).

### `defenses` — `do_defenses`

See §1 (`P_DEFENSES`). No arg → report state; `raise`/`lower` → toggle, with `WAIT_STATE` 4 / 2.

---

## 4. Passive ward/block mitigation & rune combat math (always-on, `src/core/fight.c`)

These are not commands but govern most of the class's power. Line numbers are at commit `e5ac280`.

### 4.1 Melee damage dealt (Patryn attacker) — `fight.c:461`

```
dam += dam * (50 + P_FIRE/2) * (fire runes LEFTARM + fire runes RIGHTARM) / 100;
if ( P_DEFENSES ) dam -= dam / 10;   /* defensive stance: -10% */
```

Fire arm-runes scale melee damage; with `P_DEFENSES` raised, melee output drops 10%.

### 4.2 Torso damage mitigation (Patryn victim) — `fight.c:839`

A per-damage-type `mod` is computed from **torso** runes, then mitigation is applied. `mod` by `dt`:

| Damage type(s) | `mod` |
|---|---|
| `DAM_KIFLAME`, `DAM_CRIMSON` | fire>0 → `earth + fire*3/2`; else `UMAX(0, earth/2)` |
| `DAM_LIGHTNING`, `F_DEATHTOUCH` | `earth`; if air≤0 → `mod /= 2` |
| `DAM_FIRE` | `earth`; if fire≤0 → `mod /= 2` |
| `DAM_CERULEAN` | `earth`; if water>0 → `mod += water*3/2`; else `mod /= 2` |
| (any other) | `earth` |

Mitigation applied (`earth`/`fire`/`air`/`water` = torso counts):

```
/* defenses raised: */
dam -= ( ( dam * mod * (4 + P_EARTH/20) ) / ( mod*10 + (35 - mod*2) + dice(1,20) ) ) * 11/10;
/* defenses lowered: */
dam -= ( dam * mod * (4 + P_EARTH/20) ) / ( mod*10 + (35 - mod*2) + dice(1,20) );
```

### 4.3 Flat ward absorb (Patryn victim) — `fight.c:880`

`wardbonus = P_DEFENSES ? 11 : 10` (so absorb is `base * wardbonus/10`, i.e. +10% with defenses up).
Switch on `dt`; if the matching ward is active, subtract a flat amount and drain the ward by the
**base** amount:

| Damage type | Ward | `dam -=` | `dec_duration` |
|---|---|---|---|
| `DAM_SHOCKWAVE` | `gsn_wind_ward` | `500 * wardbonus/10` | 500 |
| `DAM_EMERALD` | `gsn_earth_ward` | `200 * wardbonus/10` | 200 |
| `F_SHINKICK`/`F_JAB`/`F_SPINKICK`/`F_KNEE`/`F_ELBOW`/`F_UPPERCUT` | `gsn_earth_ward` | `15 * wardbonus/10` | 60 |
| melee types `1000`–`1012` | `gsn_earth_ward` | `15 * wardbonus/10` | 15 |
| `DAM_KIFLAME`/`DAM_CRIMSON` | `gsn_flame_ward` | `400 * wardbonus/10` | 400 |
| `DAM_CERULEAN` | `gsn_water_ward` | `400 * wardbonus/10` | 400 |
| `F_DEATHTOUCH`/`DAM_OBSIDIAN` | `gsn_negative_ward` | `200 * wardbonus/10` | 200 |

> Note: the ward `duration` (absorption pool) is drained by the **base** value while the damage
> reduction is the **boosted** value — so defenses-up makes each block ~10% stronger without spending
> the ward any faster.

### 4.4 Balus Wall (melee + fire chant) — `fight.c:750` and `fight.c:1554`

- **Melee `DAM_KIFLAME` (`fight.c:750`):** for a non-Saiyan, non-NPC victim with `gsn_balus_wall`,
  `dam = 0`, message "absorbs … and vanishes", then `affect_strip(gsn_balus_wall)`.
- **Sorcerer chant, `SCHOOL_FIRE` (`fight.c:1554`):** same full absorb (`dam = 0`) + strip. (This
  branch is inside the Sorcerer-victim block but is reached for the Patryn's own wall via the shared
  affect.)

### 4.5 Spell/chant mitigation (Patryn victim) — `fight.c:1470`

```
mod = (fire torso > 0) ? earth torso : earth torso / 2;
dam -= (dam * mod * P_AIR) / ((mod+4) * 100 + (rank*5));
```

Then per chant `school`, an active ward subtracts a flat 100 (floored at 1) and drains 100:

| `school` | Ward effect |
|---|---|
| `SCHOOL_WIND` | wind_ward: `dam = UMAX(dam-100,1)`, drain 100 *(was water_ward — fixed #48)*. **Then** if water torso>0: `dam += dam*mod/6` (`gsn_water_ward` up) or `dam*mod/3` (no water ward) — water torso is a *weakness* vs wind, and this second block still keys off the *water* ward by design |
| `SCHOOL_EARTH` | earth_ward: `dam = UMAX(dam-100,1)`, drain 100 |
| `SCHOOL_FIRE` | flame_ward: `dam = UMAX(dam-100,1)`, drain 100 |
| `SCHOOL_WATER` | water_ward: `dam = UMAX(dam-100,1)`, drain 100 |
| `case 2` (negative) | negative_ward: `dam = UMAX(dam-100,1)`, drain 100 |

**Spirit ward (`fight.c:1586`):** if `gsn_spirit_ward` and `school ∈ {BLACK, EARTH, WIND, FIRE, WATER,
ASTRAL, WHITE}`: `dam -= dam * 15/100`, drain 25. (Also: spirit ward gives 15% chance to resist a
hostile curse and to fizzle a dispel — `src/classes/sorcerer.c`, drain 25 each.)

### 4.6 Parry / dodge / enemy-evasion suppression

- **Parry (Patryn victim) — `fight.c:1852`:** requires a wielded weapon (or the class exceptions);
  `chance += (earth LEFTARM + earth RIGHTARM) * 12; chance += P_EARTH/2`. (#23: earth arms grant
  parry, not damage.) `chance` is `URANGE(20, …, 80)`.
- **Patryn attacker suppresses victim parry — `fight.c:1896`:**
  `chance -= (earth LEFTLEG + earth RIGHTLEG) * UMIN(13, 21 - runes)`.
- **Dodge (Patryn victim) — `fight.c:1970`:** `runes = water LEFTLEG + water RIGHTLEG`; if `runes>=8`
  → `chance += 90` else `chance += runes*12`; then `chance += UMAX(runes-8, 1) * 5`. `URANGE(20,…,80)`.
- **Patryn attacker suppresses victim dodge — `fight.c:2002`:** `runes = earth LEFTLEG + earth
  RIGHTLEG`; if `runes<=9` → `chance -= runes*9` else `chance -= runes*7`.
- **Patryn attacker suppresses victim block (Fist block defense) — `fight.c:2087`:**
  `chance -= 8 * (earth LEFTLEG + earth RIGHTLEG)`.

### 4.7 Extra attacks — `fight.c:3607`

```
atk += get_runes(ch, RUNE_AIR, LEFTARM) + get_runes(ch, RUNE_AIR, RIGHTARM);
// the leg-air half-bonus is commented out
```

Air arm-runes add one attack each per round.

### 4.8 Anti-scry (gated on `P_DEFENSES`, victim `level >= 2`)

- **vs Saiyan Ki Sense — `src/classes/saiyan.c:do_kisense`:** keyed on **fire torso** runes (`fcount`):
  `fcount >= 5` **or** `gsn_spirit_ward` → fully shrouded; `fcount >= 3` → location clouded; `1–2` →
  warning glow only (sense still succeeds).
- **vs Astral Detect / chant vision / locate — `src/classes/sorcerer.c`:** keyed on **air torso** runes
  (`acount`): `acount >= 5` **or** `gsn_spirit_ward` → fully blocked; `acount >= 3` → vision dissolves
  (no room snapshot); `1–2` → warning glow only.

---

## ⚠️ Needs verification

- **`fMana = FALSE` vs mana cost.** `class_table[CLASS_PATRYN]` sets `fMana = FALSE` (no mana gained on
  level), yet every runeweave spends mana and `tattoo`/`erase` cost large mana sums. Confirm how a
  Patryn accrues mana (item/regen path) — the class clearly relies on a mana pool the level-up flag says
  it doesn't grow.
- **Fire + Death (`132`) rebound uses `P_AIR`.** The Fire-Block rebound rolls
  `dice(ch->pcdata->mind/4, ch->pcdata->powers[P_AIR])` — almost certainly a copy-paste from the
  Air+Death case (`129`); intended value is presumably `P_FIRE`. Flagged as a likely bug, not changed.
- **Abjuration ward target inconsistency.** Wind ward (`16385`) applies to `ch`; the other five
  abjuration cases call `affect_to_char(victim, …)`. Harmless today because all are `TAR_CHAR_SELF`
  (`victim == ch`), but confirm none can ever be cast with `victim != ch`.
- **`rank` in §4.2 / §4.5.** The torso-mitigation and spell-mitigation denominators use a `rank`
  variable computed earlier in `damage()`; its exact derivation (level/align-based?) was not traced
  here, so absolute mitigation magnitudes depend on it.
- **`mod`/`dice(1,20)` variance in §4.2.** Torso mitigation has a `+ dice(1,20)` term in the
  denominator, so per-hit reduction swings; the table gives the formula, not a fixed percentage.
- **~~`SCHOOL_WATER` and `SCHOOL_WIND` share `gsn_water_ward` (§4.5).~~** *(Resolved, #48.)* The
  `SCHOOL_WIND` mitigation case was a copy-paste of the water case; it now keys off `gsn_wind_ward`,
  so each ward blunts its own school. (The *second* block under `SCHOOL_WIND` — water-torso runes
  amplifying wind damage, halved when a *water* ward is up — intentionally still references
  `gsn_water_ward` and was left as-is; flagged to reexamine if wind/water balance is revisited.)
- **`case 2` school constant (§4.5).** The negative-school branch is written as bare `case 2:` with a
  comment, not a `SCHOOL_*` macro — verify `2` is indeed the negative/black school id.
