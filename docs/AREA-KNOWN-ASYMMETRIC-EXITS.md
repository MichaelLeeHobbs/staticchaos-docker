# Known asymmetric exits (the boot `Fix_exits` warnings) (#82)

At boot the server logs a handful of `BUG: Fix_exits: A:d -> B:d' -> C.` lines.
These are **not bugs** and the game world is correct. `fix_exits()` (`src/db.c`)
only *reports* them; it never changes an exit. The message fires when room A has an
exit to room B, and B *also* has an exit in the reverse direction but it leads to
some room C ≠ A — i.e. the two directions don't mirror.

That asymmetry is normal and intentional here: it's how one area links into another
(the destination's reverse direction belongs to *its own* area's layout), and how
one-way / maze passages work. Forcing symmetry would **break** the destination
area's internal connections, so the data is deliberately left as-is.

## The current accepted set (5)
| Warning | What it is |
|---------|-----------|
| `1037:5 -> 1017:4 -> 820` | Room 1017 (Copper In the Air) is a junction: down to 1037 (same area), up to 820 (link into the Larsen area). |
| `812:0 -> 904:2 -> 903`   | Cross-area link: Larsen 812 → Olympus 904; 904's south is Olympus-internal (903). |
| `813:2 -> 1312:0 -> 1313` | Cross-area link: Larsen 813 → Anon High Tower 1312; 1312's north is tower-internal (1313). |
| `814:2 -> 3001:0 -> 3054` | Cross-area link: Larsen 814 → **Diku Midgaard 3001** (the temple); 3001's north is Midgaard-internal (3054). |
| `5063:1 -> 5031:3 -> 5030` | Intra-area maze twist in the Great Eastern Desert (one-way / non-mirrored desert paths). |

## For maintainers
Treat these five as the expected baseline. A **new** `Fix_exits` line means an area
edit introduced an asymmetry — review it: if it's another deliberate cross-area or
one-way link, add it here; if a destination vnum is a typo, fix the `.are` data.
Do **not** blanket-symmetrize existing exits.
