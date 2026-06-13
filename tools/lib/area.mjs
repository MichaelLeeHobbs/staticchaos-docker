/*
 * area.mjs -- shared parser for Static Chaos / Merc-derived .are files.
 *
 * Exposes a tolerant reader plus parsers for the #ROOMDATA, #MOBILES and
 * #OBJECTS sections, matching how src/db.c loads them:
 *
 *   Room   : name~ desc~ "<area> <flags> <sector>" then D<n>/E blocks, "S".
 *   Mobile : keywords~ short~ long~ desc~ "<act> <aff> <clan> S <level> ..."
 *            (this fork stores clan, not alignment; mob progs may follow).
 *   Object : keywords~ short~ desc~ action~ "<type> <extra> <wear>"
 *            value0..3, weight cost perday, then repeating A/E blocks.
 *
 * Room/mob/object vnums overlap, so each section is parsed in isolation.
 */

import fs from 'node:fs';
import path from 'node:path';

export const DIRS = ['N', 'E', 'S', 'W', 'U', 'D'];
export const DIR_WORD = { N: 'north', E: 'east', S: 'south', W: 'west', U: 'up', D: 'down' };

/* ---- a cursor reader mimicking Diku fread_string/fread_number ---- */
export class Reader {
  constructor(text) { this.s = text; this.i = 0; }
  eof() { return this.i >= this.s.length; }
  _skipWs() { while (this.i < this.s.length && /\s/.test(this.s[this.i])) this.i++; }
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
  readLetter() { this._skipWs(); return this.eof() ? '' : this.s[this.i++]; }
  readNumber() { const n = parseInt(this.readWord(), 10); return Number.isNaN(n) ? 0 : n; }
}

/* ---- text helpers ---- */
export function clean(s) {
  return (s || '')
    .replace(/`./g, '')        // backtick colour codes
    .replace(/\{[^}]*\}/g, '') // brace colour codes
    .replace(/\s+/g, ' ')
    .trim();
}
export function mlabel(s) {
  return clean(s).replace(/"/g, "'").replace(/[[\]{}|<>]/g, '').slice(0, 48) || '(unnamed)';
}
// escape a markdown table cell
export function cell(s) {
  return clean(s).replace(/\|/g, '\\|').replace(/\r?\n/g, ' ') || '';
}

const SECTION_HEADERS =
  /^#(AREADATA|AREA|MOBILES|OBJECTS|MOBOLD|ROOMDATA|ROOMS|RESETS|SHOPS|SPECIALS|HELPS|SOCIALS|\$)\b/;

export function sectionSlice(lines, headerNames) {
  const start = lines.findIndex((l) => headerNames.some((h) => l.trim() === h));
  if (start < 0) return null;
  let end = lines.length;
  for (let i = start + 1; i < lines.length; i++) {
    if (SECTION_HEADERS.test(lines[i].trim())) { end = i; break; }
  }
  return lines.slice(start + 1, end).join('\n');
}

// Split a section into per-vnum blocks (stops at "#0"). Robust against mob
// programs / extra descriptions, which never begin a line with "#<digits>".
function splitBlocks(text) {
  const out = [];
  let cur = null;
  for (const line of text.split('\n')) {
    const m = line.match(/^#(-?\d+)\s*$/);
    if (m) {
      if (cur) out.push(cur);
      const vnum = parseInt(m[1], 10);
      if (vnum === 0) { cur = null; break; }
      cur = { vnum, lines: [] };
    } else if (cur) {
      cur.lines.push(line);
    }
  }
  if (cur) out.push(cur);
  return out.map((b) => ({ vnum: b.vnum, body: b.lines.join('\n') }));
}

/* ---- rooms (parsed sequentially; structure is regular) ---- */
function parseRooms(block) {
  const rooms = [];
  if (!block) return rooms;
  const r = new Reader(block);
  while (!r.eof()) {
    const tag = r.readWord();
    if (!tag.startsWith('#')) break;
    const vnum = parseInt(tag.slice(1), 10);
    if (!vnum) break;
    const name = r.readString();
    r.readString();                                  // description
    r.readWord(); r.readWord(); const sector = r.readNumber();
    const exits = {};
    let guard = 0;
    for (;;) {
      if (guard++ > 5000) throw new Error(`runaway room parse at vnum ${vnum}`);
      const letter = r.readLetter();
      if (letter === '' || letter === 'S') break;
      if (letter === 'D') {
        const door = r.readNumber();
        r.readString(); r.readString();
        r.readNumber(); r.readNumber();
        const to = r.readNumber();
        if (DIRS[door]) exits[DIRS[door]] = to;
      } else if (letter === 'E') {
        r.readString(); r.readString();
      } else if (letter === 'H' || letter === 'M') {
        r.readNumber();
      } else if (letter === 'C' || letter === 'O') {
        r.readString();
      } else {
        throw new Error(`unexpected room token '${letter}' at vnum ${vnum}`);
      }
    }
    rooms.push({ vnum, name: clean(name), sector, exits });
  }
  return rooms;
}

/* ---- mobiles (head of each block; mob progs ignored) ---- */
function parseMobiles(block) {
  if (!block) return [];
  return splitBlocks(block).map(({ vnum, body }) => {
    const r = new Reader(body);
    const keywords = r.readString();
    const short = r.readString();
    const long = r.readString();
    const desc = r.readString();
    const act = r.readNumber();
    r.readNumber();              // affected_by
    r.readNumber();              // clan
    r.readLetter();              // 'S'
    const level = r.readNumber();
    return { vnum, keywords: clean(keywords), name: clean(short), long: clean(long), desc: clean(desc), act, level };
  });
}

/* ---- objects (full block: type/flags/values/affects) ---- */
function parseObjects(block) {
  if (!block) return [];
  return splitBlocks(block).map(({ vnum, body }) => {
    const r = new Reader(body);
    const keywords = r.readString();
    const short = r.readString();
    const desc = r.readString();
    r.readString();                                  // action description
    const itemType = r.readNumber();
    const extra = r.readNumber();
    const wear = r.readNumber();
    const values = [r.readNumber(), r.readNumber(), r.readNumber(), r.readNumber()];
    const weight = r.readNumber();
    const cost = r.readNumber();
    r.readNumber();                                  // cost per day
    const affects = [];
    let guard = 0;
    for (;;) {
      if (guard++ > 5000) break;
      const letter = r.readLetter();
      if (letter === 'A') { const loc = r.readNumber(); const mod = r.readNumber(); affects.push({ loc, mod }); }
      else if (letter === 'E') { r.readString(); r.readString(); }
      else break;
    }
    return { vnum, keywords: clean(keywords), name: clean(short), desc: clean(desc), itemType, extra, wear, values, weight, cost, affects };
  });
}

/* ---- resets (M/O/P/G/E/D/R; see db.c load_resets) ----
 * Line shape: "<cmd> <if_flag> <arg1> <arg2> [<arg3>]" (arg3 omitted for G/R).
 *   M arg1=mob  arg2=limit arg3=room     O arg1=obj arg3=room
 *   G arg1=obj (to last mob)             E arg1=obj arg3=wear-loc (on last mob)
 *   P arg1=obj (into last container)     D/R door/randomise (ignored here)
 */
function parseResets(block) {
  if (!block) return [];
  const out = [];
  for (const raw of block.split('\n')) {
    const line = raw.trim();
    if (!line || line.startsWith('*')) continue;
    const cmd = line[0];
    if (cmd === 'S') break;
    if (!'MOPGEDR'.includes(cmd)) continue;
    const n = line.slice(1).trim().split(/\s+/).map((x) => parseInt(x, 10));
    const [, arg1, arg2, arg3] = n;            // n[0] = if_flag
    out.push({ command: cmd, arg1: arg1 || 0, arg2: arg2 || 0, arg3: (cmd === 'G' || cmd === 'R') ? 0 : (arg3 || 0) });
  }
  return out;
}

/* ---- shops (see db.c load_shops): keeper, 5 buy-types, profit buy/sell, hours ---- */
function parseShops(block) {
  if (!block) return [];
  const out = [];
  for (const raw of block.split('\n')) {
    const line = raw.trim();
    if (!line || line.startsWith('*')) continue;
    const n = line.split(/\s+/).map((x) => parseInt(x, 10));
    if (!n.length || Number.isNaN(n[0])) continue;
    const keeper = n[0];
    if (keeper === 0) break;
    out.push({ keeper, buyTypes: n.slice(1, 6).filter(Boolean), profitBuy: n[6] || 0, profitSell: n[7] || 0, openHour: n[8] || 0, closeHour: n[9] || 0 });
  }
  return out;
}

/* ---- one area file -> { id, file, name, rooms, mobiles, objects, resets, shops } ---- */
export function parseAreaFile(file) {
  const id = path.basename(file).replace(/\.are$/i, '');
  const lines = fs.readFileSync(file, 'utf8').split('\n');
  let name = id;
  const areaData = sectionSlice(lines, ['#AREADATA']);
  if (areaData) {
    const m = areaData.match(/^\s*Name\s+([^~]*)~/m);
    if (m) name = clean(m[1]) || id;
  }
  return {
    id, file: path.basename(file), name,
    rooms: parseRooms(sectionSlice(lines, ['#ROOMDATA', '#ROOMS'])),
    mobiles: parseMobiles(sectionSlice(lines, ['#MOBILES'])),
    objects: parseObjects(sectionSlice(lines, ['#OBJECTS'])),
    resets: parseResets(sectionSlice(lines, ['#RESETS'])),
    shops: parseShops(sectionSlice(lines, ['#SHOPS'])),
  };
}

export function loadAreaList(areaDir) {
  return fs.readFileSync(path.join(areaDir, 'area.lst'), 'utf8')
    .split('\n').map((l) => l.trim())
    .filter((l) => /\.are$/i.test(l))
    .filter((l) => fs.existsSync(path.join(areaDir, l)));
}

/* ---- flag / constant decoders (from src/merc.h) ---- */
export const ITEM_TYPE = {
  1: 'Light', 2: 'Scroll', 3: 'Wand', 4: 'Staff', 5: 'Weapon', 8: 'Treasure',
  9: 'Armor', 10: 'Potion', 12: 'Furniture', 13: 'Trash', 15: 'Container',
  17: 'Drink Con', 18: 'Key', 19: 'Food', 20: 'Money', 22: 'Boat',
  23: 'NPC Corpse', 24: 'PC Corpse', 25: 'Fountain', 26: 'Pill', 27: 'Portal',
  28: 'Gate', 29: 'Suit', 30: 'Munition', 31: 'Accessory', 32: 'Materia',
};
const WEAR_BITS = [
  [2, 'finger'], [4, 'neck'], [8, 'body'], [16, 'head'], [32, 'legs'],
  [64, 'feet'], [128, 'hands'], [256, 'arms'], [512, 'shield'], [1024, 'about'],
  [2048, 'waist'], [4096, 'wrist'], [8192, 'wielded'], [16384, 'held'],
];
const EXTRA_BITS = [
  [1, 'glow'], [2, 'hum'], [64, 'magic'], [128, 'nodrop'], [256, 'bless'],
  [512, 'anti-good'], [1024, 'anti-evil'], [2048, 'anti-neutral'],
  [4096, 'noremove'], [32768, 'unique'], [131072, 'noloot'], [262144, 'hardened'],
];
const ACT_BITS = [
  [2, 'sentinel'], [4, 'scavenger'], [16, 'aggro-all'], [32, 'aggressive'],
  [64, 'stay-area'], [128, 'wimpy'], [512, 'trainer'], [1024, 'practice'],
  [131072, 'no-ranged'], [262144, 'finisher'],
];
const APPLY = {
  1: 'Str', 2: 'Dex', 3: 'Int', 4: 'Wis', 5: 'Con', 6: 'Sex', 7: 'Class',
  8: 'Level', 9: 'Age', 10: 'Height', 11: 'Weight', 12: 'Mana', 13: 'HP',
  14: 'Move', 15: 'Gold', 16: 'XP', 17: 'AC', 18: 'Hitroll', 19: 'Damroll',
  20: 'Save-Para', 21: 'Save-Rod', 22: 'Save-Petri', 23: 'Save-Breath', 24: 'Save-Spell',
};
const decodeBits = (flags, table) => table.filter(([bit]) => flags & bit).map(([, n]) => n);

export const itemTypeName = (t) => ITEM_TYPE[t] || `type${t}`;
export const wearList = (w) => decodeBits(w, WEAR_BITS);
export const extraList = (e) => decodeBits(e, EXTRA_BITS);
export const actList = (a) => decodeBits(a, ACT_BITS);
export const applyName = (loc) => APPLY[loc] || `apply${loc}`;
export const affectStr = (a) => `${a.mod >= 0 ? '+' : ''}${a.mod} ${applyName(a.loc)}`;

/* ------------------------------------------------------------------ */
/* Reset cross-reference: where mobs spawn and where items come from   */
/* ------------------------------------------------------------------ */

/* Global vnum indexes across all parsed areas. Mob/room/object vnums are each
 * globally unique, so flat maps are correct. Pass an array of parseAreaFile()
 * results. */
export function buildWorldIndex(areas) {
  const mobs = new Map(), rooms = new Map(), objs = new Map();
  for (const a of areas) {
    for (const m of a.mobiles) mobs.set(m.vnum, { ...m, area: a.id });
    for (const r of a.rooms)   rooms.set(r.vnum, { ...r, area: a.id });
    for (const o of a.objects) objs.set(o.vnum, { ...o, area: a.id });
  }
  return { mobs, rooms, objs };
}

/* Walk every area's #RESETS the way src/db.c reset_room() does, returning:
 *   mobSpawns   : Map<mobVnum, Set<roomVnum>>
 *   itemSources : Map<objVnum, Source[]>
 * where Source is one of:
 *   { kind:'floor',     room }              -- O: lying in a room
 *   { kind:'equipped',  mob, room, wear }   -- E: worn by the last-loaded mob
 *   { kind:'carried',   mob, room }         -- G: carried by the last-loaded mob
 *   { kind:'container', container }         -- P: put inside object <arg3>
 *
 * State is per-area. E/G attach to the most recent M (LastMob and the room it
 * loaded in). P nests arg1 inside the object named by arg3 (the container's own
 * vnum) -- it does NOT belong to "the last room", which is the long-standing
 * mistake this centralises away. Items that never appear as an O/E/G/P arg1
 * are simply absent from itemSources -> not placed -> unobtainable in play. */
export function resolveResets(areas) {
  const mobSpawns = new Map();
  const itemSources = new Map();
  const addSpawn = (mob, room) => { (mobSpawns.get(mob) ?? mobSpawns.set(mob, new Set()).get(mob)).add(room); };
  const addSrc = (obj, s) => { (itemSources.get(obj) ?? itemSources.set(obj, []).get(obj)).push(s); };
  for (const a of areas) {
    let lastMob = 0, lastMobRoom = 0;
    for (const r of a.resets) {
      switch (r.command) {
        case 'M': lastMob = r.arg1; lastMobRoom = r.arg3; addSpawn(r.arg1, r.arg3); break;
        case 'O': addSrc(r.arg1, { kind: 'floor', room: r.arg3 }); break;
        case 'E': addSrc(r.arg1, { kind: 'equipped', mob: lastMob, room: lastMobRoom, wear: r.arg3 }); break;
        case 'G': addSrc(r.arg1, { kind: 'carried', mob: lastMob, room: lastMobRoom }); break;
        case 'P': addSrc(r.arg1, { kind: 'container', container: r.arg3 }); break;
        default: break;
      }
    }
  }
  return { mobSpawns, itemSources };
}

export const WEARLOC = [
  'light', 'left finger', 'right finger', 'neck', 'neck', 'body', 'head', 'legs',
  'feet', 'hands', 'arms', 'shield', 'about body', 'waist', 'left wrist',
  'right wrist', 'wielded', 'held',
];

/* Human description of one item Source. `idx` is buildWorldIndex() output;
 * `itemSources` lets us resolve where a container itself lives (one level
 * deep, so chained P resets don't loop forever). */
export function describeSource(s, idx, itemSources, depth = 0) {
  const roomStr = (v) => { const r = idx.rooms.get(v); return r ? `${r.name} (#${v}, ${r.area})` : `room #${v}`; };
  const mobStr  = (v) => { const m = idx.mobs.get(v);  return m ? `${m.name} (L${m.level})` : `a creature (#${v})`; };
  switch (s.kind) {
    case 'floor':    return `on the ground in ${roomStr(s.room)}`;
    case 'equipped': return `worn (${WEARLOC[s.wear] || 'slot ' + s.wear}) by ${mobStr(s.mob)} in ${roomStr(s.room)}`;
    case 'carried':  return `carried by ${mobStr(s.mob)} in ${roomStr(s.room)}`;
    case 'container': {
      const c = idx.objs.get(s.container);
      const name = c ? `${c.name} (#${s.container})` : `container #${s.container}`;
      let where = '';
      if (depth < 2) {
        const cs = (itemSources.get(s.container) || []).find((x) => x.kind !== 'container');
        if (cs) where = ` — ${describeSource(cs, idx, itemSources, depth + 1)}`;
      }
      return `inside ${name}${where}`;
    }
    default: return '?';
  }
}

/* ------------------------------------------------------------------ */
/* Area map partitioning + Mermaid generation                         */
/* ------------------------------------------------------------------ */

const LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
export function partLetter(i) {
  return i < 26 ? LETTERS[i] : LETTERS[Math.floor(i / 26) - 1] + LETTERS[i % 26];
}

/*
 * Split an area's rooms into connected sub-maps of at most `maxSize` rooms.
 * Areas at/under `threshold` stay whole. Each part is grown best-first (by
 * vnum) from the lowest-vnum unplaced room over the *undirected* exit graph,
 * so parts are connected, vnum-local neighbourhoods. Parts are returned as
 * arrays of room objects, each sorted by vnum, ordered by their lowest vnum.
 */
export function partitionArea(roomsInArea, { maxSize = 24, threshold = 30 } = {}) {
  if (roomsInArea.length <= threshold) return [[...roomsInArea].sort((a, b) => a.vnum - b.vnum)];
  const byVnum = new Map(roomsInArea.map((r) => [r.vnum, r]));
  const inArea = new Set(byVnum.keys());
  const adj = new Map([...inArea].map((v) => [v, new Set()]));
  for (const r of roomsInArea) for (const to of Object.values(r.exits)) {
    if (inArea.has(to)) { adj.get(r.vnum).add(to); adj.get(to).add(r.vnum); }   // undirected
  }
  const unplaced = new Set([...inArea].sort((a, b) => a - b));
  const parts = [];
  while (unplaced.size) {
    const seed = unplaced.values().next().value;          // lowest remaining vnum
    const part = [];
    const frontier = new Set([seed]);
    while (frontier.size && part.length < maxSize) {
      const v = Math.min(...frontier);                    // best-first by vnum -> compact, local
      frontier.delete(v);
      if (!unplaced.has(v)) continue;
      unplaced.delete(v); part.push(v);
      for (const n of adj.get(v)) if (unplaced.has(n)) frontier.add(n);
    }
    parts.push(part.map((v) => byVnum.get(v)));
  }
  // Coalesce tiny fragments (isolated rooms / dead-ends with no in-area exits)
  // into shared bins so we don't emit one-room sub-maps. Real slices (>= MIN)
  // are left intact. Bins are filled first-fit-decreasing up to maxSize.
  const MIN = 6;
  const keep = parts.filter((p) => p.length >= MIN);
  const bins = [];
  for (const p of parts.filter((p) => p.length < MIN).sort((a, b) => b.length - a.length)) {
    let bin = bins.find((b) => b.length + p.length <= maxSize);
    if (!bin) { bin = []; bins.push(bin); }
    bin.push(...p);
  }
  return [...keep, ...bins]
    .map((p) => p.sort((a, b) => a.vnum - b.vnum))
    .sort((a, b) => a[0].vnum - b[0].vnum);
}

/*
 * Build the Mermaid diagram(s) for one area. Returns [{ letter, rooms,
 * mermaid }] -- one entry if the area is small, several if it was split.
 * `allRooms` is the global vnum->room map (rooms carry `.area`), used to
 * label exits that leave the part: a sibling part (▸ Part X) or another area.
 */
export function areaDiagrams(areaId, roomsInArea, allRooms, opts = {}) {
  const parts = partitionArea(roomsInArea, opts);
  const multi = parts.length > 1;
  const partOf = new Map();
  parts.forEach((p, i) => p.forEach((r) => partOf.set(r.vnum, i)));
  return parts.map((part, pi) => {
    const inPart = new Set(part.map((r) => r.vnum));
    const lines = ['graph LR'];
    for (const r of part) lines.push(`  R${r.vnum}["${mlabel(r.name)}<br/>#${r.vnum}"]`);
    const ext = new Map();
    for (const r of part) for (const [dir, to] of Object.entries(r.exits)) {
      if (inPart.has(to)) { lines.push(`  R${r.vnum} -->|${dir}| R${to}`); continue; }
      const dest = allRooms.get(to);
      if (dest && dest.area === areaId) ext.set(to, { label: `▸ Part ${partLetter(partOf.get(to))}: ${mlabel(dest.name)}<br/>#${to}`, cls: 'part' });
      else if (dest) ext.set(to, { label: `${mlabel(dest.name)}<br/>${dest.area} #${to}`, cls: 'ext' });
      else ext.set(to, { label: `?? broken<br/>#${to}`, cls: 'ext' });
      lines.push(`  R${r.vnum} -->|${dir}| X${to}`);
    }
    for (const [to, { label, cls }] of ext) lines.push(`  X${to}["${label}"]:::${cls}`);
    lines.push('  classDef ext fill:#222,stroke:#888,color:#bbb,stroke-dasharray:3 3;');
    lines.push('  classDef part fill:#16313a,stroke:#5aa,color:#bfe,stroke-dasharray:4 2;');
    return { letter: multi ? partLetter(pi) : '', rooms: part, mermaid: lines.join('\n') };
  });
}

// human description of an item's value[] block, by type
export function itemValueStr(o) {
  const v = o.values;
  switch (o.itemType) {
    case 5: { // weapon: v1 dice, v2 size
      if (v[1] && v[2]) return `dmg ${v[1]}d${v[2]} (avg ${Math.round(v[1] * (v[2] + 1) / 2)})`;
      return '';
    }
    case 9: return v[0] ? `AC ${v[0]}` : '';                     // armor
    case 1: return v[2] ? `${v[2] < 0 ? 'infinite' : v[2] + 'h'} light` : ''; // light
    case 15: return v[0] ? `holds ${v[0]}` : '';                 // container
    case 3: case 4: return v[2] ? `${v[2]} charges` : '';        // wand/staff
    default: {
      const nz = v.filter((x) => x);
      return nz.length ? `val ${v.join('/')}` : '';
    }
  }
}
