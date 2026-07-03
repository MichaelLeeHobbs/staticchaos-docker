#!/usr/bin/env node
/*
 * test-generators.mjs -- output-contract tests for the tools/ data generators (#70).
 *
 * The generators (build-world-map, build-items, build-bestiary, build-spawns,
 * build-shops, build-classes, build-world-graph, ...) transform area.mjs's parse
 * into world-maps/. area.mjs itself is golden-tested (#57); this guards the
 * generators' OUTPUT contract: valid JSON, non-empty, required fields present,
 * unique keys, expected classes. These are invariants that hold regardless of the
 * area data, so they stay stable across content edits while still catching the real
 * regression -- a parser/format change silently producing malformed or empty output.
 *
 * Requires the outputs to exist: run `pnpm run build:data` first (CI does).
 * Usage:  node tools/test-generators.mjs   (wired as `pnpm test:generators`)
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const WM = path.join(ROOT, 'world-maps');

let failures = 0;
const problems = [];
function check(cond, msg) {
  if (!cond) { failures++; problems.push(msg); }
}
function load(name) {
  const p = path.join(WM, name);
  if (!fs.existsSync(p)) {
    console.error(`test-generators: ${name} is missing.`);
    console.error('Run `pnpm run build:data` first — world-maps/ is generated (#65).');
    process.exit(2);
  }
  try {
    return JSON.parse(fs.readFileSync(p, 'utf8'));
  } catch (e) {
    check(false, `${name}: not valid JSON (${e.message})`);
    return null;
  }
}
const isArr = (x) => Array.isArray(x) && x.length > 0;
const isStr = (x) => typeof x === 'string' && x.length > 0;
const isNum = (x) => typeof x === 'number' && Number.isFinite(x);
function uniqueVnums(rows, label) {
  const seen = new Set();
  for (const r of rows) {
    if (seen.has(r.vnum)) { check(false, `${label}: duplicate vnum ${r.vnum}`); return; }
    seen.add(r.vnum);
  }
}

// ---- world.json ----
{
  const w = load('world.json');
  if (w) {
    check(isArr(w.areas), 'world.json: areas is a non-empty array');
    check(w.rooms && typeof w.rooms === 'object' && Object.keys(w.rooms).length > 0,
      'world.json: rooms is a non-empty map');
    check(isNum(w.recallVnum), 'world.json: recallVnum is a number');
    for (const a of (w.areas || []).slice(0, 50)) {
      check(isStr(a.id) && isStr(a.name), `world.json: area ${a.id} has id+name`);
      check(isNum(a.vnumMin) && isNum(a.vnumMax) && a.vnumMin <= a.vnumMax,
        `world.json: area ${a.id} has sane vnum range`);
    }
    const rooms = Object.values(w.rooms || {});
    const badRoom = rooms.find((r) => !isNum(r.vnum) || !isStr(r.name) || typeof r.exits !== 'object');
    check(!badRoom, `world.json: every room has vnum/name/exits (offender ${badRoom && badRoom.vnum})`);
    check(w.rooms[String(w.recallVnum)] || rooms.some((r) => r.vnum === w.recallVnum),
      'world.json: recallVnum points at a real room');
  }
}

// ---- items.json ----
{
  const it = load('items.json');
  if (it) {
    check(isArr(it.items), 'items.json: items is a non-empty array');
    const bad = (it.items || []).find((x) => !isNum(x.vnum) || !isStr(x.name) || !isStr(x.type));
    check(!bad, `items.json: every item has vnum/name/type (offender ${bad && bad.vnum})`);
    uniqueVnums(it.items || [], 'items.json');
  }
}

// ---- bestiary.json ----
{
  const b = load('bestiary.json');
  if (b) {
    check(isArr(b.mobs), 'bestiary.json: mobs is a non-empty array');
    const bad = (b.mobs || []).find((m) => !isNum(m.vnum) || !isStr(m.name) || !isNum(m.level) || m.level < 0 || m.level > 200);
    check(!bad, `bestiary.json: every mob has vnum/name/sane level (offender ${bad && bad.vnum})`);
    uniqueVnums(b.mobs || [], 'bestiary.json');
  }
}

// ---- spawns.json ----
{
  const s = load('spawns.json');
  if (s) {
    const nonEmptyObj = (o) => o && typeof o === 'object' && !Array.isArray(o) && Object.keys(o).length > 0;
    check(nonEmptyObj(s.mobSpawns), 'spawns.json: mobSpawns is a non-empty map (vnum -> placement)');
    check(nonEmptyObj(s.itemSources), 'spawns.json: itemSources is a non-empty map');
    const sample = s.mobSpawns && Object.values(s.mobSpawns)[0];
    check(sample && isStr(sample.name) && Array.isArray(sample.rooms),
      'spawns.json: a mob spawn has name + rooms[]');
  }
}

// ---- shops.json ----
{
  const sh = load('shops.json');
  if (sh) {
    check(isArr(sh.shops), 'shops.json: shops is a non-empty array');
    const bad = (sh.shops || []).find((s) => !isNum(s.keeper) || !isNum(s.room));
    check(!bad, `shops.json: every shop has numeric keeper+room (offender ${bad && bad.keeper})`);
  }
}

// ---- classes.json ----
{
  const c = load('classes.json');
  if (c) {
    check(isArr(c.classes), 'classes.json: classes is a non-empty array');
    check(isArr(c.skills), 'classes.json: skills is a non-empty array');
    for (const cls of ['Saiyan', 'Patryn', 'Fist', 'Sorcerer', 'Mazoku']) {
      check((c.classNames || []).includes(cls), `classes.json: classNames includes ${cls}`);
    }
    const bad = (c.skills || []).find((sk) => !isStr(sk.name));
    check(!bad, 'classes.json: every skill has a name');
  }
}

// ---- world-graph.json ----
{
  const g = load('world-graph.json');
  if (g) {
    check(Array.isArray(g.edges) && g.edges.length > 0, 'world-graph.json: edges is a non-empty array');
    check(g.nodes != null, 'world-graph.json: nodes present');
  }
}

// ---- presence + non-trivial size of the human-readable outputs ----
for (const f of ['WORLD-MAP.md', 'ITEMS.md', 'BESTIARY.md', 'SHOPS.md', 'SPAWNS.md', 'CLASSES.md', 'world-browser.html']) {
  const p = path.join(WM, f);
  check(fs.existsSync(p) && fs.statSync(p).size > 200, `${f}: exists and is non-trivial`);
}

if (failures > 0) {
  console.error(`\ntest-generators: ${failures} contract violation(s):`);
  for (const p of problems) console.error('  - ' + p);
  process.exit(1);
}
console.log('test-generators: all generator output contracts hold.');
