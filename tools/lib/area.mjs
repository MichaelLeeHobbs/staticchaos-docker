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

/* ---- one area file -> { id, file, name, rooms, mobiles, objects } ---- */
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
