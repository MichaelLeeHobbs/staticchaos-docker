# Mazoku — Ability Reference (Balance / Mechanics)

> **Audience:** balance review and maintainers. This is the exhaustive, numbers-first reference
> for **every** Mazoku ability, extracted directly from the server source. For the new-player
> narrative version, see **[Mazoku — Player Guide](CLASS-MAZOKU.md)** (do not balance-check against
> that doc; this one is authoritative).

**Last verified against source:** commit `cefb094`
**Primary sources:** `src/mazoku.c`, `src/fight.c`, `src/update.c`, `src/handler.c`, `src/const.c`, `src/merc.h`, `src/interp.c`, `src/act_info.c`, `src/saiyan.c`, `src/patryn.c`

## How to read this

- **Each ability is cited `file:function`** so every number can be checked against the C. If a number
  here disagrees with the code, the **code wins** — fix this doc.
- **Two resource pools, two currencies.** Mazoku is a Diku-style class that does **not** gain mana
  on level (`fMana = FALSE`). In combat it spends **essense** (`M_ESSENSE`, a regenerating pool, cap
  100000) for forms/attacks/utility. Out of combat it spends **experience** (`ch->exp`) to raise the
  four mastery stats and **primal** (`ch->pcdata->primal`) to permanently learn abilities.
- **Comparisons matter at the margin.** Most essense gates use `<=` (so you need **strictly more**
  than the listed number); a few use `<` (you need **at least** the number). The exact comparison is
  given per ability.
- **Formulas are quoted verbatim** from the source. `dice(n, size)` rolls `n` dice of `1..size`
  (mean ≈ `n*(size+1)/2`). `isquare(x)` is the integer square root. The mastery stats are
  `matter = M_MATTER`, `astral = M_ASTRAL`, `focus = M_FOCUS`, `nihilism = M_NIHILISM`,
  `ego = M_EGO`, all in `ch->pcdata->powers[]`.
- **WAIT_STATE(ch, n)** is lag in pulses (`PULSE_VIOLENCE`), the standard combat-round unit.
- **Damage types:** Mazoku energy attacks route through `damage()` in `src/fight.c` as one of four
  "colors" — `DAM_CERULEAN` (1500), `DAM_OBSIDIAN` (1501), `DAM_CRIMSON` (1502), `DAM_EMERALD`
  (1503). Melee from formed arms uses `TYPE_HIT+N` (see *Arm forms*).
- **⚠️ Needs verification** items are collected at the bottom — don't trust those numbers blindly.

## Class basics

Source: `src/const.c` `class_table[CLASS_MAZOKU]`, `src/merc.h` (struct `class_type`).

| Field | Value | Notes |
|---|---|---|
| `who_name` | `Mazoku` | |
| `attr_prime` | `APPLY_INT` | Prime attribute = **Intelligence** |
| First weapon | `OBJ_VNUM_SCHOOL_DAGGER` | Newbie weapon |
| Guild room | `3018` | |
| Max trainable Body | `95` | |
| Max trainable Mind | `95` | |
| Max trainable Spirit | `95` | |
| Max trainable Will | `95` | All four caps equal (95) — no specialized stat |
| HP gained per level | `10` | |
| Gains mana | `FALSE` | Not a mana class — combat resource is essense |

**Power slots** (`src/merc.h`, indices into `ch->pcdata->powers[]`):

| Slot | Index | Meaning |
|---|---|---|
| `M_LEARNED` | 0 | Bitfield of permanently developed abilities/parts |
| `M_SET` | 1 | Bitfield of the **currently active** form/arm/parts |
| `M_CTYPE` | 2 | Color (damage type) of the energy currently being charged; `0` = none |
| `M_CTIME` | 3 | Charge timer (ramps up while charging; damage multiplier source) |
| `M_EGO` | 4 | Ego — fuels Astral Strike, buffs parry; gained from worthy kills |
| `M_NIHILISM` | 5 | Nihilism — innate damage reduction; **lowers** evasion (see below) |
| `M_ESSENSE` | 6 | Essense — the in-combat resource pool (regenerates; cap 100000) |
| `M_MATTER` | 7 | Matter mastery (0–100) — melee damage + physical mitigation |
| `M_ASTRAL` | 8 | Astral mastery (0–100) — energy-attack damage + astral mitigation/regen |
| `M_FOCUS` | 9 | Focus mastery (0–100) — to-hit (lowers victim evasion), teleport range |

**Form / arm / part bits** (`M_SET` and `M_LEARNED` share these values, `src/merc.h`): body forms
`M_HUMAN=1`, `M_BATTLE=2`, `M_TRUE=4`; arm forms `M_HANDS=8`, `M_CLAWS=16`, `M_SPIKES=32`,
`M_BLADES=64`, `M_TENTACLES=128`; extra parts `M_THIRD=512`, `M_FOURTH=1024`, `M_FIFTH=2048`,
`M_SIXTH=4096`, `M_WINGS=8192`, `M_EYES=16384`; abilities `M_TELEPORT=32768`, `M_CHARGE=65536`,
`M_BLAST=131072`, `M_BOLT=262144`, `M_BOMB=524288`, `M_ASTRIKE=1048576`.

**Score readout** (`src/act_info.c:do_score`): prints `Essense / Ego / Nihilism` and
`Matter / Astral / Focus`.

---

## 1. Resource & advancement

### Develop (learn / raise) — `develop [list|<thing>]`
*Source: `src/mazoku.c:do_develop` · `interp.c`: POS_STANDING, level 2*

The master advancement command. `develop` with no arg (or `develop list`) prints all prices.
Three different currencies are spent depending on the target:

**Mastery stats — cost is `exp` (experience):**

| Target | Cost (exp) | Cap | Comparison |
|---|---|---|---|
| `nihilism` | `M_NIHILISM * 1000` | floor 0 | fails if `nihilism <= 0` or `M_NIHILISM*1000 > exp`; **lowers** nihilism by 1 |
| `matter` | `(M_MATTER*10+500)^2` | `>= 100` (maxed) | fails if `exp < cost`; raises by 1 |
| `astral` | `(M_ASTRAL*10+500)^2` | `>= 100` (maxed) | fails if `exp < cost`; raises by 1 |
| `focus` | `(M_FOCUS*10+500)^2` | `>= 100` (maxed) | fails if `exp < cost`; raises by 1 |

- **Nihilism is inverted:** `develop nihilism` **spends exp to decrease** the nihilism counter
  ("more at peace with yourself"). Nihilism is *gained automatically* by taking damage (see below),
  so this command is how you buy it back down. Cost scales with the current value: `M_NIHILISM*1000`.
- **Matter/astral/focus** cost `(stat*10+500)^2` exp — e.g. stat 0 → 250000, stat 50 → 1,000,000,
  stat 99 → ~2.2M. Each is capped at 100.

**Abilities & body parts — cost is `primal`** (`SET_BIT(M_LEARNED, flag)`; fails if already learned
or `primal < cost`):

| Develop keyword | Primal cost | `M_LEARNED` bit |
|---|---|---|
| `claws` | 50 | `M_CLAWS` |
| `third` (3rd arm) | 75 | `M_THIRD` |
| `fourth` (4th arm) | 100 | `M_FOURTH` |
| `wings` | 100 | `M_WINGS` |
| `eyes` (pineal eyes) | 100 | `M_EYES` |
| `spikes` | 125 | `M_SPIKES` |
| `charge` | 125 | `M_CHARGE` |
| `blades` | 150 | `M_BLADES` |
| `tentacles` | 150 | `M_TENTACLES` |
| `fifth` (5th arm) | 150 | `M_FIFTH` |
| `blast` | 150 | `M_BLAST` |
| `bolt` | 150 | `M_BOLT` |
| `bomb` | 150 | `M_BOMB` |
| `sixth` (6th arm) | 200 | `M_SIXTH` |
| `teleport` | 200 | `M_TELEPORT` |
| `astrike` | 200 | `M_ASTRIKE` |

### Essense regeneration & nihilism gain (passive)
*Source: `src/fight.c:essense_gain`, `src/fight.c:damage`, `src/update.c` (regen tick)*

- **Essense from dealing damage** (`essense_gain`, called from `damage()`): only for a Mazoku
  attacker. If `ch == victim` (self-damage), instead `M_NIHILISM = UMIN(100, M_NIHILISM+1)`. Otherwise:
  - `gain = dam`; if `dt != DAM_OBSIDIAN` then `gain /= 10` (obsidian-typed damage banks **10×**
    more essense than any other type).
  - Diminishing returns: `gain -= gain * M_ESSENSE / 100000`.
  - Banked only if it won't overflow: `if ((M_ESSENSE + gain) < 100000) M_ESSENSE += gain`.
- **Essense from passive tick** (`update.c`, `level >= 2`): if `M_ESSENSE < 10000` then `M_ESSENSE++`
  per tick — i.e. essense slowly trickles up to 10000 even out of combat, but only combat damage
  pushes it past 10000 toward the 100000 cap.
- **Nihilism from taking damage** (`damage`, two paths, melee `~line 1070` and chant/spell
  `~line 1622`): when a Mazoku victim takes `dam > victim->max_hit/10` from someone other than
  itself, `if (M_NIHILISM < 100) M_NIHILISM++`. **Nihilism therefore climbs as you get hit**, raising
  damage reduction but degrading your evasion/parry/dodge (see *Passive combat math*). Buy it back
  down with `develop nihilism`.

### Reform (heal) — `reform`
*Source: `src/mazoku.c:do_reform` · POS_STANDING, level 2*

Spends essense to repair HP.

- **Cost (essense):** `cost = isquare(max_hit - hit) + 50`; fails if `M_ESSENSE < cost`.
- **Heal:** `hit = UMIN( hit + 100 + dice(3, astral), max_hit )`.
- **Lag:** `WAIT_STATE(ch, 6)`.

---

## 2. Forms

A Mazoku is shapeless astral matter. Its **body form** (`morph`) sets its toughness profile and
gating; its **arm form** (`form`) sets its melee weapon. Switching forms costs essense.

### Morph (body form) — `morph <human|battle|true>`
*Source: `src/mazoku.c:do_morph` · POS_FIGHTING, level 2*

Mutually exclusive body forms in `M_SET`. Blocked entirely if `M_HUMAN` arm-lock applies (see Form).

| Form | Bit | Essense cost | Comparison | Effects |
|---|---|---|---|---|
| `human` | `M_HUMAN` | 50 | fails if `M_ESSENSE <= 50` | Grants permanent `AFF_INFRARED` (infravision); forces `M_HANDS` arm form; can wear material gear. `WAIT_STATE 4`. |
| `battle` | `M_BATTLE` | 100 | fails if `M_ESSENSE <= 100` | Grants permanent `AFF_INFRARED`. Monstrous combat form. `WAIT_STATE 8`. |
| `true` | `M_TRUE` | **free** (no essense) | — | Strips `AFF_INFRARED` (`gsn_infrared`). Pure astral form; required for `teleport`. `WAIT_STATE 8`. |

- Morphing into anything **other than `human`** force-drops all equipped non-`ITEM_ASTRAL` gear
  (`unequip_char`), because a non-human Mazoku "has no need for such material things"
  (`handler.c:equip_char` blocks wearing material gear unless `M_HUMAN`).
- Each morph clears the other two body bits first. `human` also clears non-hand arm forms and sets
  `M_HANDS`.

### Form (arm form) — `form <hands|claws|spikes|blades|tentacles>`
*Source: `src/mazoku.c:do_form` · POS_FIGHTING, level 2*

Sets the active arm weapon in `M_SET` (mutually exclusive). **Blocked entirely** if `M_HUMAN` is set
("You wouldn't be very human with anything other than hands.").

| Arm | Bit | Essense cost | Requires developed? |
|---|---|---|---|
| `hands` | `M_HANDS` | 5 | no (always available) |
| `claws` | `M_CLAWS` | 10 | yes (`M_LEARNED` `M_CLAWS`) |
| `spikes` | `M_SPIKES` | 12 | yes |
| `blades` | `M_BLADES` | 15 | yes |
| `tentacles` | `M_TENTACLES` | 25 | yes |

- **Comparison:** fails if `M_ESSENSE <= cost` (need strictly more than the cost).
- Clears all other arm bits, then `SET_BIT(M_SET, flag)` and `M_ESSENSE -= cost`. No lag.
- The chosen arm form determines melee damage type, melee damage bonus, extra attacks, and
  evasion/defense modifiers — see *Arm forms (melee)* and *Passive combat math*.

### Grow (extra parts) — `grow <third|fourth|fifth|sixth|eye|wings>`
*Source: `src/mazoku.c:do_grow` · POS_STANDING, level 2*

Permanently manifests an extra body part into `M_SET` (extra arms add rake/parry/attacks; eyes give
sight; wings are cosmetic/flight-adjacent).

- **Cost (essense):** flat `50000`; fails if `M_ESSENSE <= 50000`. `M_ESSENSE -= 50000`.
- **Requires** the part already developed (`M_LEARNED`) and not already grown (`M_SET`).
- **Lag:** `WAIT_STATE(ch, 12)`.
- Parts: `third`→`M_THIRD`, `fourth`→`M_FOURTH`, `fifth`→`M_FIFTH`, `sixth`→`M_SIXTH`,
  `eye`→`M_EYES`, `wings`→`M_WINGS`.

---

## 3. The charge / release system (core resource attack)

The Mazoku's signature mechanic. You **charge** energy of a chosen color over time (each tick costs
50 essense and ramps `M_CTIME`), then **release** it as a bolt, blast, or bomb. Longer charge =
bigger `M_CTIME` = more damage. Getting stunned, exhausted, or hit hard scatters the charge.

### Charge — `charge <obsidian|emerald|cerulean|crimson>`
*Source: `src/mazoku.c:do_charge` · POS_FIGHTING, level 2*

- **Requires:** `M_CHARGE` developed; must be `POS_FIGHTING`; `M_ESSENSE >= 250`
  (fails if `M_ESSENSE < 250`). **Note:** the 250 essense is only a *gate* — `do_charge` itself does
  **not** spend essense; the per-tick drain (below) does.
- **Color → damage type (`M_CTYPE`):** `cerulean`→`DAM_CERULEAN`, `obsidian`→`DAM_OBSIDIAN`,
  `crimson`→`DAM_CRIMSON`, `emerald`→`DAM_EMERALD` (matched by `str_prefix`).
- **Starting a charge while one is already up** dispels the old one first (`M_CTYPE=0, M_CTIME=0`)
  then begins the new color.
- **Sets:** `M_CTYPE = color`, `M_CTIME = 10`.
- **Lag:** `WAIT_STATE(ch, 15 - (M_ASTRAL/10))` — higher astral charges faster.

### Charge ramp & upkeep (per-tick) — passive
*Source: `src/update.c:second_update` (Mazoku charge block, `M_CTYPE != 0`)*

Each game tick while a charge is held:

- **If `position < POS_STUNNED`:** charge lost (`M_CTIME=0, M_CTYPE=0`) — being beaten down scatters it.
- **Else if `M_ESSENSE < 50`:** charge lost ("Your exhaustion causes you to lose your charge.").
- **Else if `M_CTIME == (15 + M_ASTRAL/10)`:** at the cap — prints "You cannot charge up any more!",
  `M_ESSENSE -= 50`, `M_CTIME++` (one final tick to `16 + M_ASTRAL/10`).
- **Else if `M_CTIME >= (16 + M_ASTRAL/10)`:** fully charged — `M_ESSENSE -= 50` per tick to *hold*
  it (no further ramp).
- **Else:** `M_ESSENSE -= 50`, `M_CTIME++` (normal ramp).

So `M_CTIME` climbs from 10 by 1 per tick up to a max of `16 + M_ASTRAL/10`, costing 50 essense each
tick. The release damage scales with `(M_CTIME - 5) / 5` (see Release), so the practical maximum
multiplier is `(16 + astral/10 - 5)/5` (e.g. astral 100 → `M_CTIME` 26 → `(26-5)/5 = 4` integer).

### Release — `release <bolt|blast|bomb> [target]`
*Source: `src/mazoku.c:do_release` · POS_FIGHTING, level 2*

Fires the held charge. Requires a charge up (`M_CTYPE > 0` and `M_CTIME > 0`) and the matching attack
mode developed (`M_BOLT` / `M_BLAST` / `M_BOMB`). Target is `ch->fighting` if fighting, else `arg2`
via `get_char_room`. **Damage type is the charged color `dt = M_CTYPE`.** Charge is cleared
(`M_CTYPE=0, M_CTIME=0`) after any successful release.

**Color damage multiplier `mod` (×/10):**

| Color (`M_CTYPE`) | `mod` |
|---|---|
| `DAM_CRIMSON` | 35 vs NPC, **10 vs player** |
| `DAM_EMERALD` | 20 |
| `DAM_CERULEAN` | 15 |
| `DAM_OBSIDIAN` | 10 |
| (default) | 1 |

- **True-form bonus:** if `M_SET` has `M_TRUE`, `mod = mod * 7 / 4` (×1.75).

**Per-mode base damage** (then `dam = dam * mod / 10; dam = dam * (time-5) / 5;` where `time = M_CTIME`):

| Mode | Base dice | Target | Lag |
|---|---|---|---|
| `bolt` | `dice(15, M_ASTRAL)` | single (`damage`, `dt`) | `WAIT_STATE 4` |
| `blast` | `dice(7, M_ASTRAL)` | single (`damage`, `dt`) + disruption suite | `WAIT_STATE 4` |
| `bomb` | `dice(11, M_ASTRAL)` per victim | every non-group char in room | `WAIT_STATE 8` |

- **Blast cross-class disruption (PvP only, after damage):**
  - **vs Fist** (`number_percent() < 50`): if `F_KI >= 4`, `F_KI -= dice(1,4)`; clears
    `NEW_FIGUREEIGHT`; `WAIT_STATE(victim, 4)` ("breaks your rhythm").
  - **vs Saiyan:** strips `gsn_kiwall` if up ("collapses your wall of Ki").
  - **vs Sorcerer:** `lose_chant(victim)` (interrupts the chant).
- **Bomb** hits every char in the room not in `ch`'s group (`is_same_group`); each takes an
  independent `dice(11, M_ASTRAL)` roll scaled by `mod` and `(time-5)/5`.

### Charge-scatter interactions (incoming, cross-class)
A held charge (`M_CTYPE != 0`) can be scattered by enemies, and also counts as a "major setup state"
that other classes punish:

- **Saiyan Ki Wave** (`saiyan.c:do_kiwave`, ~line 712): resets `M_CTYPE=0, M_CTIME=0`
  ("Stunned, you lose control of your charge!").
- **Saiyan Ryuken** (`saiyan.c:do_ryuken`, ~line 1142): scatters the charge (`M_CTYPE=0, M_CTIME=0`)
  and counts as `broke_major`.
- **A charging Mazoku is a "setup" target** (`saiyan.c` ~line 1084): Ryuken's +10% setup bonus
  triggers against a Mazoku with `M_CTYPE != 0`.
- **Patryn block path** (`patryn.c`, ~line 722): if a Mazoku victim has `M_CTIME > 0`, the dark
  energies are dispelled (`M_CTIME=0, M_CTYPE=0`).
- **Chant/spell damage degrades a held charge** (`fight.c:chant_damage` ~line 1578): if a Mazoku
  victim has `M_CTIME >= 10`, `M_CTIME -= dam/250` on each spell hit.

---

## 4. Arm forms (melee) & arm-driven attacks

When a Mazoku is unarmed (no wielded weapon), its melee damage type and base damage come from the
active arm form. The arm forms also unlock dedicated special attacks (`rake`, `gouge`, `lash`).

### Arm-form melee type & base damage
*Source: `src/fight.c:one_hit`*

- **Damage type** (`one_hit`, ~line 296; unarmed, `dt >= TYPE_HIT`): `dt = TYPE_HIT` then
  `+5` claws, `+2` spikes, `+1` blades, `+4` tentacles (selects the weapon-skill slot
  `weapons[dt-TYPE_HIT]`).
- **Base damage** (`one_hit`, ~line 390): for a Mazoku with no weapon and `dt >= TYPE_HIT`,
  `dam = number_range( weapons[dt-TYPE_HIT]/4, weapons[dt-TYPE_HIT]/3 )` (scales with that arm's
  weapon skill).
- **Mazoku melee bonus** (`one_hit`, ~line 470, `level >= 2`), applied to `dam`:
  - **claws:** `dam += dam * 3 * (50 + M_MATTER + M_FOCUS) / 120`
  - **spikes:** `dam += dam * 3 * (M_MATTER + M_FOCUS) / 150`
  - **blades:** `dam += dam * 4 * (M_MATTER + M_FOCUS) / 120`
  - **tentacles:** `dam += dam * 3 * (M_MATTER + M_FOCUS) / 170`
  - **else (hands/none):** `dam += dam * 3 * M_MATTER / 100`
- **vs mobile suit:** `dam /= 2` (Mazoku melee is halved against suits).

### Rake — `rake`
*Source: `src/mazoku.c:do_rake` · POS_FIGHTING, level 2*

Multi-hit claw flurry. **Requires `M_CLAWS` active.**

- **Attacks:** base `2`, `+1` for each of `M_THIRD`, `M_FOURTH`, `M_FIFTH`, `M_SIXTH` (so up to 6
  with all four extra arms).
- **Per hit:** `dam = dice(5, weapons[5])`; `+dam/2` vs NPC. Typed `TYPE_HIT+5` (claws). Repeats
  while the victim is alive, `i < attacks`, and `number_percent() < 83` (≈83% chance to continue
  each extra swing).
- **Lag:** `WAIT_STATE(ch, 6)`.

### Gouge — `gouge`
*Source: `src/mazoku.c:do_gouge` · POS_FIGHTING, level 2*

Spike strike to the eyes that **blinds**. **Requires `M_SPIKES` active.**

- **Resist check** (no blind if it passes): vs player, `victim body - 50 > number_percent()`; vs NPC,
  `victim level > number_percent() + 10`. On resist: `WAIT_STATE 8`, no effect.
- **Blind duration:** `duration = weapons[2]/6 + 5`; but for NPCs `level >= 96`, `duration = dice(1,5)`.
  Slot `weapons[2]` is the same slot spike melee trains (`M_SPIKES → dt = TYPE_HIT+2`), so the blind
  lengthens with spike proficiency — mirroring rake/claws (`weapons[5]`). *(Was `weapons[11]`, a slot
  spike use never trained — fixed in #49.)*
- **Effect:** applies `blindness` (`AFF_BLIND`, `APPLY_HITROLL -4`) for `duration`.
- **Lag:** `WAIT_STATE(ch, 8)`.

### Lash — `lash`
*Source: `src/mazoku.c:do_lash` · `interp.c`: POS_RESTING, level 2*

Tentacle strike that can knock a player out of stance. **Requires `M_TENTACLES` active.** (Has a
separate mobile-suit "heat rod" branch that fires first if the user `IS_SUIT` with a heat rod ready —
not Mazoku ability behavior.)

- **Position gate:** must be `>= POS_FIGHTING` ("You're too relaxed.").
- **Damage:** `dice(5, 100)`, typed `TYPE_HIT+4` (tentacles). Not weapon-skill scaled.
- **Unstance:** vs a standing player, `if (number_percent() < ch->pcdata->body - 20) unstance(victim)`.
- **Lag:** `WAIT_STATE(ch, 12)`.

---

## 5. Astral abilities & utility

### Astral Strike (astrike) — `astrike`
*Source: `src/mazoku.c:do_astrike` (arm) → `src/fight.c:multi_hit` (resolve) · POS_FIGHTING, level 2*

Arms a phase attack: your next melee round materializes behind the target for a big bonus hit. Sets
`M_ASTRIKE` in `M_SET`; consumed when the round resolves.

- **Requires:** `M_ASTRIKE` developed; `M_ESSENSE > 250` (fails if `<= 250`); `M_EGO > 1` (fails if
  `<= 1`); not blind (`gsn_blindness`); must be fighting (`POS_FIGHTING` with a target); not already
  astriking.
- **Cost:** `M_ESSENSE -= 250`; `WAIT_STATE(ch, 8)`.
- **Resolve** (`fight.c:multi_hit`, ~line 213): on the next attack round it materializes behind the
  victim. Against an **NPC** it first checks `M_EGO <= 1` → aborts ("lack the will"); otherwise vs an
  NPC it spends `M_EGO -= 1`. (Against a **player** no ego is spent on resolve.) `M_ASTRIKE` is then
  removed after the round (`~line 261`).
- **Damage bonus while astriking** (`one_hit`, ~line 482): `+dice(3, dam/2)` vs NPC, `+dice(3, dam)/3`
  vs player, added to each melee hit that round.
- **Evasion edge** (`fight.c` dodge/parry/block): astriking applies `chance /= 2` to the *defender's*
  parry/block against you and cuts your own dodge `chance -= chance/4` (except vs a `M_TRUE` Mazoku,
  where your dodge is forced to `80`). See *Passive combat math*.
- **Retaliation risk:** if the victim is a player with `NEW_BALL_LIGHTNING` (Patryn lightning ball),
  the astrike hit triggers `damage(victim, ch, dice(18,65), DAM_SHOCKSHIELD)` back at you
  (`one_hit`, ~line 545).
- **Cleared on flee/room change** (`handler.c` ~line 644): leaving combat removes `M_ASTRIKE`.

### Teleport — `teleport <target>`
*Source: `src/mazoku.c:do_teleport` · POS_STANDING, level 2*

Area-range teleport to an NPC's room. Drains every pool.

- **Requires:** `M_TELEPORT` developed; **must be in `M_TRUE` form**; not `M_ASTRIKE`;
  `M_ESSENSE >= 500` (fails if `< 500`); `hit > 500`, `mana > 500`, `move > 500` (each fails if
  `<= 500`); no fight timer (`extras[TIMER] > 0` blocks — "Why would you want to RUN from a fight?").
- **Target:** `get_char_area`; must be an NPC with `level <= 95`; and `victim level <= M_FOCUS - 5`
  (focus gates how high-level a target you can lock onto).
- **Cost:** `M_ESSENSE -= 500`, `hit -= 500`, `mana -= 500`, `move -= 500`.
- **Lag:** `WAIT_STATE(ch, 10)`.

### Instantiate (create astral gear) — `instantiate <slot>`
*Source: `src/mazoku.c:do_instantiate` · `interp.c`: POS_STANDING, level 2, LOG_ALWAYS*

Conjures a blank `ITEM_ASTRAL` armor piece (wearable in non-human forms; imbue to power it up).

- **Cost (essense):** `10000`; fails if `M_ESSENSE <= 10000`. `M_ESSENSE -= 10000`.
- **Created object:** `value[0] = 15` (AC set to max). Slots: ring, amulet, shirt, cap, pants, boots,
  gloves, sleeves, cloak, belt, bracer, shield, sceptre (`OBJ_VNUM_ASTRAL_*`).
- No lag.

### Imbue (enchant astral gear) — `imbue <item>`
*Source: `src/mazoku.c:do_imbue` · `interp.c`: POS_STANDING, level 2, LOG_ALWAYS*

Pours essense into an astral item to add hit/dam.

- **Target:** a carried `ITEM_ASTRAL` item ("Only astral items can be imbued.").
- **Cap:** current summed `APPLY_HITROLL` must be `< 10` ("as powerful as it can become" at `>= 10`).
- **Cost (essense):** `cost = 8000 + (hitdam * 8000)` where `hitdam` is the current hitroll total;
  fails if `M_ESSENSE <= cost`. `M_ESSENSE -= cost`.
- **Effect:** removes existing `APPLY_HITROLL`/`APPLY_DAMROLL` affects and re-adds both at
  `hitdam + 1` (permanent, `duration -1`). So each imbue raises both hit and dam by 1 (max +10/+10),
  with cost climbing 8000 → 16000 → … per level.
- No lag.

---

## 6. Passive combat math (always-on, `src/fight.c` / `src/handler.c` / `src/update.c`)

These are not commands but govern how much damage a Mazoku deals/takes and how well it evades, based
on form, arms, and the `M_MATTER` / `M_ASTRAL` / `M_FOCUS` / `M_NIHILISM` / `M_EGO` pools.

### Damage taken (mitigation)
*Source: `src/fight.c:damage` (melee path ~line 1012) and `src/fight.c:chant_damage` (~line 1564)*

- **vs `DAM_KIFLAME`** (melee path): `dam += dam/3` first (Mazoku take **+33% from ki-flame**), then
  `dam -= dam * M_NIHILISM / 400`.
- **Form mitigation (melee path):**
  - `M_HUMAN`: `dam -= dam * M_MATTER / 130`
  - `M_BATTLE`: `dam -= dam * (M_MATTER + 15) / 135`
  - `M_TRUE`: `dam -= dam * M_ASTRAL / 140`
- **Form mitigation (chant/spell path, `spell_damage`):** if chant school is `SCHOOL_WHITE`,
  `dam += dam/2` first (Mazoku take **+50% from white magic**), then:
  - `M_HUMAN`: `dam -= dam * (M_MATTER + 12) / 135`
  - `M_BATTLE`: `dam -= dam * M_MATTER / 135`
  - else (`M_TRUE`/none): `dam -= dam * M_ASTRAL / 150`
  - plus `if (M_NIHILISM > 0) dam -= dam * M_NIHILISM / 400`.
- **Nihilism reduces all incoming damage** (`/400` term) but is gained by *being hit hard* and only
  removable by spending exp — a self-balancing tank stat.

### Damage dealt
See *Arm forms (melee)* for the per-arm melee bonus and the mobile-suit halving, and *Release* for
energy-attack scaling. Key driver stats: `M_MATTER` + `M_FOCUS` (melee), `M_ASTRAL` (energy).

### Extra attacks
*Source: `src/fight.c:calc_attacks` (~line 3611)*

- **In `M_BATTLE` form:** `+1` attack for each grown extra arm (`M_THIRD`, `M_FOURTH`, `M_FIFTH`,
  `M_SIXTH`).
- **Arm form:** `M_SPIKES` `+1`; `M_BLADES` `+2`; `M_TENTACLES` `+7`.

### Parry
*Source: `src/fight.c:check_parry` (~line 1846)*

- Mazoku can parry **barehand** (the no-weapon `return FALSE` is bypassed for Mazoku, ~line 1848).
- **As defender** (`~line 1860`): `+5` per grown extra arm (third/fourth/fifth/sixth); `M_HUMAN`
  `+20` (only if no sixth arm — `else if`); arm form bonus `M_BLADES +20`, `M_TENTACLES +30`,
  `M_SPIKES +15`, `M_CLAWS +10`; then `chance -= M_NIHILISM/2` and `chance += UMIN(100, M_EGO)/2`.
- **As attacker** (defender's parry against you, `~line 1900`): `chance -= M_NIHILISM/3`; arm
  penalties `M_SPIKES -= M_FOCUS/2 + 20`, `M_CLAWS -= M_FOCUS/3`, `M_BLADES -= M_FOCUS/3`; if
  `M_ASTRIKE`, defender's `chance /= 2`.

### Dodge
*Source: `src/fight.c:check_dodge` (~line 1982)*

- **As defender:** `M_BATTLE` `+ M_MATTER*3/4`; `M_HUMAN` `+ M_MATTER/2`.
- **As attacker** (your dodge, `~line 2011`): `chance -= M_NIHILISM/3`; `M_CLAWS -= M_FOCUS/2`,
  `M_SPIKES -= M_FOCUS/2 + 20`, `M_BLADES -= M_FOCUS/3`,
  `M_TENTACLES -= (M_FOCUS + M_MATTER)/3`; if `M_ASTRIKE`, your dodge is forced to `80` vs a
  `M_TRUE` Mazoku, else `chance -= chance/4`.

### Block (vs Fist monk block)
*Source: `src/fight.c:check_block` (~line 2089)*

- When the Mazoku is the attacker: `M_SPIKES -= M_MATTER*2/3`; `M_BLADES`/`M_CLAWS -= M_MATTER/2`; if
  `M_ASTRIKE`, defender's `chance /= 2`.

### Backstab vulnerability
*Source: `src/fight.c:one_hit` (~line 501)*

- A Mazoku victim takes `dam *= 2` from backstab (on top of the base `dice(2,4)` multiplier) when the
  attacker is **not** a Fist.

### Regeneration (per tick)
*Source: `src/update.c:regen_update` (~line 1459, `level >= 2`)*

- **`M_TRUE` form:** `hit/mana/move += 150 + dice(6, M_ASTRAL)` each (strong astral regen).
- **Other forms:** `hit/mana/move += dice(6, 10)` each.
- Plus the essense trickle `if (M_ESSENSE < 10000) M_ESSENSE++`.

### Senses & vision
*Source: `src/handler.c:can_see` (~line 1433), `can_see_obj` (~line 1466), `src/act_info.c` (~line 495)*

- With `M_EYES` grown, a Mazoku can see characters and objects (true-sight equivalent) and is not
  blocked by darkness in room display.

### Ego gain
*Source: `src/fight.c:group_gain` (~line 2562)*

- On a kill where `victim level > EVAL/2`, `M_EGO = UMIN(200, M_EGO + 1)` ("You bask in $N's dying
  agony."). Ego fuels Astral Strike and buffs parry.

### Evaluation / effective level
*Source: `src/handler.c:eval` (~line 1745)*

- Mazoku EVAL: `(max_hit-2000)/3000` (counts hp twice, no secondary stat) `+ (M_MATTER+1)/20
  + (M_ASTRAL+1)/20 + (M_FOCUS+1)/20`, `+1` per developed `M_LEARNED` bit in the
  `2^4..2^15` range, `+ max_move/2250 + max_mana/2250`.

---

## ⚠️ Needs verification

- **`do_release` essense cost.** Release spends **no essense** in `do_release` (the only essense gate
  is the 250 checked by `do_charge`, and the 50/tick drained while charging in `update.c`). Confirm
  this is intended — the entire essense cost of a charged attack is the per-tick upkeep, not the shot.
- **~~`do_gouge` blind duration uses `weapons[11]`.~~** *(Resolved, #49.)* Gouge now reads
  `weapons[2]` — the slot spike melee actually trains (`M_SPIKES → dt = TYPE_HIT+2`) — so the blind
  duration scales with spike use, consistent with rake/claws (`weapons[5]`). The old `weapons[11]`
  (rapier/stiletto/dirk in `wgen_table`) was never trained by using spikes.
- **`do_instantiate` duplicate `boots` / fallback help text.** The `else if` chain lists `boots`
  twice and the no-match help text omits some slots vs the success list; cosmetic, but confirm every
  advertised slot is reachable (e.g. `shield`, `cloak`).
- **Charge `M_CTIME` cap interplay with `(time-5)/5`.** Confirmed `M_CTIME` maxes at
  `16 + M_ASTRAL/10` and damage scales `(time-5)/5` (integer). Double-check the off-by-one at the
  `== (15 + astral/10)` branch (it ticks once more to `16 + astral/10`) gives the intended top
  multiplier, and that a freshly-charged `M_CTIME = 10` yields the `(10-5)/5 = 1` floor multiplier.
- **`mod` default = 1.** If `M_CTYPE` is somehow not one of the four colors, `release` damage collapses
  to `dam * 1/10 * (time-5)/5` (near-zero). Not reachable via `do_charge` (which only sets valid
  colors), but confirm no save/load path can leave a stale/invalid `M_CTYPE`.
- **`develop nihilism` lower bound.** It refuses at `M_NIHILISM <= 0`; confirm nihilism cannot go
  negative through any other path and that the exp cost `M_NIHILISM*1000` is charged off the
  *pre-decrement* value.
