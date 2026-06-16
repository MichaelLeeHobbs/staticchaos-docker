# Computer Players — Feasibility & Approaches

*Internal design / feasibility note. Not published; not part of the website content sync.*

## Introduction

This document evaluates adding **computer players** — AI/bot-controlled characters — to the Static Chaos MUD (a Dockerized Merc/Diku C codebase). "Computer player" here means a character that plays the game on its own: it moves, fights, casts, heals, flees, and otherwise behaves like a participant rather than a static room decoration.

The headline finding is that **the engine is already most of the way there.** The command layer, immortal puppeteering, mob AI, scripted mob behavior, and — crucially — a structured GMCP state stream and a `world.json` room graph all already exist. A computer player is, at bottom, the same thing an immortal does with `do_switch`: issue ordinary player commands through `interpret()`. The only thing missing is the automation layer that decides *which* command to issue and *when*.

## What already exists (verified)

The following building blocks are already in the codebase. They are why this is a feasible feature rather than a rewrite.

- **`interpret()` — the universal command dispatcher.** `src/interp.c:4111` (`void interpret( CHAR_DATA *ch, char *argument )`). Its own comment reads *"The main entry point for executing commands. Can be recursively called from 'at', 'order', 'force'."* It runs commands for **any** `CHAR_DATA`, player or NPC. Nothing in it requires a human socket. This is the single most important fact: any code that can call `interpret(bot, "command")` can drive a character.

- **`do_switch` — proof an NPC can run the full player command set.** `src/act_wiz.c:1259`. An immortal "switches into" a mobile and then plays as it — move, kill, cast, wear, flee, recall, the lot. A computer player is exactly this, automated: code calling `interpret()` on a timer instead of a human typing into a switched session.

- **`do_force` — forces a character to run a command.** `src/act_wiz.c:3248`. Another existing path that pushes a command string through `interpret()` on behalf of a target character.

- **Mob AI infrastructure already in the world:**
  - `mobile_update` in `src/update.c` — the per-pulse mob behavior tick.
  - ROM-style **MobPrograms** — `src/mob_prog.c` + `src/mob_commands.c` — scripted mob behavior (triggers and reactions).
  - **Special procedures** — `src/special.c` — hard-coded per-mob behaviors.

- **GMCP structured state stream.** `src/gmcp.c` already emits machine-readable JSON: `Char.Vitals`, `Char.Status`, `Room.Info` (room number/name/area/exits), `game.Commands`, `game.Chants`, `Client.GUI`. This is a major enabler for external bots — they can **read structured JSON instead of scraping ANSI text.**

- **A world graph already exists.** `tools/build-world-map.mjs` generates a `world.json` room/exit graph used by the web map. It is reusable as a navigation substrate for bots. (See also the forthcoming `world-graph.json`, below.)

## Three approaches, by effort

### A) Beefed-up mobs / MobPrograms — *easiest (days)*

Use the existing mob infrastructure (`mobile_update`, MobPrograms, special procs) to build mobs that wander, greet players, fight, flee at low HP, and cast. This is pure scripted reaction — these remain NPCs with no class/gear/level progression like a real player has.

- **Good for:** world ambiance, sparring dummies, lively-feeling rooms.
- **Cost / risk:** zero architecture change, low risk, lands in days.
- **Limits:** not real strategy, and not "real players" — they don't live in the world the way a leveled character does.

### B) In-process C bot brain — *moderate (~1–2 weeks for a basic one)*

A controller hooked into the pulse/update loop that, each tick, inspects a bot's state and calls `interpret()` with chosen commands — **exactly what `do_switch` does, but automated** by a state machine or behavior tree.

- **Hard parts:**
  - Pathfinding over the room/exit graph (BFS / Dijkstra / A* — the graph algorithm is already understood from the web-map work).
  - Per-class combat logic — Saiyan, Sorcerer, Fist, Patryn, and Mazoku each play very differently.
  - `pcdata` plumbing if the bots are to be **persistent, leveled, geared characters** rather than mobs.
- **Best when:** you want NPCs that genuinely live in the world as residents.
- **Risk:** higher — the bot code runs **inside the live C server**, so a bot bug can crash the game for everyone.

### C) External GMCP bots — *moderate, and the recommended fit (~1–2 weeks in Node/TS)*

A separate Node/TypeScript program connects to **port 4000** as a normal client, creates a character, and plays the game like any human would.

This is the sweet spot precisely because of what already exists:

- Thanks to GMCP, the bot reads structured `Room.Info` / `Char.Vitals` / `game.Commands` **instead of scraping text.**
- It can reuse `world.json` (and the new `world-graph.json`) for navigation.
- **Zero C changes.** Fully decoupled — a bot bug cannot crash the server.
- Scales by spawning more processes.
- The bots are **genuine players:** they pick a class at creation, level up, gear, and train/pilot skills using the same paths a human uses.

Suggested progression: start with a **behavior tree** (wander → fight → flee/heal when hurt → recall when in danger), then optionally swap the decision function for an **LLM brain** (feed GMCP state in, get the next command out) — which fits the owner's Node / agent-harness background well.

- **Downsides:** each bot consumes a socket / player slot; LLM latency and cost if used; pathfinding is still needed (mitigated — the graph data already exists as JSON).

## Effort estimate

| Approach | What you get | Effort | Risk |
|---|---|---|---|
| A — Beefed-up mobs / MobPrograms | Scripted NPC ambiance, sparring dummies | Days | Low — no architecture change |
| B — In-process C bot brain | NPCs that live in the world as residents | ~1–2 weeks (basic); more if persistent/leveled | Higher — runs inside the live C server |
| C — External GMCP bots *(recommended)* | Genuine players (class, level, gear) | ~1–2 weeks in Node/TS | Low — decoupled, can't crash server |

Effort snapshot:

- **One dumb wandering/fighting external bot:** a day or two.
- **Polished multi-bot system** with class rotations + optional LLM brain: a few weeks.
- **In-process persistent leveled C bots:** more, due to `pcdata` plumbing + C-side pathfinding.

## The hard parts (true in any approach)

These are the real work — *not* the plumbing, which already exists:

- **Pathfinding / navigation.** Moving sensibly from A to B across the room graph. *Mitigated:* graph data already exists as JSON.
- **Class-aware combat decisions.** The genuine content work — what to do, per class (Saiyan / Sorcerer / Fist / Patryn / Mazoku each differ).
- **Not looking dumb.** Anti-stuck logic, flee thresholds, regen/downtime handling, target selection.
- **Persistence decision.** Decide whether bots are persistent, leveled characters or ephemeral throwaways. This choice drives a lot of the surrounding work (especially for Approach B).

## Recommendation

**Approach C — external GMCP bots.**

Given the existing GMCP layer, the `world.json` / `world-graph.json` graph, and the owner's Node/TS + agent-harness background, this is the **lowest-risk, highest-leverage** path. It reuses almost everything already built, requires zero C changes, cannot crash the server, scales by spawning processes, and produces bots that are genuine players rather than dressed-up mobs.

## Next steps

1. **Build a minimal Option-C Node client as a proof of concept:** log in, create a character, read GMCP state, and run a basic behavior tree — wander → fight → flee → heal.
2. **Layer class-aware combat** on top of the working loop, one class at a time.
3. **Optionally swap the decision function for an LLM brain** — feed GMCP state in, get the next command out.
4. **Use the forthcoming `world-graph.json` as the navigation substrate** for the bot's A* pathfinding: rooms as nodes, directional exits as edges carrying cost / restrictions and agent annotations.
