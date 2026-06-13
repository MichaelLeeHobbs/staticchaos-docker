# GMCP Implementation Plan — Static Chaos

> **Status (2026-06-13):**
> - **Phase 0 + 1** — implemented, **deployed**, verified live (WILL GMCP offered,
>   `Core.Hello` parsed, `Char.Vitals` pushed once/sec, text uncorrupted, non-GMCP
>   clients unaffected).
> - **Phase 2 (`Room.Info`)** — implemented, **deployed**, verified live (a fake
>   client walking the Mud School got valid `Room.Info {num,name,area,exits}`,
>   including the down/up link to the Temple of Midgaard). Emitted from
>   `char_to_room`.
> - **Client:** Mudlet packages in `client/` — gauges (`Char.Vitals`) and
>   auto-mapper (`Room.Info`). Distributed manually (the XML) for now.
> - Phase 3 (status/completion) remains; auto-install via `Client.GUI` + a web
>   host is a later, separate task.
>
> Full plan before any code. GMCP (Generic Mud Communication Protocol) is a small
> telnet side-channel that lets the server send the client structured data (JSON)
> out-of-band. It's the foundation for modern Mudlet features: status gauges, an
> auto-mapper, and context-aware tab completion. **Opt-in by design — non-GMCP
> clients (raw telnet, anyone who declines) see zero change.**
>
> Grounded against the current source (`comm.c`, `merc.h`) with file:line refs.

---

## How this engine does I/O today (and why GMCP needs a small input change)

- **Telnet is already partly used.** `comm.c:107` includes `<arpa/telnet.h>`, so
  `IAC, WILL, WONT, DO, DONT, SB, SE, GA, TELOPT_ECHO` are all available, and the
  engine already negotiates (echo off/on for passwords `comm.c:108-110`, optional
  GA). So adding another option is well-precedented.
- **Input is raw + line-based.** `read_from_descriptor` (`comm.c:1140`) `read()`s
  raw bytes into `d->inbuf[4*MAX_INPUT_LENGTH]` with **no telnet parsing**.
  `read_from_buffer` (`comm.c:1213`) waits for a `\n`/`\r`, then copies only
  `isascii && isprint` characters into `d->incomm` (`:1254`).
- **Consequence:** a GMCP packet `IAC SB 201 …json… IAC SE` contains no newline
  (so `read_from_buffer` stalls waiting for one) and its bytes would be partly
  stripped (IAC=0xFF isn't printable) and partly injected as a bogus command
  (the JSON text). **So GMCP requires a telnet pre-parser in the input path** —
  this is the one real piece of work.
- **Output is simple.** `process_output` → `write_to_descriptor(desc, txt, len)`
  (`comm.c:1832`) is a raw socket write (`len==0` ⇒ `strlen`). GMCP output will
  bypass the normal text/colour buffer and call `write_to_descriptor` directly
  with an explicit length.
- **Connection setup.** `new_descriptor` (`comm.c:903`) builds the descriptor and
  sends the greeting/initial negotiation — the place to offer `IAC WILL GMCP`.
- **Descriptor struct** `merc.h:629` has `inbuf/incomm/inlast/outbuf/connected` —
  we add a couple of small GMCP fields here.

---

## Protocol primer (GMCP in a nutshell)

- GMCP is **telnet option 201** (`0xC9`). `arpa/telnet.h` may not name it, so we
  `#define TELOPT_GMCP 201`.
- **Enable:** server sends `IAC WILL 201`; client replies `IAC DO 201` (enable) or
  `IAC DONT 201` (decline). We only send GMCP to descriptors that said DO.
- **A message:** `IAC SB 201 "Package.Message <json-value>" IAC SE`. The text
  before the first space is the package/message; the rest is JSON (often an object
  or array; sometimes absent).
- **Client → server (we receive):** `Core.Hello {"client":"Mudlet","version":"…"}`,
  `Core.Supports.Set ["Char 1","Room 1",…]`, `Core.Supports.Add/Remove`, `Core.Ping`.
- **Server → client (we send):** standard packages like `Char.Vitals`, `Char.Status`,
  `Room.Info`. Using the **standard names** means players can use off-the-shelf
  Mudlet GUI packages instead of bespoke scripts.

---

## Architecture

Keep it self-contained to minimise edits to the hot I/O path:

- **New module:** `src/gmcp.c` + `src/gmcp.h` — all GMCP logic lives here.
- **`merc.h` / DESCRIPTOR_DATA:** add `bool gmcp;` (negotiated on), and
  `int gmcp_supports;` (bitmask of modules the client asked for; optional — we can
  send regardless at first). Optionally `char *gmcp_client;` (name from Core.Hello).
- **Input hook (the crux):** a telnet filter that runs on `d->inbuf` right after
  the `read()` in `read_from_descriptor`, extracting/handling all `IAC …` sequences
  and compacting the buffer so `read_from_buffer` only ever sees clean text.
- **Output helper:** `void send_gmcp( DESCRIPTOR_DATA *d, const char *msg, const char *json )`
  → builds `IAC SB 201 <msg> <json> IAC SE`, `write_to_descriptor` with explicit
  length, guarded by `d->gmcp`.
- **Negotiation:** `new_descriptor` writes `IAC WILL 201` after the greeting.

---

## Phase 0 — Core plumbing (the one real chunk)

1. **Constants / struct** — `#define TELOPT_GMCP 201`; add `gmcp` (+ `gmcp_supports`)
   to DESCRIPTOR_DATA; free/clear them in `close_socket` and on descriptor reuse.
2. **`telnet_filter(d)`** — scan `d->inbuf` for `IAC` (compare as **`unsigned char`**
   — `IAC`=0xFF is `-1` as signed `char`, a real gotcha on the `-m32` build):
   - `IAC IAC` → keep one literal `0xFF`.
   - `IAC WILL/WONT/DO/DONT <opt>` → negotiation. `DO 201` ⇒ `d->gmcp = TRUE` (and
     send our `Core` handshake); `DONT 201` ⇒ `d->gmcp = FALSE`. Reply politely to
     other options (WONT/DONT) so we don't hang clients.
   - `IAC SB <opt> … IAC SE` → if `opt==201`, hand the payload to `gmcp_in(d, …)`;
     otherwise ignore. Strip the whole block.
   - Any other `IAC <x>` → strip (1–2 bytes).
   - **Partial packets:** if an `IAC SB …` has no closing `IAC SE` yet (split across
     two `read()`s), **leave it in `inbuf` untouched** and process it next tick when
     the rest arrives. (Most important robustness detail.)
   - Compact the buffer in place so only real text remains for `read_from_buffer`.
3. **`gmcp_in(d, payload, len)`** — split `"Module.Message"` from the JSON tail;
   handle `Core.Hello` (log/store client), `Core.Supports.Set/Add/Remove` (set the
   bitmask), `Core.Ping` (reply). **No full JSON parser needed for input** — we
   mostly care which modules the client wants; a lenient token scan is plenty.
4. **`send_gmcp(d, msg, json)`** — as above; the single choke point for all output.
5. **Hook-ups:** call `telnet_filter(d)` inside `read_from_descriptor` after the
   read; send `IAC WILL 201` in `new_descriptor`; add `gmcp.o` to the Makefile
   `O_FILES`; prototypes in `merc.h`.

**Risks & mitigations (Phase 0 is the only sensitive part):**

| Risk | Mitigation |
|---|---|
| Input path touches *all* players | Filter only acts when it sees `IAC` (0xFF); pure-ASCII input passes through byte-for-byte. Everything gated behind negotiation. |
| `char` signedness (`-m32`) | Compare bytes as `unsigned char` throughout the parser. |
| Split `IAC SB…SE` across reads | Leave the incomplete sequence in `inbuf`; process when complete. Explicit test. |
| Buffer compaction dropping real text | Careful in-place rewrite + a fake-client test that interleaves text and GMCP. |
| Breaking non-GMCP clients | Regression test with raw `telnet`: declines WILL 201, plays identically to today. |

---

## Phase 1 — `Char.Vitals` (status gauges) — first visible payoff

- Build a small JSON object: `{"hp":1700,"maxhp":2000,"mana":…,"maxmana":…,"move":…,
  "ki":…,"mystic":…}` (class-relevant fields).
- **When:** send on change (cache the last-sent values per char; only emit on a
  delta) plus a low-rate heartbeat. Cheap and smooth gauges.
- **Where:** a `gmcp_update_char(ch)` called from the regen/update loop (`update.c`)
  and after damage/heal.
- **Client:** standard `Char.Vitals` → community Mudlet gauge packages, or a ~20-line
  script. Immediate, visible win.

## Phase 2 — `Room.Info` (auto-mapper) — the "wow"

- On room change (after `char_to_room` / movement completes; also on look/recall/
  teleport), send `Room.Info`: `{"num":vnum,"name":"…","area":"…","exits":{"n":vnum,…}}`.
- We already generated the whole room graph for the world maps, so the data model is
  done; the packet is just "the current room."
- Mudlet's built-in mapper consumes GMCP room data → a live map as players walk.

## Phase 3 — Status, commands, completion (closes the original loop)

- `Char.Status` (level, class, gold, primal, xp, align), `Char.StatusVars`.
- A small custom package (e.g. `game.Commands` + context lists like "chant names you
  can use") so a Mudlet script can do the **context-aware tab completion** that
  started this whole thread — now fed accurate, live data from the server.
- Optional later: `Comm.Channel` (route channels to the client), `Char.Items`
  (inventory widget).

---

## Testing strategy

- **Fake Mudlet (Python), like the chant test:** negotiate `IAC DO 201`, send
  `Core.Hello` + `Core.Supports.Set`, then print every `IAC SB 201 … IAC SE` the
  server sends. Verify `Char.Vitals`/`Room.Info` arrive and that ordinary text play
  is byte-identical.
- **Real Mudlet:** connect; Mudlet auto-enables GMCP; use Mudlet's GMCP inspector to
  see packets; load a gauge script.
- **Regression:** raw `telnet` declines GMCP → identical to today.
- **Edge cases:** split packets, `IAC IAC`, unknown subnegotiation, oversized payload.

## Rollout

Ship **Phase 0 + 1 together** (plumbing + gauges) so the hard part lands with a
visible payoff, then Phase 2, then Phase 3 — each its own clean, opt-in deploy
(check "0 players online" as we've been doing).

## Client side (Mudlet)

Mudlet enables GMCP automatically when the server offers `WILL 201`. With standard
package names, gauges/mapper are mostly "install and go." Ship players a small Mudlet
package (gauges + mapper toggle + completion) the same way they got `mpkg`.

---

## File-by-file change summary

| File | Change | Phase |
|---|---|---|
| `src/gmcp.c`, `src/gmcp.h` | **new** — filter, `gmcp_in`, `send_gmcp`, package builders | 0+ |
| `src/merc.h` | DESCRIPTOR_DATA `gmcp`/`gmcp_supports`; `#define TELOPT_GMCP 201`; prototypes | 0 |
| `src/comm.c` | call `telnet_filter` in `read_from_descriptor`; `WILL 201` in `new_descriptor`; clear fields in `close_socket` | 0 |
| `src/Makefile` | add `gmcp.o` to `O_FILES` | 0 |
| `src/update.c` | `gmcp_update_char` hook (vitals) | 1 |
| `src/act_move.c` / `handler.c` | `Room.Info` on room change | 2 |
| (various) | `Char.Status`, completion lists | 3 |

## Open decisions (none blocking)

- Vitals cadence: on-change + heartbeat (recommended).
- Match standard package names for off-the-shelf Mudlet GUIs (recommended).
- Honour `Core.Supports` strictly, or just send everything at first and refine
  (recommend: send everything initially — simpler, harmless).
- GMCP only — skip the older ATCP/MSDP (Mudlet prefers GMCP).

## Effort recap

Phase 0 (the telnet input parser + send/negotiate plumbing) is the single real
chunk — bounded, with reference implementations for Diku/Merc, and fully testable.
Phases 1–3 are each small, additive, and low-risk. The only sensitive surface (the
input path) is gated behind negotiation, so it cannot affect existing play.
