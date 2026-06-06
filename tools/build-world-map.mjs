#!/usr/bin/env node
/*
 * build-world-map.mjs -- Static Chaos world cartographer.
 *
 * Reads every live area file listed in area/area.lst, parses the #ROOMDATA
 * section of each into a directed graph of rooms (nodes) and exits (edges),
 * and emits:
 *
 *   world.json              -- the canonical graph + analysis reports
 *   WORLD-MAP.md            -- world overview: stats, an area-connectivity
 *                              Mermaid diagram, an area index, and reports
 *                              (orphaned/unreachable areas, dangling exits).
 *   world-maps/<area>.md    -- one Mermaid room graph + room table per area.
 *
 * Diku/Merc area format notes (this fork):
 *   - Sections: #AREADATA (or #AREA), #MOBILES, #OBJECTS, #ROOMDATA (or
 *     #ROOMS), #RESETS, #SHOPS, #SPECIALS, terminated by #$.
 *   - A room: "#<vnum>", name~, description~, then "<area> <flags> <sector>",
 *     then exit/extra blocks, then "S".
 *   - Exit block: "D<n>" (n=0..5 => N E S W Up Down), a description string~, a
 *     keyword string~, then "<locks> <key> <to-vnum>".
 *   - Extra desc block: "E", keyword~, description~.
 *   - Room vnums collide with mob/object vnums, so we ONLY parse rooms inside
 *     the #ROOMDATA section.
 *
 * Usage:  node tools/build-world-map.mjs   (run from the project root)
 */

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const ROOT = path.resolve(__dirname, '..');
const AREA_DIR = path.join(ROOT, 'area');
const OUT_JSON = path.join(ROOT, 'world.json');
const OUT_MD = path.join(ROOT, 'WORLD-MAP.md');
const OUT_MAPS = path.join(ROOT, 'world-maps');

const DIRS = ['N', 'E', 'S', 'W', 'U', 'D'];
const DIR_WORD = { N: 'north', E: 'east', S: 'south', W: 'west', U: 'up', D: 'down' };
const RECALL_VNUM = 3001; // Temple of Midgaard -- the recall point.

/* ------------------------------------------------------------------ */
/* A tiny cursor reader that mimics Diku's fread_string/fread_number.  */
/* ------------------------------------------------------------------ */
class Reader {
  constructor(text) { this.s = text; this.i = 0; }
  eof() { return this.i >= this.s.length; }
  _skipWs() { while (this.i < this.s.length && /\s/.test(this.s[this.i])) this.i++; }
  // Read up to (and consuming) the next '~'.
  readString() {
    this._skipWs();
    let end = this.s.indexOf('~', this.i);
    if (end < 0) end = this.s.length;
    const str = this.s.slice(this.i, end);
    this.i = end + 1;
    return str.replace(/\r/g, '').trim();
  }
  readWord() {
    this._skipWs();
    const start = this.i;
    while (this.i < this.s.length && !/\s/.test(this.s[this.i])) this.i++;
    return this.s.slice(start, this.i);
  }
  // Read a single non-whitespace character (for D/E/S/H/M/C/O letters).
  readLetter() {
    this._skipWs();
    if (this.eof()) return '';
    return this.s[this.i++];
  }
  readNumber() {
    const w = this.readWord();
    const n = parseInt(w, 10);
    return Number.isNaN(n) ? 0 : n;
  }
}

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

// Strip Merc colour codes & control sequences for display.
function clean(s) {
  return (s || '')
    .replace(/`./g, '')        // backtick colour codes: `m `n ...
    .replace(/\{[^}]*\}/g, '') // brace codes: { 5 35} {R} ...
    .replace(/\s+/g, ' ')
    .trim();
}

// Sanitise a string for use inside a Mermaid "..." quoted label.
function mlabel(s) {
  return clean(s)
    .replace(/"/g, "'")
    .replace(/[[\]{}|<>]/g, '')
    .slice(0, 48) || '(unnamed)';
}

// Extract the slice of lines belonging to one named section.
const SECTION_HEADERS = /^#(AREADATA|AREA|MOBILES|OBJECTS|MOBOLD|ROOMDATA|ROOMS|RESETS|SHOPS|SPECIALS|HELPS|SOCIALS|\$)\b/;

function sectionSlice(lines, headerNames) {
  const startIdx = lines.findIndex((l) => headerNames.some((h) => l.trim() === h));
  if (startIdx < 0) return null;
  let endIdx = lines.length;
  for (let i = startIdx + 1; i < lines.length; i++) {
    if (SECTION_HEADERS.test(lines[i].trim())) { endIdx = i; break; }
  }
  return lines.slice(startIdx + 1, endIdx).join('\n');
}

/* ------------------------------------------------------------------ */
/* Parse one area file                                                */
/* ------------------------------------------------------------------ */
function parseArea(file) {
  const id = path.basename(file).replace(/\.are$/i, '');
  const raw = fs.readFileSync(file, 'utf8');
  const lines = raw.split('\n');

  // ---- friendly area name (best-effort, from #AREADATA "Name ...~") ----
  let name = id;
  const areaData = sectionSlice(lines, ['#AREADATA']);
  if (areaData) {
    const m = areaData.match(/^\s*Name\s+([^~]*)~/m);
    if (m) name = clean(m[1]) || id;
  }

  // ---- rooms ----
  const roomBlock = sectionSlice(lines, ['#ROOMDATA', '#ROOMS']);
  const rooms = [];
  if (roomBlock) {
    const r = new Reader(roomBlock);
    while (!r.eof()) {
      const tag = r.readWord();                 // "#<vnum>" or "#0"
      if (!tag.startsWith('#')) break;          // ran off the end
      const vnum = parseInt(tag.slice(1), 10);
      if (!vnum) break;                          // "#0" terminates rooms
      const rname = r.readString();             // name~
      const desc = r.readString();              // description~
      r.readWord(); r.readWord(); const sector = r.readNumber(); // area flags sector
      const exits = {};
      let guard = 0;
      for (;;) {
        if (guard++ > 5000) throw new Error(`runaway room parse near vnum ${vnum} in ${id}`);
        const letter = r.readLetter();
        if (letter === '' || letter === 'S') break;
        if (letter === 'D') {
          const door = r.readNumber();          // 0..5
          r.readString();                        // exit description
          r.readString();                        // exit keyword
          r.readNumber();                        // locks
          r.readNumber();                        // key
          const to = r.readNumber();             // destination vnum
          if (DIRS[door]) exits[DIRS[door]] = to;
        } else if (letter === 'E') {
          r.readString();                        // keyword
          r.readString();                        // description
        } else if (letter === 'H' || letter === 'M') {
          r.readNumber();                        // heal/mana rate (ROM)
        } else if (letter === 'C' || letter === 'O') {
          r.readString();                        // clan / owner (ROM)
        } else {
          throw new Error(`unexpected room token '${letter}' near vnum ${vnum} in ${id}`);
        }
      }
      rooms.push({ vnum, name: clean(rname), sector, exits });
    }
  }
  return { id, file: path.basename(file), name, rooms };
}

/* ------------------------------------------------------------------ */
/* Build the world                                                    */
/* ------------------------------------------------------------------ */
function loadAreaList() {
  const lst = fs.readFileSync(path.join(AREA_DIR, 'area.lst'), 'utf8');
  return lst.split('\n')
    .map((l) => l.trim())
    .filter((l) => /\.are$/i.test(l))
    .filter((l) => fs.existsSync(path.join(AREA_DIR, l)));
}

function build() {
  const files = loadAreaList();
  const areas = [];
  const rooms = new Map();          // vnum -> room
  const vnumToArea = new Map();     // vnum -> area id

  for (const f of files) {
    const a = parseArea(path.join(AREA_DIR, f));
    if (!a.rooms.length) continue;  // help/social files etc.
    const vnums = a.rooms.map((r) => r.vnum);
    areas.push({
      id: a.id, file: a.file, name: a.name,
      roomCount: a.rooms.length,
      vnumMin: Math.min(...vnums), vnumMax: Math.max(...vnums),
    });
    for (const room of a.rooms) {
      room.area = a.id;
      rooms.set(room.vnum, room);
      vnumToArea.set(room.vnum, a.id);
    }
  }

  // ---- edge analysis ----
  const dangling = [];   // exit -> nonexistent room
  const oneWay = [];     // A->B with no B->A
  const crossArea = [];  // A(area X) -> B(area Y), X != Y
  let exitCount = 0;
  const inbound = new Map(); // vnum -> count of inbound edges

  for (const room of rooms.values()) {
    for (const [dir, to] of Object.entries(room.exits)) {
      exitCount++;
      const dest = rooms.get(to);
      if (!dest) { dangling.push({ from: room.vnum, dir, to }); continue; }
      inbound.set(to, (inbound.get(to) || 0) + 1);
      const back = Object.values(dest.exits).includes(room.vnum);
      if (!back) oneWay.push({ from: room.vnum, dir, to });
      if (dest.area !== room.area) {
        crossArea.push({ fromArea: room.area, toArea: dest.area, from: room.vnum, to, dir });
      }
    }
  }

  // rooms nothing points at
  const orphans = [...rooms.values()].filter((r) => !inbound.has(r.vnum)).map((r) => r.vnum);

  // ---- reachability BFS from the recall point ----
  const reachable = new Set();
  if (rooms.has(RECALL_VNUM)) {
    const q = [RECALL_VNUM];
    reachable.add(RECALL_VNUM);
    while (q.length) {
      const v = q.shift();
      const room = rooms.get(v);
      if (!room) continue;
      for (const to of Object.values(room.exits)) {
        if (rooms.has(to) && !reachable.has(to)) { reachable.add(to); q.push(to); }
      }
    }
  }
  const reachableByArea = new Map();
  for (const v of reachable) {
    const a = vnumToArea.get(v);
    reachableByArea.set(a, (reachableByArea.get(a) || 0) + 1);
  }
  const unreachableAreas = areas
    .filter((a) => !(reachableByArea.get(a.id) > 0))
    .map((a) => a.id);

  // inbound cross-area links per area (how the world reaches into it)
  const inboundAreas = new Map(); // area -> Set of source areas
  for (const e of crossArea) {
    if (!inboundAreas.has(e.toArea)) inboundAreas.set(e.toArea, new Set());
    inboundAreas.get(e.toArea).add(e.fromArea);
  }

  const world = {
    generatedFrom: 'area/area.lst',
    recallVnum: RECALL_VNUM,
    stats: {
      areas: areas.length,
      rooms: rooms.size,
      exits: exitCount,
      crossAreaLinks: crossArea.length,
      danglingExits: dangling.length,
      oneWayExits: oneWay.length,
      orphanRooms: orphans.length,
      reachableFromRecall: reachable.size,
      unreachableRooms: rooms.size - reachable.size,
      unreachableAreas: unreachableAreas.length,
    },
    areas: areas.sort((a, b) => a.vnumMin - b.vnumMin),
    rooms: Object.fromEntries([...rooms.entries()]
      .sort((a, b) => a[0] - b[0])
      .map(([v, r]) => [v, { vnum: r.vnum, name: r.name, area: r.area, sector: r.sector, exits: r.exits }])),
    reports: {
      unreachableAreas,
      orphanRooms: orphans.sort((a, b) => a - b),
      danglingExits: dangling,
      oneWayExits: oneWay,
      crossAreaLinks: crossArea,
    },
  };

  return { world, rooms, areas, crossArea, reachable, reachableByArea, inboundAreas };
}

/* ------------------------------------------------------------------ */
/* Render Markdown                                                    */
/* ------------------------------------------------------------------ */
function areaOverviewMermaid(areas, crossArea) {
  // collapse to area->area edges with a count
  const pair = new Map();
  for (const e of crossArea) {
    if (e.fromArea === e.toArea) continue;
    const k = `${e.fromArea}${e.toArea}`;
    pair.set(k, (pair.get(k) || 0) + 1);
  }
  const lines = ['graph LR'];
  for (const a of areas) lines.push(`  ${a.id}["${mlabel(a.name)}"]`);
  for (const [k, count] of pair) {
    const [from, to] = k.split('');
    lines.push(count > 1 ? `  ${from} -->|${count}| ${to}` : `  ${from} --> ${to}`);
  }
  return lines.join('\n');
}

function writeOverview(world, areas, crossArea, reachableByArea, inboundAreas) {
  const s = world.stats;
  const out = [];
  out.push('# Static Chaos -- World Map');
  out.push('');
  out.push('> Generated by `tools/build-world-map.mjs` from `area/area.lst`. Do not edit by hand.');
  out.push('');
  out.push('## At a glance');
  out.push('');
  out.push('| Metric | Value |');
  out.push('|---|---:|');
  out.push(`| Areas | ${s.areas} |`);
  out.push(`| Rooms | ${s.rooms} |`);
  out.push(`| Exits | ${s.exits} |`);
  out.push(`| Cross-area links | ${s.crossAreaLinks} |`);
  out.push(`| Reachable from recall (Temple, #${world.recallVnum}) | ${s.reachableFromRecall} |`);
  out.push(`| **Unreachable rooms** | ${s.unreachableRooms} |`);
  out.push(`| **Unreachable areas** | ${s.unreachableAreas} |`);
  out.push(`| One-way exits | ${s.oneWayExits} |`);
  out.push(`| Dangling exits (broken) | ${s.danglingExits} |`);
  out.push(`| Orphan rooms (no way in) | ${s.orphanRooms} |`);
  out.push('');
  out.push('## World overview (area connectivity)');
  out.push('');
  out.push('Each node is an area; an arrow means at least one room in the source area has an exit into the target area. A number on the arrow is how many such links exist.');
  out.push('');
  out.push('```mermaid');
  out.push(areaOverviewMermaid(areas, crossArea));
  out.push('```');
  out.push('');
  out.push('## Areas');
  out.push('');
  out.push('| Area | Name | Rooms | VNUMs | Reachable from recall? | Reached from |');
  out.push('|---|---|---:|---|:---:|---|');
  for (const a of areas) {
    const reach = reachableByArea.get(a.id) || 0;
    const into = [...(inboundAreas.get(a.id) || [])].sort().join(', ') || '—';
    const flag = reach > 0 ? `✅ ${reach}/${a.roomCount}` : '❌ 0';
    out.push(`| [\`${a.id}\`](world-maps/${a.id}.md) | ${a.name} | ${a.roomCount} | ${a.vnumMin}–${a.vnumMax} | ${flag} | ${into} |`);
  }
  out.push('');

  if (world.reports.unreachableAreas.length) {
    out.push('## ⚠️ Unreachable areas');
    out.push('');
    out.push(`These areas have **no path from the recall point** (Temple of Midgaard, room ${world.recallVnum}) by walking exits. They still exist in the world data, but a mortal cannot reach them on foot — an immortal must \`goto\` a room inside. (This is exactly how the old Apocalypse HQ got stranded.)`);
    out.push('');
    for (const id of world.reports.unreachableAreas) {
      const a = areas.find((x) => x.id === id);
      const into = [...(inboundAreas.get(id) || [])].sort().join(', ');
      out.push(`- **${a.name}** (\`${id}\`, rooms ${a.vnumMin}–${a.vnumMax})${into ? ` — only linked from: ${into}` : ' — no inbound links at all'}`);
    }
    out.push('');
  }

  if (world.reports.danglingExits.length) {
    out.push('## Dangling exits (point at rooms that do not exist)');
    out.push('');
    out.push('| From room | Dir | Target vnum (missing) |');
    out.push('|---:|:--:|---:|');
    for (const e of world.reports.danglingExits.slice(0, 200)) {
      out.push(`| ${e.from} | ${e.dir} | ${e.to} |`);
    }
    if (world.reports.danglingExits.length > 200) out.push(`| … | | ${world.reports.danglingExits.length - 200} more |`);
    out.push('');
  }
  fs.writeFileSync(OUT_MD, out.join('\n') + '\n');
}

function areaMermaid(area, rooms) {
  const inArea = [...rooms.values()].filter((r) => r.area === area.id);
  const lines = ['graph LR'];
  for (const r of inArea) lines.push(`  R${r.vnum}["${mlabel(r.name)}<br/>#${r.vnum}"]`);
  const extNodes = new Map(); // vnum -> label, for out-of-area destinations
  for (const r of inArea) {
    for (const [dir, to] of Object.entries(r.exits)) {
      const dest = rooms.get(to);
      if (dest && dest.area === area.id) {
        lines.push(`  R${r.vnum} -->|${dir}| R${to}`);
      } else {
        // exit leaves the area (or is broken)
        const label = dest ? `${mlabel(dest.name)}<br/>${dest.area} #${to}` : `?? broken<br/>#${to}`;
        if (!extNodes.has(to)) extNodes.set(to, label);
        lines.push(`  R${r.vnum} -->|${dir}| X${to}`);
      }
    }
  }
  for (const [to, label] of extNodes) lines.push(`  X${to}["${label}"]:::ext`);
  lines.push('  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;');
  return lines.join('\n');
}

function writeAreaPages(areas, rooms) {
  fs.mkdirSync(OUT_MAPS, { recursive: true });
  for (const a of areas) {
    const inArea = [...rooms.values()].filter((r) => r.area === a.id).sort((x, y) => x.vnum - y.vnum);
    const out = [];
    out.push(`# ${a.name}  \`(${a.id})\``);
    out.push('');
    out.push(`[← back to world map](../WORLD-MAP.md) · ${inArea.length} rooms · vnums ${a.vnumMin}–${a.vnumMax}`);
    out.push('');
    out.push('Dashed nodes are exits that leave this area.');
    out.push('');
    out.push('```mermaid');
    out.push(areaMermaid(a, rooms));
    out.push('```');
    out.push('');
    out.push('## Rooms');
    out.push('');
    out.push('| VNUM | Name | Exits |');
    out.push('|---:|---|---|');
    for (const r of inArea) {
      const ex = Object.entries(r.exits)
        .map(([d, to]) => `${d}→${to}`)
        .join(' ') || '—';
      out.push(`| ${r.vnum} | ${r.name || '(unnamed)'} | ${ex} |`);
    }
    out.push('');
    fs.writeFileSync(path.join(OUT_MAPS, `${a.id}.md`), out.join('\n') + '\n');
  }
}

/* ------------------------------------------------------------------ */
function main() {
  const { world, rooms, areas, crossArea, reachableByArea, inboundAreas } = build();
  fs.writeFileSync(OUT_JSON, JSON.stringify(world, null, 2) + '\n');
  writeOverview(world, areas, crossArea, reachableByArea, inboundAreas);
  writeAreaPages(areas, rooms);

  const s = world.stats;
  console.log(`Parsed ${s.areas} areas, ${s.rooms} rooms, ${s.exits} exits.`);
  console.log(`  cross-area links : ${s.crossAreaLinks}`);
  console.log(`  reachable/recall : ${s.reachableFromRecall}  (unreachable: ${s.unreachableRooms} rooms, ${s.unreachableAreas} areas)`);
  console.log(`  dangling exits   : ${s.danglingExits}`);
  console.log(`  orphan rooms     : ${s.orphanRooms}`);
  console.log(`Wrote: world.json, WORLD-MAP.md, world-maps/<area>.md`);
  if (world.reports.unreachableAreas.length) {
    console.log(`  unreachable areas: ${world.reports.unreachableAreas.join(', ')}`);
  }
}

main();
