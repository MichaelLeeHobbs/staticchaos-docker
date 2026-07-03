# Class ability-dispatch: descriptor-table spike (#76)

**Recommendation: do NOT undertake the broad "ability descriptor table" refactor.**
The premise that motivated #76 has largely been addressed by other means, and the
class dispatch is already mostly data-driven. A full refactor of the five class
files would be a large, balance-critical change for low marginal value.

## What #76 proposed
Replace the per-ability `switch`-on-magic-case dispatch across saiyan/fist/patryn/
mazoku/sorcerer with a data-driven descriptor table (name, cost index, dice/formula,
flags), so a new ability is a table row rather than a hand-copied branch — killing
the copy-paste bug class behind #44/#45/#48/#49.

## What the code actually looks like now (spike findings)
| Class | Dispatch shape | Switch/case |
|-------|----------------|-------------|
| saiyan | 20 `do_*` funcs, one per ability, registered in `cmd_table` (interp.c) | none |
| mazoku | 14 `do_*` funcs, one per ability | 1 (non-dispatch) |
| sorcerer | **`chant_table`** — already a data table of chants | 4 (non-dispatch) |
| patryn | `do_runeweave` switch, now `case RUNE_FIRE + RUNE_DEATH:` (named, #54) | 2 |
| fist | `do_combo` `switch(powers[F_KI])` — **ki-level tiers** (`case 10:`..), not ability IDs | 2 |

So: the interpreter's `cmd_table` already routes each class command to its own
function (data-driven at the command layer); sorcerer already uses `chant_table`;
saiyan/mazoku are one-function-per-ability with no switch dispatch; patryn's
runeweave switch is already readable named-constant cases; and fist's only real
switch is an inherently numeric **level → effect** mapping (combo tiers by ki
level), which a table wouldn't meaningfully improve (each tier has bespoke logic).

## Why the value is now low
The copy-paste bug class (#44/#45/#48/#49) came from **bare parallel-array magic
indices** and a few copy-pasted branches. That has been mitigated directly:
- `weapons[]`→`WP_*` (#52), `stances[]`→`STANCE_*` + runeweave cases named (#54),
  `condition/suit/extras2` named (#58), non-literal formats fixed + guarded (#71).
- `pnpm lint:c` now enforces named indices (6 rules) and flags unchecked fopen; the
  specific bugs (#44/#45/#48/#49) are fixed and closed.

With the indices named and lint-guarded, a new ability copied from an existing one
is already far less likely to reproduce the old bugs — which was the refactor's
main justification.

## Cost vs. benefit
A descriptor-table refactor would rewrite dispatch in balance-critical files
(3.5k-line sorcerer, 1.5k-line patryn, etc.), risking subtle behavior/balance
regressions, for a bug class that is already largely fenced off. High risk, low
marginal value.

## If pursued anyway
Do it as a **narrow, incremental** effort, one dispatcher at a time behind the new
smoke + lint gates — never a single sweeping change. The only candidate with any
table-shaped uniformity is a future rework of fist `do_combo`'s tiers, and even
that is marginal. Otherwise, prefer continuing the cheaper wins (naming, lint
rules, the smoke test) that address the same failure modes at a fraction of the risk.
