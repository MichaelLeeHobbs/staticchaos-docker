# Player save-file format & how to change it safely

Player files (`player/<letter>/<Name>`) are plain-text key/value records written by
`save_char_obj` and read by `fread_char` / `load_char_obj` in `src/save.c`. This
note documents the two field styles and the **procedure for adding or changing a
persisted field without corrupting existing player files** — the guard behind
issues #52, #54, and #61.

## Two field styles

1. **Keyed scalars** (the majority) — e.g. `Str 18`, `Wimpy 20`, read via the
   `KEY(...)` macro. These are **order-independent** and **optional**: an unknown
   key is skipped, a missing key keeps the struct's default. Adding a keyed scalar
   is therefore backward-compatible for free (old files simply lack the line).

2. **Positional integer rows** — e.g. `Weapons 0 0 0 ...` (13 ints),
   `Stances ...` (11), `Extras`/`Extras2` (10), `Suit`/`Kills` (15), `Powers` (10),
   `Clan` (10), `Condition` (3). These write a **fixed count** of ints on one line
   and historically read the same fixed count back in sequence. This is the fragile
   part: change the count and old/new files disagree.

## The version field

`pcdata->version` is stamped with `PFILE_VERSION` (`merc.h`) on every save and read
back on load (`Version` key). `save.c` uses it to run one-time **migrations** when
loading an older file — see the worked examples around the `Extras`/`Extras2` loads
(`version < 1/2/3`). Bump `PFILE_VERSION` whenever you change the meaning or layout
of persisted data, and gate the migration on it.

## The positional-row guard (#61)

Positional rows are loaded through **`fread_number_row(fp, int *dest, int max)`**
and **`fread_number_row_sh(fp, sh_int *dest, int max)`** (`db.c`). They read up to
`max` ints from the *current line*, **zero-fill** any the line doesn't supply, and
**stop at the newline** — so if a persisted array's size is grown, older/shorter
rows load with the new slots zeroed instead of the read desyncing into the next key
line and corrupting the file. For a correctly-sized row they behave exactly like the
old `for(i<max) dest[i]=fread_number(fp)` loop, so existing files load identically.

Two variants because `sh_int` is `short` on this build (see `merc.h`) and must not
alias `int`. Route by the array's declared type:
`weapons`/`stances`/`clan` are `sh_int` → `_sh`; `powers`/`extras`/`extras2`/`suit`/
`kills` are `int` → plain.

**Still on the old fixed loop (convert when next touched):** `condition` (size 3,
stable) and the 2-D `runes[5][15]` loads.

## Procedure: add or grow a persisted field

- **New keyed scalar:** add the `fprintf` in `save_char_obj` and a `KEY(...)` in the
  loader. No version bump needed (old files default it). Done.
- **New positional row (array):** add the `fprintf` writer and load it via
  `fread_number_row[_sh]`. Old files lack the line → the array keeps its zeroed
  default. Bump `PFILE_VERSION` if you also need a migration.
- **Grow an existing array's size (bump its `MAX_*`):** update the writer's format
  string and count. The guarded reader already tolerates old shorter rows
  (new slots zero-filled). Bump `PFILE_VERSION` and add a `version < N` migration
  **only if** the new slots need a non-zero default.
- **Never renumber or reorder existing slots.** Positional load is by index, so
  reordering silently corrupts every existing file. Name the indices instead
  (`WP_*`, `STANCE_*`, `COND_*`, `SUIT_*`, … — issues #52/#54/#58) so intent is
  reviewable, and only ever *append*.

## Verifying a save-format change

1. `docker compose build mud` (32-bit compile gate).
2. Boot; create a throwaway character; `save`; `quit`.
3. Inject distinctive values into the character's positional rows, log back in
   (exercises the load path), `save`, and confirm the pure-storage rows come back
   identical (runtime-mutated slots like timers/condition/ki will differ — that's
   expected, not corruption). This is exactly how #61 was validated.
