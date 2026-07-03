# TheMudConnector listing — recommended answers (Static Chaos)

Reference for filling out the TMC registration form accurately. **The form as
screenshotted had several fields wrong that *undersell* the MUD** — fix these first.

## ⚠️ Corrections (the form had these wrong)
| Field | Form showed | Correct | Why |
|-------|-------------|---------|-----|
| **Is Ansi Color Offered?** | No | **Yes** | ANSI color is fully supported (`colordef.h`, the `ansi` toggle, colored combat/prompts). |
| **Out-of-band Protocol (MSDP/GMCP/ATCP/ZMP)?** | No | **Yes — GMCP** | GMCP (telnet opt 201) is implemented in `src/core/gmcp.c`, with shipped Mudlet packages. A real selling point — don't hide it. |
| **Training System** | Not Applicable | **Has one (God-Wars stat-training)** | The `train` command spends XP on stats/Primal/Avatar/Legend ranks — that *is* the progression. Pick the closest "custom/skill-based" option, not N/A. |
| **World Originality** | All Original | **Mostly Original** | The world mixes original areas with some stock Diku content (e.g. stock **Midgaard**, vnum 3001). "Mostly/Mixed original" is accurate; "All Original" isn't. |

## Section 1 — Basic Information (codebase)
- **Codebase:** **Merc** (Merc 2.x), a **DikuMUD** derivative. Lineage: Diku → Merc → Static Chaos. If TMC offers "Custom," "Custom (Merc-based)" is also fair.
- **Additional codebase info:** *Custom Merc 2.x codebase (DikuMUD-derived), "Static Chaos / Chaosium" by Nicholas "Alathon" Lennig. Heavily modified: custom anime classes (Saiyan, Fist, Patryn, Mazoku, Sorcerer), God-Wars-style PvP + stat-training, GMCP, full OLC. Dockerized and actively developed.*
- Note: the `credits` command must (and does) report the Diku/Merc authors — a license requirement for listing publicly.

## Section 3 — Specific Mud Details (full field-by-field)
| Field | Answer | Note |
|-------|--------|------|
| Geographical Location | USA | your call (server location) |
| Primary Language | English | |
| Number of Players Online | Under 10 | ~3 testers currently |
| Operational Status | Open for player testing | accurate — not a public launch yet |
| Do you Run a Custom Server (coded from scratch)? | **Yes** (your call) | Not literally from scratch — it's heavily-modified Merc. "Yes" is fair given the customization; keep the codebase field = Merc so it stays honest. |
| Is your Mud Newbie Friendly? | No | honest — complex God-Wars PvP, steep curve (the trials/tutorial help, but it's not casual) |
| Extensive Race Selection? | No | no race system |
| Extensive Class Selection? | Yes | 5 deep, bespoke classes |
| Multiclassing Allowed? | No | one class per character |
| Is the Mud Class-less? | No | classes are central |
| Is the Mud Level-less? | **Yes** | a `level` exists but power comes from trained stats + Primal unlocks, God-Wars style — "level-less progression" fits |
| Are Quests Offered? | Yes | questmaster + per-class trials |
| Multiplaying Allowed (multiple chars/person)? | **policy call** | code's multiplay *detection* is disabled (`#if 0`), so it's not enforced — set to your intended policy (shown: No) |
| Playerkilling Allowed? | Yes | it's a PvP MUD |
| New Characters Need Approval? | No | open creation |
| Equipment Saved on logoff? | Yes | pfiles persist inventory/eq |
| Detailed Character Customization? | Yes | stat training, Primal unlock trees, class abilities |
| Can Players Join Clans? | Yes | full clan system |
| Recruiting Builders? | policy call | OLC exists; set per your needs |
| Recruiting Coders? | policy call | set per your needs |
| Pay-to-Play? | No | Diku/Merc license forbids charging |
| Pay-for-Perks? | No | same |
| **Ansi Color Offered?** | **Yes** ⚠️ | supported (see corrections) |
| Xterm 256 Colors? | No | basic ANSI only |
| **Out-of-band Protocol?** | **Yes — GMCP** ⚠️ | see corrections |
| MSP (sound)? | No | |
| Pueblo? | No | |
| MCCP (compression)? | No | not in the server (only the optional web-proxy would add it) |
| MXP? | No | |
| Screen-Reader support? | No | no special support |
| **Training System** | **Custom / stat-training** ⚠️ | `train` (God-Wars) — not "N/A" |
| Equipment System | Custom (or N/A) | standard Diku eq + the **mobile-suit** layer + Mazoku self-crafted astral gear; "Custom" is fairer than N/A |
| Player-Run Cities? | No | |
| Roleplaying | No Roleplay | PvP-focused, RP not enforced |
| Crafting System | None | (Mazoku astral gear is a limited exception) |
| Mapping System | None | in-MUD; the companion website has maps, but that's external |
| **World Size** | **Small (under 3,000 rooms)** | confirmed: **2,595 rooms across 47 areas** |
| **World Originality** | **Mostly Original** ⚠️ | mix of original + some stock (Midgaard); not "All Original" |
| Adult Content | No Adult Content (with a caveat) | no sexual/explicit content, but there is period profanity/crude text — "No Adult Content" is defensible if "adult" = sexual; flag if you'd rather note mature language |

## Section 5 — Mud Description (suggested blurb)
> **Static Chaos** — a God-Wars-style PvP MUD on a heavily-customized Merc/DikuMUD
> codebase. Fight as a **Super Saiyan**, a Slayers-style **Sorcerer** or **Mazoku**
> shapeshifter, a rune-weaving **Patryn**, or a ki-fueled **Fist** — each a deep,
> bespoke class. Train your stats, weave your build with **Primal**, pilot a
> **Gundam mobile suit**, join a clan, and prove yourself in class **trials** and
> open PvP. GMCP + Mudlet packages, full ANSI color, actively developed.
