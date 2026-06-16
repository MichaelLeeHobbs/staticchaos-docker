#!/usr/bin/env node
/*
 * build-world-graph.mjs -- emit world-maps/world-graph.json: a directed,
 * A*-ready navigation graph of the entire MUD, for external tools/AI agents
 * to plan paths through the world.
 *
 *   nodes : every room        -> terrain, per-room move cost, navigation
 *           flags, capability requirements (fly/boat), and agent-facing notes.
 *   edges : every traversable link -> normal walk exits AND portal/gate
 *           objects, each with cost, a one-way marker, door/key detail, and a
 *           `note`/`via` text prop an AI can act on.
 *   meta  : the cost model + the RULES an agent must follow to use the graph.
 *
 * All constants below mirror the C source (src/act_move.c, src/merc.h) so the
 * graph reflects how the engine actually moves players.
 *
 * Usage:  node tools/build-world-graph.mjs   (from the project root)
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseAreaFile, loadAreaList, buildWorldIndex, resolveResets, DIRS, DIR_WORD } from './lib/area.mjs';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const AREA_DIR = path.join(ROOT, 'area');
const OUT = path.join(ROOT, 'world-maps');

/* ---- mirrored from src/act_move.c + src/merc.h ---- */
const MOVEMENT_LOSS = [1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6];   // movement_loss[SECT_MAX]
const SECTOR_NAME = ['inside', 'city', 'field', 'forest', 'hills', 'mountain',
  'water_swim', 'water_noswim', 'unused', 'air', 'desert'];
const SECT_AIR = 9, SECT_WATER_NOSWIM = 7;
const EX = { ISDOOR: 1, CLOSED: 2, LOCKED: 4, PICKPROOF: 32 };  // exit_info bits
const ROOM_FLAG_BITS = [
  [1, 'dark'], [4, 'no_mob'], [8, 'indoors'], [512, 'private'], [1024, 'safe'],
  [2048, 'solitary'], [4096, 'pet_shop'], [8192, 'no_recall'], [32768, 'hq'],
  [65536, 'arena'], [131072, 'hanger'], [262144, 'regen'],
];
const ITEM_PORTAL = 27, ITEM_GATE = 28;
const RECALL_VNUM = 3001;

const ml = (s) => MOVEMENT_LOSS[Math.min(MOVEMENT_LOSS.length - 1, Math.max(0, s | 0))];
const stepCost = (a, b) => ml(a) + ml(b);
const decodeFlags = (f) => ROOM_FLAG_BITS.filter(([b]) => (f & b) === b).map(([, n]) => n);

function build() {
  const areas = [];
  for (const f of loadAreaList(AREA_DIR)) {
    const a = parseAreaFile(path.join(AREA_DIR, f));
    if (a.rooms.length) areas.push(a);
  }
  const idx = buildWorldIndex(areas);            // { mobs, rooms, objs } (rooms carry .area/.flags/.exitInfo)
  const { itemSources } = resolveResets(areas);  // objVnum -> Source[] (we use kind:'floor')
  const rooms = idx.rooms;

  /* ---- nodes ---- */
  const nodes = {};
  for (const r of rooms.values()) {
    const flags = decodeFlags(r.flags || 0);
    const node = {
      id: r.vnum,
      name: r.name || '(unnamed)',
      area: r.area,
      sector: r.sector,
      terrain: SECTOR_NAME[r.sector] ?? `sect${r.sector}`,
      enterCost: ml(r.sector),
    };
    if (flags.length) node.flags = flags;

    const requires = [];
    if (r.sector === SECT_AIR) requires.push('fly');
    if (r.sector === SECT_WATER_NOSWIM) requires.push('boat');
    if (requires.length) node.requires = requires;

    const notes = [];
    if (r.sector === SECT_AIR) notes.push('Flight required to enter or leave (fly spell, mobile suit, or Mazoku wings).');
    if (r.sector === SECT_WATER_NOSWIM) notes.push('A boat (or flight) is required to enter or leave.');
    if (flags.includes('no_recall')) notes.push('Cannot recall from here.');
    if (flags.includes('private')) notes.push('Private: entry refused while 2+ are inside.');
    if (flags.includes('solitary')) notes.push('Solitary: entry refused while anyone is inside.');
    if (flags.includes('hq')) notes.push('Headquarters: portals cannot be used to enter or leave.');
    if (flags.includes('hanger')) notes.push('Mobile-suit hangar: mount your clan suit here.');
    if (flags.includes('safe')) notes.push('Safe room: no player-vs-player combat.');
    if (notes.length) node.note = notes.join(' ');

    nodes[r.vnum] = node;
  }

  /* ---- walk edges ---- */
  const edges = [];
  let dangling = 0;
  for (const r of rooms.values()) {
    for (const dir of DIRS) {
      const info = r.exitInfo?.[dir];
      if (!info) continue;
      const to = info.to;
      if (to <= 0 || !rooms.has(to)) { if (to > 0) dangling++; continue; }
      const dest = rooms.get(to);
      const edge = {
        from: r.vnum, to, kind: 'walk',
        dir: DIR_WORD[dir],
        cost: stepCost(r.sector, dest.sector),
      };
      if (!Object.values(dest.exits).includes(r.vnum)) edge.oneWay = true;

      const f = info.flags || 0;
      if (f & EX.ISDOOR) {
        const door = { keyword: info.keyword || '' };
        if (f & EX.CLOSED) door.closed = true;
        if (f & EX.LOCKED) door.locked = true;
        if (f & EX.PICKPROOF) door.pickproof = true;
        if (info.key > 0) {
          door.key = info.key;
          const ko = idx.objs.get(info.key);
          if (ko) door.keyName = ko.name;
        }
        edge.door = door;
      }

      const notes = [];
      if (edge.oneWay) notes.push('One-way: the destination has no exit back.');
      if (edge.door) {
        const d = edge.door;
        let s = `Door${d.keyword ? ` '${d.keyword}'` : ''}`;
        if (d.locked) s += ` (locked${d.key ? `; key: ${d.keyName || '#' + d.key}` : ''})`;
        else if (d.closed) s += ' (closed)';
        s += ' — advisory only; closed/locked doors do not block walking in this engine.';
        notes.push(s);
      }
      const need = [...(nodes[r.vnum].requires || []), ...(nodes[to].requires || [])];
      if (need.length) notes.push(`Requires capability: ${[...new Set(need)].join(', ')}.`);
      if (notes.length) edge.note = notes.join(' ');

      edges.push(edge);
    }
  }

  /* ---- portal / gate edges (static objects lying in a room) ---- */
  const portals = [];
  for (const o of idx.objs.values()) {
    if (o.itemType !== ITEM_PORTAL && o.itemType !== ITEM_GATE) continue;
    const destV = o.values?.[0] || 0;
    if (destV <= 0 || !rooms.has(destV)) continue;        // dead (value0=0) or dynamic portal
    for (const s of (itemSources.get(o.vnum) || [])) {
      if (s.kind !== 'floor' || !rooms.has(s.room)) continue;
      // do_enter forbids portalling out of or into a headquarters
      if (nodes[s.room]?.flags?.includes('hq') || nodes[destV]?.flags?.includes('hq')) continue;
      const kw = (o.keywords || o.name || 'portal').split(/\s+/)[0];
      const kind = o.itemType === ITEM_GATE ? 'gate' : 'portal';
      edges.push({
        from: s.room, to: destV, kind, via: `enter ${kw}`, cost: 1,
        note: `Special: type "enter ${kw}" to step through ${o.name} into room ${destV}. Blocked by a fight timer.`,
      });
      portals.push({ object: o.vnum, name: o.name, type: kind, from: s.room, to: destV, via: `enter ${kw}` });
    }
  }

  return { nodes, edges, portals, dangling };
}

/* ------------------------------------------------------------------ */
function main() {
  fs.mkdirSync(OUT, { recursive: true });
  const { nodes, edges, portals, dangling } = build();
  const walk = edges.filter((e) => e.kind === 'walk').length;

  const graph = {
    generatedBy: 'tools/build-world-graph.mjs',
    generatedFrom: 'area/area.lst',
    meta: {
      directed: true,
      costUnit: 'move points (player movement-point cost)',
      costModel: 'walk edge cost = movement_loss[from.sector] + movement_loss[to.sector]; portal/gate edges cost 1 (a single action)',
      movementLoss: MOVEMENT_LOSS,
      sectorNames: SECTOR_NAME,
      recallVnum: RECALL_VNUM,
      capabilities: {
        fly: 'fly spell / mobile suit / Mazoku wings — needed for "air" rooms',
        boat: 'carry an ITEM_BOAT (flight also works) — needed for "water_noswim" rooms',
      },
      rules: [
        'Graph is directed: each exit is its own edge; check edge.oneWay for asymmetry.',
        'Traverse an edge only if you meet the `requires` of BOTH endpoint nodes (air needs fly; water_noswim needs a boat or flight). edge.note repeats this where it applies.',
        "Doors: edge.door records closed/locked/key, but this engine's closed-door check in move_char is DISABLED, so doors do NOT block walking. Treat door/key info as advisory/flavour.",
        "Portals & gates are edges with kind 'portal'|'gate' and a `via` command (e.g. \"enter nexus\"); they are blocked by a fight timer and cannot be used from/into a headquarters (ROOM_HQ). ITEM_PORTAL is consumed per use but resets, so it is treated as a persistent edge.",
        'recall is NOT an edge: from most rooms a player may `recall` to their home room (default Temple of Midgaard #3001); it is blocked by the no_recall flag, a curse, or while fighting.',
        'Speech/mobprog-triggered transfers (e.g. say a password / "xyzzy") are NOT auto-extracted. If you discover one, hand-annotate it onto the relevant node or edge `note`.',
      ],
      stats: {
        nodes: Object.keys(nodes).length,
        edges: edges.length,
        walkEdges: walk,
        portalEdges: edges.length - walk,
        portalObjects: portals.length,
        danglingExitsSkipped: dangling,
      },
    },
    nodes,
    edges,
    portals,
  };

  fs.writeFileSync(path.join(OUT, 'world-graph.json'), JSON.stringify(graph, null, 2) + '\n');
  console.log(`graph : ${Object.keys(nodes).length} nodes, ${edges.length} edges (${walk} walk, ${edges.length - walk} portal/gate), ${portals.length} portal objs, ${dangling} dangling skipped`);
  console.log('        -> world-maps/world-graph.json');
}
main();
