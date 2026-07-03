---
issue: 26
type: feature
status: spec-review
opened: 2026-06-27
reporter: ScathachMDG (via #14)
---

# Show derived stat effects in identify (#26)

GitHub issue: https://github.com/MichaelLeeHobbs/staticchaos-docker/issues/26

## Summary
`equipment` was fixed (#14) to show each item's real *effect* (e.g. `+2 STR → Dam+4`), but
`identify` still prints the raw stat name ("affects strength by 2"), so it stays misleading.
Reuse the #14 translation in `identify` so item-affect output is consistent everywhere.

## Current state (verified)
- `equipment` (`src/commands/act_info.c`): translates affects → derived buckets via global helpers
  `eq_add_derived` (act_info.c:1473) + `eq_render_derived` (act_info.c:1500).
- `spell_identify` (`src/world/magic.c:1540`): prints raw `affect_loc_name(paf->location) ... by N`
  (lines ~1628 / ~1638, over both `pIndexData->affected` and `obj->affected`).
- Wiz `stat` (`src/commands/act_wiz.c:741/748`): same raw loop.

The raw `STR/DEX/INT/WIS/CON` labels don't reflect the `affect_modify` remap, so identify under-
sells/mislabels gear the same way equipment used to.

## Proposed design
1. **Shared renderer:** add `show_obj_affects( CHAR_DATA *ch, OBJ_DATA *obj )` (in act_info.c,
   prototyped in merc.h) that walks both affect lists through `eq_add_derived` and prints a single
   `eq_render_derived` line (e.g. `Affects: Dam+4 Hp+10 AC-10`). This also collapses the
   duplicated affect-display loop the #5/#6 reviews flagged.
2. **Apply to `identify`:** replace its raw per-affect lines with `show_obj_affects(ch, obj)`.
3. **Wiz `stat` (maintainer call):** admins may *prefer* the raw `APPLY_` location for debugging/OLC.
   Either leave it raw, or show both raw + derived. Recommend leaving wiz stat raw; decide at review.

## Acceptance criteria
- [ ] `identify` on an item with stat affects shows the derived effect (`Dam+4`, `Hp+10 Move+10`,
      `Save-2`, …), matching what `equipment` shows.
- [ ] Items with direct hit/dam/ac/etc. affects still display correctly (combined).
- [ ] No change to item mechanics (display only).
- [ ] Dev verification: `identify` a stat-bearing item and a direct-bonus item; compare to `equipment`.

## Risk & balance notes
- Display only; zero mechanical/balance impact. Keep `eq_add_derived` the single source of the
  APPLY_→derived mapping (already mirrors `affect_modify`).

## Out of scope
- The exact wiz `stat` decision (raw vs derived) — maintainer call at review.
