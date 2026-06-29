# Sorcerer — Ability Reference (Balance / Mechanics)

> **Audience:** balance review and maintainers. This is the exhaustive, numbers-first reference
> for **every** Sorcerer chant and command, extracted directly from the server source. For the new-player
> narrative version, see **[Sorcerer — Player Guide](CLASS-SORCERER.md)** (do not balance-check against
> that doc; this one is authoritative).

**Last verified against source:** commit `e5ac280`
**Primary sources:** `src/sorcerer.c`, `src/fight.c` (`chant_damage`, school amps), `src/update.c`
(`chant_update`, Mystic regen, Laguna Blade upkeep, Dragon Slave tick, Flame Breath DoT), `src/const.c`,
`src/merc.h`

## How to read this

- **Each ability is cited `file:function`** so every number can be checked against the C. If a number
  here disagrees with the code, the **code wins** — fix this doc.
- **Resource:** Sorcerers spend **Mystic** (`powers[SORC_MYSTIC]`), an ammo pool that caps at the
  caster's **Will** (max 100). The Mystic spent by a chant is its **`rank` field** (which doubles as
  "required rank"), *not* its `cost` field — see *Chant system* below. The legacy `mana` cost is
  commented out (`do_chant`), except for **prepared** auto-casts (which spend `cost/2` mana).
- **`rank` (in formulas)** is the caster's **effective school rank** (`sorc_rank`): the raw researched
  rank, **0–50** in your specialty, **capped at 44** off-spec (`SORC_OFFSPEC_CAP`). Every `chant_*`
  function receives this as its `rank` argument. Where a formula instead reads `powers[SCHOOL_*]`
  directly (raw, uncapped), it is called out.
- **Formulas are quoted verbatim.** `dice(n, size)` rolls `n` dice of `1..size` (mean ≈ `n*(size+1)/2`).
  `number_percent()` is `1..100`. `will` = `ch->pcdata->will`.
- **All offensive damage routes through `chant_damage()` (`src/fight.c`)**, which applies school amps,
  per-class soaks, the "scorched" debuff, and the `dam > 30000` clamp. See *Combat math* at the bottom.
- **Saves** use `saves_chant()` (`src/sorcerer.c`); the shared formula is in *Chant system*. "save→/2"
  means a successful save halves damage; "save→dodge" means a save negates it entirely. Each chant's
  save behavior is given per entry.
- **Lag / wait:** `lag` is the `WAIT_STATE` after the `chant` command (`do_chant`). `wait` is the
  per-line tick spacing of a complex (multi-line) chant (`chant_update`). `lines` = 1 is an instant
  chant (resolves the same pulse path); `lines` > 1 is an interruptible complex chant (resolves over
  ticks). All table values are quoted from `chant_table` (`src/sorcerer.c`).
- **⚠️ Needs verification** items are collected at the bottom — don't trust those numbers blindly.
  Given the size of this class, that list is intentionally long; flagged items are ones not fully
  traced, not necessarily bugs.

## Class basics

Source: `src/const.c` `class_table[CLASS_SORCERER]`, `src/merc.h` (struct `class_type`).

| Field | Value | Notes |
|---|---|---|
| `who_name` | `Sorcerer` | |
| `attr_prime` | `APPLY_INT` | Prime attribute = **Intelligence** |
| First weapon | `OBJ_VNUM_SCHOOL_DAGGER` | Newbie weapon |
| Guild room | `3018` | |
| Max trainable Body | `75` | |
| Max trainable Mind | `90` | |
| Max trainable Spirit | `80` | |
| Max trainable Will | `100` | Highest Will in the game; caps the Mystic pool + drives saves/regen |
| HP gained per level | `5` | Lowest in the game |
| Gains mana | `TRUE` | Only mana class; mana fuels prep auto-casts + Concentrate |

---

## Chant system (mechanics every chant shares)

Sources: `src/sorcerer.c` (`do_chant`, `do_research`, `do_specialize`, `do_prepare`, `do_concentrate`,
`sorc_rank`, `saves_chant`, `chant_cast`, `lose_chant`); `src/update.c` (`chant_update`, Mystic regen);
`src/merc.h` (`SORC_*`, `SCHOOL_*`, `MAX_CHANT`).

### Schools & specialization
- **Seven schools** (`merc.h`): `SCHOOL_BLACK`(2), `SCHOOL_EARTH`(3), `SCHOOL_WIND`(4), `SCHOOL_FIRE`(5),
  `SCHOOL_WATER`(6), `SCHOOL_ASTRAL`(7), `SCHOOL_WHITE`(8). Indices double as `powers[]` slots holding
  the researched rank in each school.
- **`research <school>`** (`do_research`): buys the next rank with **primal**. In-spec cost
  `UMIN(75, rank*5)`; off-spec cost `UMIN(75, rank*5) + rank`. In-spec cap **50**; off-spec cap **44**
  (`SORC_OFFSPEC_CAP`). `research list` prints ranks/caps/next cost.
- **`specialize <black magic|white magic|shamanism>`** (`do_specialize`): first spec is free; respec
  costs **250 primal**. `SORC_SPEC` holds `SCHOOL_BLACK`, `SCHOOL_WHITE`, or `SCHOOL_ASTRAL`.
  **Shamanism** (`SCHOOL_ASTRAL`) counts as in-spec for **all four elemental schools** (earth, wind,
  fire, water) — `school > SCHOOL_BLACK && school < SCHOOL_WHITE`.
- **`sorc_rank(ch, school)`** (`do_chant` and every formula): returns the raw rank in-spec, else
  `UMIN(raw, 44)`. **Respec is non-destructive** — off-spec ranks above 44 go *inactive*, not deleted,
  and return at full if you spec back.

### Mystic (the resource), regen, Concentrate
- **Mystic pool caps at `will`** (≤100). To cast, `powers[SORC_MYSTIC] >= chant_table[cn].rank`;
  on cast, `SORC_MYSTIC -= rank` (floored at 1). The chant then resolves at full **school rank**, not
  throttled by remaining Mystic (the old death-spiral was removed). A **misfire** (target gone at
  resolution) refunds `rank/2` Mystic (`chant_cast`).
- **Passive regen** (`update.c`, per tick): `if SORC_MYSTIC < will → SORC_MYSTIC += 1 + will/20`
  (≈ +6/tick at will 100).
- **`concentrate`** (`do_concentrate`, toggle, `WAIT_STATE 4`): while `NEW_CONCENTRATE` and `mana>=100`,
  each tick `SORC_MYSTIC += dice(3,3)` and `mana -= 100`; auto-cancels at `mana<100`. Needs ≥100 mana
  to start.

### Instant vs complex chants (lines)
- **Instant chant (`lines == 1`):** queued by `do_chant`, resolves on the next `chant_update` tick;
  `cha->wait = chant_table[cn].wait`.
- **Complex chant (`lines > 1`):** speaks `line1` immediately, then `line2..lineN` over ticks
  (`chant_update` echoes a line every `wait` ticks); resolves when `cha->wait <= 0`. The queued
  `wait = chant_table[cn].wait * (lines-1)`.
- **Mutual exclusions (`do_chant`):**
  - `lines > 1 && pcdata->chant != NULL` → "You cannot maintain two complex chants at once."
  - `lines > 1 && NEW_LAGUNABLADE` → "You cannot weave complex magic while the blade burns in your
    hands." (Laguna Blade and any complex chant are mutually exclusive.)
- **Interruption:** a queued chant is dropped by `lose_chant()`. It is called when the Sorcerer is
  reduced to ≤ `POS_MORTAL`/death (`fight.c` × `chant_damage`), and by cross-class disruptors: Saiyan
  Ki Wave / Ryuken (`saiyan.c`), Fist (`fist.c`), Mazoku (`mazoku.c`), Patryn (`patryn.c`), another
  Sorcerer's **Diem Wing** / **Flow Break** (`sorcerer.c`), and admin `freeze`/`mppurge` (`act_wiz.c`).

### `saves_chant(ch, victim, cn)` — the universal save roll
*Source: `src/sorcerer.c:saves_chant`* — `chance` starts at **45**, then:
- **NPC victim:** `+level/3`; if `level >= 102`, `+level + 50`.
- **PC victim:** `+will/3`; **Saiyan w/ Ki Wall** `+spirit` (*except* against wall-breaking chants —
  Diem Wing / Flow Break / Bomb di Wind, via `chant_breaks_kiwall`); **Patryn** `+P_AIR/2`; **Fist**
  `+spirit/5`; **Sorcerer** `+sorc_rank(WHITE)` if `AFF_HOLY_RESIST`, `+10` if `AFF_VAS_GLUUDO`, and
  `+ powers[SPEC]/2` if the chant is in the victim's own specialty.
- **Then for the caster:** `-will/3` and `-sorc_rank(school)`; if the **caster is specialized** in the
  chant's school, `chance /= 2` (specialists land spells far more reliably).
- **Clamp:** low = 15 (PvE boss `level>=100`), 10 (specialist caster), or 12 (off-spec); high = 85
  (`NPC level>=102`) else 75. `return number_percent() < chance`.

### `prepare` — passive melee auto-cast
*Source: `src/sorcerer.c:do_prepare` + `src/fight.c:multi_hit`* — `prepare <school>` sets `SORC_PREP`
to a school (`prepare none` clears it, `WAIT_STATE 4`). While set, **every melee round** (`multi_hit`)
the highest-rank **prep-flagged** chant of that school you qualify for is auto-cast at the victim,
spending **`cost/2` mana** (not Mystic); if mana is short, the round is skipped. Only chants with the
table `prep = TRUE` flag are eligible (listed per school below). The chant's damage type is `1400+cn`.

> ⚠️ Both `do_prepare` and `multi_hit` iterate `for (i = MAX_CHANT; i > 0; i--)`, starting at index
> `MAX_CHANT` (81) — one past the last valid index (0–80). This is a latent off-by-one read into
> `chant_table[81]`. Flagged, not changed (upstream).

### `chant list` / `chant info <spell>`
`do_chant` provides `chant list` (chants you currently qualify for, with school/rank/lines) and
`chant info <spell>` (school, required rank = Mystic cost, target type, lines/wait/lag, your effective
rank). Names are matched space/hyphen-insensitively by prefix (`sorc_match_chant`).

---

## Black magic (`SCHOOL_BLACK`)

Specialty for raw single-target and AoE nuke damage; several chants self-heal or execute. Black-school
damage gets **+10%** vs a "scorched" victim (`chant_damage`). Prep-eligible: **Balus Rod, Dynast Brass,
Zelas Brid**.

| Chant | Rank/Mystic | Cost(mana base) | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| balus rod | 1 | 200 | 1 | 4 | 2 | offensive | ✓ |
| disfang | 4 | 300 | 1 | 8 | 2 | offensive | |
| dolph zork | 7 | 1000 | 1 | 8 | 8 | special/area | |
| dynast brass | 10 | 500 | 1 | 8 | 3 | offensive | ✓ |
| blast ash | 13 | 1000 | 1 | 12 | 4 | offensive | |
| ferrous bleed | 16 | 1500 | 1 | 8 | 4 | offensive | |
| hell blast | 19 | 1200 | 1 | 10 | 6 | offensive | |
| zelas brid | 22 | 800 | 1 | 4 | 2 | offensive | ✓ |
| dolph strash | 25 | 1000 | 1 | 6 | 2 | offensive | |
| rubyeye blade | 28 | 3000 | 3 | 8 | 10 | special/area | |
| dynast breath | 34 | 3000 | 2 | 8 | 5 | offensive | |
| gaav flare | 37 | 2000 | 1 | 9 | 3 | offensive | |
| laguna blast | 40 | 2000 | 2 | 8 | 10 | offensive | |
| flame breath | 45 | 1200 | 2 | 8 | 10 | offensive | |
| dragon slave | 50 | 3000 | 5 | 8 | 10 | offensive | |
| laguna blade | 50 | 6000 | 5 | 8 | 12 | special/area | |

### Balus Rod — `chant balus-rod` · `chant_balus_rod`
- **Damage:** `dice(10, 20 + rank)`; **save→/2**.

### Disfang — `chant_disfang`
- **Damage:** `dice(15, 20 + rank)`; **save→dodge** (full negate).

### Dolph Zork — `chant_dolph_zork` (TAR_IGNORE)
- **Creates** an ice-blade weapon (`DOLPH_ZORK`): `timer = rank + dice(1,4)`, `value[1]=10`,
  `value[2]=20`, given to caster. No direct damage.

### Dynast Brass — `chant_dynast_brass`
- **AoE** to the target and everyone in its group (or all NPCs if target is NPC): per victim
  `dice(15, 30 + rank)`; **save→/2** (save rolled once vs the primary `victim`, applied to all).

### Blast Ash — `chant_blast_ash` (execute)
- **Hit chance:** `chance = rank + will/2`; `+ (rank-40)*2` if `rank>40`. NPC: `chance=0` if
  `level>95`, else `-= level*2/3`. PC: `-= hit/150`. Miss if `number_percent() > chance`.
- **On hit (PvE, or PC at ≤25% HP):** `chant_damage(victim->hit + dice(2,5))` — full execute.
- **On hit (PC above 25% HP, #5 cap):** `dam = victim->hit/5 + rank*20`, then
  `dam = UMIN(dam, (victim->hit - 1) * 10 / 11)` — clamped non-lethal (the 10/11 leaves headroom for
  the +10% scorched amp). No `saves_chant`.

### Ferrous Bleed — `chant_ferrous_bleed`
- **save→nothing.** Else strips up to **2** affects, each dealing `dice(10, rank)`; if the target had
  **no** affects to strip, a single `dice(5, rank)` backlash so a landed cast always bites.

### Hell Blast — `chant_hell_blast` (drain/heal)
- **Hit chance:** miss if `number_percent() > (rank-10)*2`.
- **Damage:** `dice(rank, rank)`. **Lifesteal:** caster heals by the HP actually removed
  (`oldhit - victim->hit`), capped at `max_hit`, if the victim survives (`> POS_STUNNED`). No save.

### Zelas Brid — `chant_zelas_brid`
- **Damage:** `dice(rank*5/8, rank*3)`. **No save.** (Note: `placeholder` field = 6 here; unused.)

### Dolph Strash — `chant_dolph_strash`
- **Damage:** `dice(rank*1/2, rank*3)`. **No save.**

### Rubyeye Blade — `chant_ruby_eye_blade` (TAR_IGNORE)
- **Creates** weapon `RUBYEYE_BLADE`: `timer = (rank-25)*3 + dice(1,4)`, `value[1]=rank`,
  `value[2]=rank*2`. No direct damage.

### Dynast Breath — `chant_dynast_breath` (root/AC, #2/#34 rework)
- Fails if target already has the `dynast breath` affect.
- **AC penalty (`APPLY_AC`):** saved → PvP `UMIN(rank*2,100)` / PvE `rank*3`; **not** saved → PvP
  `UMIN(rank*4,200)` / PvE `rank*6`.
- **Duration:** saved → PvP `dice(2,10)` / PvE `dice(3,10)`; not saved → `dice(10,20)`.
- **Root:** `AFF_NO_FLEE` **only on a failed save**; a **Holy Bless** ward (5%) shrugs the root
  (AC still lands). No damage.

### Gaav Flare — `chant_gaav_flare` (#27)
- **Damage:** `dice(rank, rank*3)`; **save → `dam -= dam/4`** (−25%).
- **Balus Wall block:** if victim has `gsn_balus_wall`, `dam -= dam/2`, the wall is stripped, and the
  Scorched debuff is prevented.
- **Scorched (failed save, no wall):** applies `scorched` affect, `duration = dice(2,4)` (~2–8 ticks)
  → +10% Black damage taken (`chant_damage`).

### Laguna Blast — `chant_laguna_blast`
- **AoE** target's group / all NPCs: per victim `dice(rank, rank*2)`; **save→/2** (rolled vs primary).

### Flame Breath — `chant_flame_breath`
- **Damage:** `dice(rank, rank*4)`; **save→/2**. On a **failed** save also applies the `flame breath`
  **DoT** affect, `duration = dice(7,5)` → `dice(10,20)` `DAM_FLAME_BREATH` per tick (`regen_update`).

### Dragon Slave — `chant_dragon_slave`
- **AoE** all non-group in room: `dice(rank, rank*4)`; **save → `dam -= dam/3`**.
- **Sets `NEW_DRAGON_SLAVE`** → one follow-up AoE tick in `char_update`: `dice(SCHOOL_BLACK rank,
  SCHOOL_BLACK rank*3)` using the **raw** Black rank, then the bit is cleared.

### Laguna Blade — `chant_laguna_blade` (TAR_IGNORE, finisher)
- Unequips current weapon, equips conjured `LAGUNA_BLADE`: `timer=2`, `value[1]=UMAX(20,rank)`,
  `value[2]=40*UMAX(20,rank)`. Sets `NEW_LAGUNABLADE`.
- **Upkeep (`update.c`):** drains **`SORC_MYSTIC -= 8`/tick**; ends (blade shatters) when Mystic hits 0
  or the blade is gone (decayed/disarmed/removed). Blocks all complex chants while up. Counterplay:
  disarm, flee, Flow Break, Diem Wing.

---

## White magic (`SCHOOL_WHITE`)

The sustain / disruption / utility school: heals, buffs (Bless, Holy Resist, Vas Gluudo, Visfarank,
Mos Varim), Flow Break (anti-magic), sleep, and resurrection. White-spec confers the **Holy Bless**
ward (5% shrug, `HOLY_BLESS_WARD_PCT`) and amplifies several `chant_damage` resist terms. No prep-eligible
chants.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target |
|---|---|---|---|---|---|---|
| lighting | 1 | 350 | 1 | 8 | 3 | offensive |
| dicleary | 4 | 400 | 1 | 8 | 4 | defensive |
| holy bless | 7 | 750 | 2 | 8 | 2 | self |
| visfarank | 10 | 1500 | 1 | 12 | 4 | special/area |
| laphas seed | 13 | 1200 | 1 | 8 | 6 | offensive |
| recovery | 16 | 1000 | 1 | 8 | 3 | defensive |
| sleeping | 19 | 2500 | 1 | 12 | 8 | special/area |
| mos varim | 22 | 500 | 1 | 8 | 4 | self |
| flow break | 25 | 1500 | 1 | 8 | 8 | offensive |
| vas gluudo | 28 | 1500 | 1 | 8 | 4 | self |
| chaos strings | 31 | 1000 | 1 | 10 | 8 | offensive |
| holy resist | 34 | 800 | 2 | 8 | 8 | self |
| defense | 37 | 3000 | 1 | 10 | 8 | self |
| chaotic disintegrate | 45 | 1500 | 3 | 8 | 8 | offensive |
| ressurection | 50 | 2500 | 4 | 8 | 10 | defensive |

### Lighting — `chant_lighting` (blind, #6)
- No effect on `IS_SUIT` or `level>95`.
- **save → dazzled:** `blindness` affect `APPLY_HITROLL -3`, `duration 1` (if not already affected).
- **failed save:** **Holy Bless** ward (5%) may shrug; else `blindness` `APPLY_HITROLL -8` + `AFF_BLIND`,
  `duration = 2` (PC) or `1 + dice(5,20)` (NPC).

### Dicleary — `chant_dicleary` (defensive heal + cleanse, #6)
- **Heal:** `100 + dice(10, rank)` to both `hit` and `move`.
- **Cleanse ONE debuff** (highest priority first), gated by rank: `rank>=35` curse (wind/earth/flame/
  water), `rank>=45` an `AFF_NO_FLEE` root, `rank>=25` blindness, `rank>=15` poison.

### Holy Bless — `chant_holy_bless` (self buff)
- **Bless:** `APPLY_HITROLL` **and** `APPLY_DAMROLL` (two nodes), `modifier = UMAX(2, UMAX(rank/5,
  (rank-30)/2))`, `duration = 30*rank + dice(1,6)`. Re-cast strips the old first.
- **White-spec only:** the hitroll node carries `AFF_HOLY_BLESS` → 5% (`holy_bless_wards`) chance to
  shrug curse/blind/sleep/magical root.

### Visfarank — `chant_visfarank` (TAR_IGNORE; self barehand buff)
- Self `AFF_VISFARANK`, `duration = 30*rank + dice(1,6)`. **Effect (`fight.c` unarmed):** barehand
  damage `number_range((weapon/2)+i, (weapon*2)+40+i)`, `i = UMAX(0, (SCHOOL_WHITE-20)*2)`.

### Laphas Seed — `chant_laphas_seed` (bind)
- Fails if target already `AFF_NO_FLEE`. **save→dodge.** **Holy Bless** ward (5%) may shrug.
- Else `AFF_NO_FLEE`, `duration = dice(5,20)` (PC capped at **20**, `LAPHAS_PVP_DUR_CAP`), `APPLY_AC`
  `modifier = rank*4`; plus a second affect `APPLY_MOVE -50` (`LAPHAS_MOVE_DRAIN`).

### Recovery — `chant_recovery` (HoT, PC only)
- Fails if target already `NEW_RECOVERY`. Sets `NEW_RECOVERY` (heal-over-time in `char_update`).
- **Immediate heal:** `200 + dice(rank, 30)`. `rank>=30` also cleanses poison/blindness and lifts a
  light stun (POS_STUNNED→POS_RESTING). Disrupted by Flow Break.

### Sleeping — `chant_sleeping` (AoE, TAR_IGNORE)
- All non-group, non-self in room. **Resists if:** NPC saves or `level>75`; PC saves or `max_hit>10000`
  or `level<2`. **Holy Bless** ward (5%) may shrug. Else `AFF_SLEEP`, `position=POS_SLEEPING`,
  `duration = dice(3, rank)`.

### Mos Varim — `chant_mos_varim` (self, anti-fire)
- Self `gsn_mos_varim`, `duration = 30*rank + dice(1,6)`. **Effect (`chant_damage`):** vs **Fire**
  chants, `dam -= dam * (SCHOOL_WHITE-20)/35`.

### Flow Break — `chant_flow_break` (anti-magic, #8/#25)
- Strips up to `URANGE(1, 1 + rank/18, 3)` temporary affects (never all; permanent/innate left).
- **Holy Resist** barrier: `duration -= 200 + rank*4`; collapses at 0. Tears down **Vas Gluudo**.
  Shatters victim **Laguna Blade** and **Recovery**.
- **Patryn:** failed save removes a ward outright (+ a block); save only drains the ward
  (`duration -= 300`, floor 1) and has a 25% chance to pop a block (`patryn_active_block/ward`).
- **vs Sorcerer:** on a failed save, `lose_chant(victim)` (disrupts a gathering chant). Does not deal
  damage. (Counts as a Ki-Wall breaker — bypasses the Saiyan Ki-Wall save bonus.)

### Vas Gluudo — `chant_vas_gluudo` (self shield)
- Self `AFF_VAS_GLUUDO`, `APPLY_AC modifier = -rank`, `duration = 30*rank + dice(1,6)`. Grants **+10**
  to `saves_chant` and **−15** to Saiyan Ki-Wave interrupt chance. Torn down by Flow Break.

### Chaos Strings — `chant_chaos_strings` (#6 peel)
- **Debuff (unless `ROOM_SAFE`):** `AFF_CHAOS_STRING` (fight code halves Fist melee, cuts ~1/3 attacks),
  `duration = saved ? (PvP 1 / PvE 2) : (PvP 2 / PvE 4)`.
- **Damage loop:** `dam = dice(15, rank)`, `dam += dam/2 * i`; repeat while
  `number_percent() > (30 - rank/2)` and `i < max` (NPC 6 / PC 4). (Save affects only the debuff
  duration, not damage.)

### Holy Resist — `chant_holy_resist` (self barrier)
- Self `AFF_HOLY_RESIST`, `APPLY_AC modifier = -3*(rank-15)`, `duration = 30*rank + dice(1,6)`.
- **Effects (`chant_damage`, victim is this Sorcerer):** `dam -= dam*2*(SCHOOL_WHITE-20)/100`; plus
  Black/Astral/White chant damage `-15%` (or `-20%` if White-spec). Also `+sorc_rank(WHITE)` to saves.

### Defense — `chant_defense` (emergency barrier)
- Fails if already `gsn_defense`. Self `gsn_defense`, `duration = 2 + rank/20`. Absorbs/shrugs a hit
  in cross-class logic; stripped by Saiyan Ki Wave/Ryuken, Diem Wing, Bomb di Wind, Patryn.

### Chaotic Disintegrate — `chant_chaotic_disintegrate`
- **Damage loop:** `dam = dice(rank, rank)`, `dam += dam/2 * i`; repeat while
  `(number_percent() > 33 || i < min)` and `i < max`. NPC `level>95`: min 3 / max 5; NPC: min 5 / max
  10; PC: min 2 / max 4. No `saves_chant`.

### Ressurection — `chant_ressurection` (group heal)
- Group-only on players. `heal = 1 + Σ URANGE(1, max_hit/30, 1000)` over every **other** char in room;
  if `victim->position < POS_STUNNED`, `heal += heal/2`; capped at **25000**.

---

## Wind magic (`SCHOOL_WIND`) — shamanism elemental

Mobility + lightning damage + wall-breaking (Diem Wing, Bomb di Wind). Wind chants do **2×** damage vs
mobile suits (`chant_damage`). Prep-eligible: **Scatter Brid, Damu Brass, Dimilar Ai**.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| levitation | 1 | 200 | 1 | 10 | 8 | self | |
| scatter brid | 5 | 200 | 1 | 8 | 3 | offensive | ✓ |
| digger bolt | 10 | 300 | 1 | 6 | 2 | offensive | |
| diem wing | 15 | 500 | 1 | 6 | 2 | offensive | |
| damu brass | 20 | 400 | 1 | 10 | 4 | offensive | ✓ |
| raywing | 25 | 1000 | 1 | 12 | 8 | self | |
| mono volt | 30 | 1200 | 1 | 10 | 4 | offensive | |
| dimilar ai | 35 | 1250 | 1 | 10 | 8 | offensive | ✓ |
| windy shield | 40 | 1000 | 1 | 12 | 8 | self | |
| bomb di wind | 50 | 1500 | 4 | 8 | 8 | offensive | |

### Levitation — `chant_levitation`
- Casts `fly` at `level = rank` (`skill_table[fly].spell_fun`).

### Scatter Brid — `chant_scatter_brid`
- **Damage loop:** `dam = dice(10, 10)`, `dam += dam/2 * i`; while `number_percent() > (30 - rank/2)`
  and `i < max` (NPC 10 / PC 5). No save.

### Digger Bolt — `chant_digger_bolt`
- **Damage:** `dice(15, 20 + rank)`; **save→/2**.

### Diem Wing — `chant_diem_wing` (wall-breaker, no damage)
- vs PC only: Fist `F_KI--` (if `>1`); **Sorcerer** `lose_chant`; clears `NEW_FIGUREEIGHT`; strips
  `gsn_kiwall`, `gsn_defense`, `gsn_balus_wall` (each with a message). Bypasses Saiyan Ki-Wall save
  bonus (`chant_breaks_kiwall`).

### Damu Brass — `chant_damu_brass`
- **Damage:** `dice(rank, rank*5/4)`; **save→/2**.

### Raywing — `chant_raywing` (self)
- Self `AFF_FLYING` + `AFF_RAYWING`, `duration = 30*rank + dice(1,6)`.

### Mono Volt — `chant_mono_volt`
- **Damage:** `dice(rank, rank*7/4)`. **No save.**

### Dimilar Ai — `chant_dimilar_ai`
- **Damage:** `dice(rank, rank*3/4)`. **No save.** vs PC: `number_percent() > (mind-30)` → `unstance`.

### Windy Shield — `chant_windy_shield` (self)
- Self `AFF_WINDY_SHIELD`, `duration = 30*rank + dice(1,6)`. **Effect (`fight.c`):** deflects
  projectiles, deflect chance `num = 30 + (SCHOOL_WIND-40)`.

### Bomb di Wind — `chant_bomb_di_wind` (mass wall-breaker, no damage)
- AoE all non-group PCs: zeroes Fist `F_KI`; clears `NEW_FIGUREEIGHT`; strips `gsn_kiwall`,
  `gsn_defense`, `gsn_balus_wall`; clears `NEW_FIRE_BLOCK`, `NEW_AIR_BLOCK`; `unstance`. Bypasses
  Saiyan Ki-Wall save bonus.

---

## Earth magic (`SCHOOL_EARTH`) — shamanism elemental

Knockdown / unstance / heavy hits. Prep-eligible: **Dug Wave**.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| gray bomb | 1 | 150 | 1 | 10 | 8 | offensive | |
| earth 5 | 5 | 1 | 1 | 12 | 8 | self | |
| dill brand | 10 | 750 | 1 | 10 | 4 | offensive | |
| dug wave | 15 | 250 | 1 | 8 | 8 | offensive | ✓ |
| blade haut | 20 | 900 | 2 | 9 | 12 | offensive | |
| bephis bring | 25 | 500 | 1 | 12 | 8 | offensive | |
| vigarthagaia | 30 | 500 | 1 | 12 | 12 | self | |
| mega brand | 35 | 1500 | 1 | 14 | 12 | offensive | |
| dug haut | 40 | 1200 | 2 | 10 | 9 | special/area | |
| vlave howl | 50 | 1400 | 3 | 12 | 12 | offensive | |

### Gray Bomb — `chant_gray_bomb`
- **Damage:** `dice(10, 20 + rank)`; **save→/2**.

### Earth 5 — `chant_earth_5`
- **Empty function — no effect.** (Vestigial; rank 5, Mystic cost 1.)

### Dill Brand — `chant_dill_brand` (unstance, PC only)
- No effect on NPCs. **save→dodge**; on **failed** save, `unstance(victim)`. No damage.

### Dug Wave — `chant_dug_wave`
- **Damage:** `dice(rank, rank*5/4)`; **save→/2**.

### Blade Haut — `chant_blade_haut` (weapon-gated)
- Requires an **edged** wielded weapon (`value[3] == 1 || 3`). **Damage:** `dice(rank, rank*3/2)`.
- vs **Saiyan w/ Ki Wall:** strips it, `dam += dice(4, dam)`. vs **Fist w/ Figure-Eight:** clears it,
  `dam *= 5`. No `saves_chant`.

### Bephis Bring — `chant_bephis_bring`
- **Empty function — no effect.** (Vestigial.)

### Vigarthagaia — `chant_vigarthagaia`
- **Empty function — no effect.** (Vestigial.)

### Mega Brand — `chant_mega_brand`
- **AoE** target's group / all NPCs: `dice(rank, rank*1/2)`; if `number_percent() < rank` and PC,
  `unstance`. No `saves_chant`.

### Dug Haut — `chant_dug_haut` (AoE knockout, TAR_IGNORE)
- All except caster: `dice(rank, rank)`; **save→/2**. PC knocked `< POS_STUNNED` and untied →
  `extras[TIE] = dice(1,3)`.

### Vlave Howl — `chant_vlave_howl` (multi-hit)
- Up to **5** hits while victim alive (`position >= POS_STUNNED`): `dice(rank, rank)` each;
  **save→/3** per hit.

---

## Fire magic (`SCHOOL_FIRE`) — shamanism elemental

Heaviest sustained damage; Balus Wall fully absorbs fire. Mos Varim and Patryn flame ward reduce fire
damage (`chant_damage`). Prep-eligible: **Flare Bit, Flare Arrow, Burst Rondo, Fireball, Flare Lance,
Rune Flare**.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| flare bit | 1 | 125 | 1 | 10 | 5 | offensive | ✓ |
| flare arrow | 5 | 150 | 1 | 7 | 3 | offensive | ✓ |
| val flare | 10 | 330 | 2 | 7 | 3 | offensive | |
| burst rondo | 15 | 455 | 1 | 10 | 6 | offensive | ✓ |
| fireball | 20 | 750 | 2 | 6 | 8 | special/area | ✓ |
| flare lance | 25 | 725 | 1 | 10 | 4 | offensive | ✓ |
| rune flare | 30 | 950 | 1 | 12 | 8 | offensive | ✓ |
| balus wall | 35 | 900 | 1 | 9 | 6 | self | |
| burst flare | 40 | 1200 | 3 | 10 | 8 | offensive | |
| blast bomb | 50 | 4000 | 3 | 12 | 12 | special/area | |

### Flare Bit — `chant_flare_bit`
- **Damage:** `dice(10, 20 + rank)`; **save→/2**.

### Flare Arrow — `chant_flare_arrow`
- **Damage:** `dice(10, 30 + rank)`; **save→/2**.

### Val Flare — `chant_val_flare`
- **Damage:** `dice(10, 40 + rank*2)`; **save→/2**.

### Burst Rondo — `chant_burst_rondo`
- **Damage loop:** `dam = dice(8, 20 + rank)`, `dam += dam/5 * i`; while `number_percent() > (30 -
  rank/2)` and `i < max` (NPC 7 / PC 4).

### Fireball — `chant_fireball` (AoE, TAR_IGNORE)
- All non-group: `dice(rank, rank)`; **save → `dam -= dam/3`**.

### Flare Lance — `chant_flare_lance`
- **Damage:** `dice(rank, rank*3/2)`; **save→/2**. Melts a target's `NEW_VAN_REHL` icy barrier.

### Rune Flare — `chant_rune_flare`
- **Damage loop:** `dam = dice(rank, rank/3)`, `dam += dam/5 * i`; while `number_percent() > (30 -
  rank/2)` and `i < max` (NPC 8 / PC 4).

### Balus Wall — `chant_balus_wall` (self)
- Self `gsn_balus_wall`, `duration = 30` (fixed). **Effect (`chant_damage`):** a Fire-school chant
  hitting this Sorcerer is **fully absorbed** (`dam=0`) and the wall is stripped. Also half-absorbs
  Gaav Flare (Black); scattered by Diem Wing / Bomb di Wind.

### Burst Flare — `chant_burst_flare`
- **Damage:** `dice(rank, rank*4)`; **save→/2**. Melts `NEW_VAN_REHL`. Applies the `flame breath`
  affect, `duration = dice(7,5)` (note: applied unconditionally here, `bitvector 0`).

### Blast Bomb — `chant_blast_bomb` (AoE, TAR_IGNORE)
- All non-group: `dice(rank, rank*5)`; **save → `dam -= dam/3`**. Melts `NEW_VAN_REHL`.

---

## Water magic (`SCHOOL_WATER`) — shamanism elemental

Ice damage, utility (Aqua Create, Dark Mist field), an execute (Ly Briem), and a PvP fatality (Sea
Cucumber Spin). Prep-eligible: **Freeze Arrow, Freeze Brid, Icicle Lance**.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| aqua create | 1 | 125 | 1 | 12 | 10 | special/area | |
| freeze arrow | 5 | 150 | 1 | 7 | 4 | offensive | ✓ |
| dark mist | 10 | 100 | 1 | 15 | 12 | special/area | |
| ly briem | 15 | 1000 | 1 | 13 | 12 | offensive | |
| freeze brid | 20 | 450 | 1 | 10 | 8 | offensive | ✓ |
| icicle lance | 25 | 625 | 1 | 10 | 6 | offensive | ✓ |
| sea cucumber spin | 30 | 750 | 1 | 12 | 10 | offensive | |
| vice freeze | 35 | 2500 | 1 | 14 | 12 | offensive | |
| demona crystal | 40 | 1500 | 3 | 10 | 10 | offensive | |
| van rehl | 50 | 1500 | 3 | 10 | 10 | offensive | |

### Aqua Create — `chant_aqua_create`
- Casts `create spring` at `level = rank`.

### Freeze Arrow — `chant_freeze_arrow`
- **Damage:** `dice(10, 40 + rank)`; **save→/2**.

### Dark Mist — `chant_dark_mist` (#22 room field, TAR_IGNORE)
- Marks `ch->in_room` in a registry with `timer = 5`, owner = caster. While active: non-allies take
  **−10 hitroll** (`fight.c`), grouped allies get **+15% flee** (`do_flee`); **fire burns it away**;
  else expires on timer (`dark_mist_update`). An enemy-owned room can't be hijacked; owner cleared on
  extract.

### Ly Briem — `chant_ly_briem` (execute)
- **Hit chance:** `chance = rank + will/2`; NPC: `0` if `level>95` else `-= level`; PC: `-= max_hit/100`.
  Miss if `number_percent() > chance`. **On hit:** `chant_damage(victim->hit + dice(2,5))` — instakill.

### Freeze Brid — `chant_freeze_brid`
- **Damage:** `dice(rank, rank*3/2)`; **save→/2**.

### Icicle Lance — `chant_icicle_lance`
- **Damage:** `dice(rank, rank*3/2)`; **save→/2**.

### Sea Cucumber Spin — `chant_sea_cucumber_spin` (PvP fatality)
- PC only. Refuses if `EVAL < 10`; **dodge** if `EVAL >= 50`; needs a `spring` object in room. On a
  living target: `do_fatality`, then **`raw_kill`** with `extras[TIMER]=20`, `PKCOUNT += 5`, `level=1`,
  suit cleared — a hard PvP kill/de-level. Autosaves the caster.

### Vice Freeze — `chant_vice_freeze`
- **Damage:** `dice(rank, rank*5/2)`; **save→/2**.

### Demona Crystal — `chant_demona_crystal`
- **AoE** target's group / all NPCs: `dice(rank, rank*4)`; **save→/2** (rolled vs primary victim).

### Van Rehl — `chant_van_rehl` (group icy barrier)
- Self + group gain `NEW_VAN_REHL` (icy barrier). Melted by fire chants (Flare Lance, Burst Flare,
  Blast Bomb). No damage. (Listed `offensive` in table but resolves as a group buff.)

---

## Astral magic / Shamanism (`SCHOOL_ASTRAL`)

The shamanism specialty school; spec-ing Astral also makes all four elemental schools in-spec. Mana
drains, scry/enchant utility, the Ra-Tilt nuke. Prep-eligible: **Bram Blazer, Elmekia Lance**.

| Chant | Rank/Mystic | Cost | Lines | Lag | Wait | Target | Prep |
|---|---|---|---|---|---|---|---|
| bram blazer | 1 | 130 | 1 | 4 | 4 | offensive | ✓ |
| elmekia lance | 5 | 150 | 1 | 6 | 5 | offensive | ✓ |
| assha dist | 10 | 200 | 1 | 10 | 8 | offensive | |
| astral detect | 15 | 350 | 1 | 12 | 10 | special/area | |
| astral break | 20 | 600 | 1 | 10 | 9 | offensive | |
| shadow snap | 25 | 500 | 1 | 12 | 10 | offensive | |
| elmekia flame | 30 | 2250 | 1 | 12 | 10 | offensive | |
| astral vine | 35 | 3000 | 1 | 12 | 10 | special/area | |
| vision | 40 | 1500 | 1 | 12 | 10 | special/area | |
| ra-tilt | 50 | 4000 | 4 | 12 | 10 | offensive | |

### Bram Blazer — `chant_bram_blazer`
- **Damage:** `dice(15, 20 + rank)`; **save→/2**. **Mana drain:** if victim survives and `dam <
  victim->mana`, `mana -= (oldhit - victim->hit) / (6 - rank/10)`, floored at 1.

### Elmekia Lance — `chant_elmekia_lance`
- **Damage:** `dice(15, 30 + rank)`; **save→/2**. Same mana drain as Bram Blazer.

### Assha Dist — `chant_assha_dist`
- **Damage:** `dice(30, 30 + rank)`; **save→dodge** (full negate).

### Astral Detect — `chant_astral_detect` (locate object, TAR_IGNORE)
- Locates objects by name (skips `ITEM_NO_LOCATE`). **Anti-scry vs Patryn** holder (Air runes on TORSO,
  `level>=2`, `P_DEFENSES`): 5+ Air runes or `gsn_spirit_ward` → that item hidden; 3–4 → "vision
  clouds"; 1–2 → warning glow only.

### Astral Break — `chant_astral_break`
- **Damage:** `dice(rank, rank*3)`; **save→dodge** (full negate).

### Shadow Snap — `chant_shadow_snap` (root)
- Fails if already `AFF_NO_MOVE`. **Double save** (`saves_chant` rolled twice — either passing dodges).
  Else `AFF_NO_MOVE`, `duration = dice(2, rank) - 1`, `APPLY_AC modifier = dice(2, rank)`.

### Elmekia Flame — `chant_elmekia_flame`
- **Damage:** `dice(rank, rank*2)`; **save→/2**. Same mana drain as Bram Blazer.

### Astral Vine — `chant_astral_vine` (enchant weapon, TAR_IGNORE)
- Weapons only; no uniques/hardened. `chance = number_percent() - (50 - powers[SCHOOL_ASTRAL])`; strips
  & re-banks existing hit/dam (`chance -= (hit+dam)*3/2`); AC affects adjust chance. Outcomes:
  `<15` disintegrate, `<45` nothing, `<85` +1 hit/+1 dam, else +2/+2 (`enchant weapon` affects,
  `duration = -1`).

### Vision — `chant_vision` (scry, TAR_IGNORE)
- Can't while fighting. `get_char_world`; fails on `saves_chant` or self. **Anti-scry vs Patryn**
  (Air-rune torso, `P_DEFENSES`): 5+ runes or spirit ward fully blocks; 3–4 dissolves; 1–2 warns only.
- **Protected rooms** (vnum 1, 8400–8599, 8100–8299): backlash `damage(ch, ch, 29999, DAM_SHOCKSHIELD)`
  + `WAIT_STATE 40`. Else briefly teleports the caster to look, then returns.

### Ra-Tilt — `chant_ra_tilt` (ultimate)
- `chance = number_percent()` + modifiers. **NPC:** `dam = dice(rank, rank*8)`, `chance += 100-level`,
  `-20` if `level>95`. **PC:** `dam = dice(rank, 400 - EVAL*3)`, `chance -= (hit+max_hit)/1500`,
  `chance = 0` if `level >= 17`.
- **Resolution:** `chance <= 20` → resist (no damage); `<= 80` → normal `dam`; else **consume** —
  10 × `chant_damage(10000)`.

---

## Combat math (`src/fight.c:chant_damage`)

Every offensive chant's damage passes through `chant_damage(ch, victim, dam, dt)` where `dt = cn`
(`rank = chant_table[dt].rank`, `school = chant_table[dt].school`). Key transforms, in order:

- **Hard clamp:** `dam > 30000 → 30000` (bug-logged). Safe-room check (`is_safe`) aborts.
- **Curse fizzles:** opposed-element curse pairs make the school's chants "fizzle and die" (e.g. Wind
  fizzles with earth+flame curse; Black with flame+water; Astral with earth+wind, etc.). The caster's
  own matching curse is stripped on cast.
- **Scorched:** Black-school `dam += dam/10` if victim has the `scorched` affect.
- **Sanctuary:** `dam /= 2` (non-suit). **Steely:** `dam /= 5`.
- **EVAL gate:** a low-`EVAL` (`<50`) player hitting a maxed char/NPC has damage scaled down to
  `dam * (1 + EVAL/10) / 6`.
- **NPC soak:** `dam -= dam * level / (300 + rank*2)`; `level>95` adds `-= dam*(level-95)*10/100`.
- **Mobile suits:** Wind `dam *= 2`; armor soak `dam * (armor-60)/160`, capped 75%.
- **Per-class victim soaks:** Saiyan (Ki Wall `/2`, aegis), Fist (Figure-Eight `/2`, F_TORSO), Patryn
  (Earth/Fire rune mitigation + per-school wards), Mazoku (White `+50%`, matter/nihilism), **Sorcerer**
  (Holy Resist −15%/−20% on Black/Astral/White; `dam*2*(WHITE-20)/100`; Mos Varim vs Fire; Balus Wall
  full-absorb vs Fire), and Patryn Spirit Ward −15% (any chant school).
- **Death:** dropping a Sorcerer to ≤ `POS_MORTAL` runs `while(chant != NULL) lose_chant` — all queued
  chants are lost.

---

## ⚠️ Needs verification

- **`cost` field semantics.** Confirmed the **Mystic** cost paid by `do_chant` is the `rank` field, not
  `cost`; the `cost` field is read only by prepared auto-casts (`cost/2` mana in `multi_hit`) and the
  old commented-out mana check. No other live reader of `cost` was found — worth a second pass.
- **`prepare`/`multi_hit` off-by-one.** Both loop `for (i = MAX_CHANT; i > 0; i--)`, reading
  `chant_table[81]` (out of bounds, valid 0–80) on the first iteration. Latent; not changed.
- **AoE save application.** Several AoE chants (Dynast Brass, Laguna Blast, Mega Brand, Demona Crystal)
  roll `saves_chant` against the **primary** `victim` once and apply the result to every target in the
  loop. Verify this is intended vs per-victim saves.
- **Dragon Slave follow-up tick.** Uses **raw** `powers[SCHOOL_BLACK]` (uncapped by `sorc_rank`) for
  `dice(rank, rank*3)`; confirm off-spec Black sorcerers get the uncapped DoT (likely an oversight).
- **Burst Flare DoT.** Applies the `flame breath` affect **unconditionally** (no save branch), unlike
  Flame Breath which gates it on a failed save. Confirm intended.
- **Vestigial chants.** `chant_earth_5`, `chant_bephis_bring`, `chant_vigarthagaia` are empty stubs
  (no effect) yet remain learnable/listed. Documented as no-ops; confirm none are wired elsewhere.
- **Van Rehl target type.** Table marks it `TAR_CHAR_OFFENSIVE` but the function only buffs self/group
  with `NEW_VAN_REHL`. The offensive targeting requires picking a victim to cast — confirm the UX.
- **Mana-drain divisor.** Bram Blazer / Elmekia Lance / Elmekia Flame use `/(6 - rank/10)`; at `rank=50`
  this is `/1`. Rank caps at 50, so the divisor never reaches ≤0 — noted for clamp safety.
- **Ra-Tilt PC scaling.** `dam = dice(rank, 400 - EVAL*3)` can produce a non-positive die size at high
  `EVAL` (≥134); `dice()` behavior there not verified. Also `chance=0` for PCs `level>=17` makes the
  consume branch PvP-unreachable — confirm intended.
- **Windy Shield / Mos Varim / Visfarank magnitudes.** The `(SCHOOL_WIND-40)` and `(SCHOOL_WHITE-20)`
  terms can go negative at low rank; the net effect at low rank was not fully traced.
- **`chant_damage` soak ordering.** The exact order of Holy Resist / Spirit Ward / per-class soaks and
  whether they compound multiplicatively as written was read top-to-bottom but not exhaustively unit-checked.
