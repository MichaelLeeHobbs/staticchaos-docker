---
issue: 45
type: bug
status: spec-review
opened: 2026-06-30
reporter: code-audit (#35)
---

# Patryn flame lance (case 132) rebound uses P_AIR (#45)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/45

## The bug (confirmed in source)
`do_runeweave` **case 132** (Fire + Death, "flame lance") in `src/classes/patryn.c`. The normal hit rolls off
the Fire pool, but the **rebound** branch (self-damage when the weave rebounds) rolls off **Air**:
```
rebound: dam = dice( ch->pcdata->mind/4, ch->pcdata->powers[P_AIR]  ); damage( ch, ch, dam, DAM_FIRE ); // BUG
normal : dam = dice( ch->pcdata->mind/2, ch->pcdata->powers[P_FIRE] );                                   // ok
```
A fire weave's self-damage shouldn't scale off the Air pool — clearly a copy-paste slip.

## Fix
Change the rebound branch's `powers[P_AIR]` to `powers[P_FIRE]`. One token.

## ⚠️ Balance impact (maintainer decision)
Rebound self-damage will scale with **Fire** runes instead of Air. A Patryn casting a fire weave
typically has Fire ≥ Air, so the rebound gets slightly larger — but it only fires on the (uncommon)
rebound path, so impact is small. **Confirm: fix as-is** (recommended), or leave it?

## Acceptance criteria
- [ ] Rebound damage uses `powers[P_FIRE]`; normal hit unchanged. Compiles (32-bit Docker).

## Risk
Trivial; only the minor rebound-magnitude change above.
