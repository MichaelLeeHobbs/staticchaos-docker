#!/usr/bin/env node
/*
 * build-npc-dialogue.mjs (v2) -- convert the NPC keyword dialogue pack into
 * engine-loadable MOBprog speech triggers.
 *
 * v2 adds:
 *   - response ROTATION: each keyword group picks a random variant via nested
 *     `if rand(N)` ladders (the idiom used in beggar.prg / janitor.prg).
 *   - ROLE templates + UNIVERSAL CHATTER wired by classifying mobs:
 *       shopkeeper (from #SHOPS), trainer (ACT_TRAIN/ACT_PRACTICE), and
 *       healer/guard/bard/innkeeper/druid/civilian/animal/prisoner by name.
 *       Non-aggressive mobs (no ACT_AGGRESSIVE) also get the universal chatter
 *       pool as a fallback (animals excluded -- they keep flavor-only lines).
 *   - $-code fix: the pack's "$n says ..." means the NPC, but engine $n is the
 *     ACTOR (player). We rewrite $n -> $I (mob short_descr) and emit via `mpecho`
 *     (do_mpecho runs act(), which expands $I and capitalizes).
 *
 * Priority per keyword (deduped so a word never fires two blocks): area-specific
 * > role template > universal chatter.
 *
 * Safety: load_mobprogs exit(1)s on a vnum that doesn't exist, so we only attach
 * to vnums defined in an area listed in area.lst.  Idempotent: clears old dlg_*.prg
 * and strips prior M-lines before rewriting.
 *
 * Usage: node tools/build-npc-dialogue.mjs ["pack.md"]
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const AREA = path.join(ROOT, 'area');
const MPDIR = path.join(AREA, 'MOBProgs');
const PACK = process.argv[2] || path.join(ROOT, 'tools', 'data', 'npc-dialogue-pack.md');

const ACT_AGGRESSIVE = 32, ACT_TRAIN = 512, ACT_PRACTICE = 1024;
const EXCLUDE = new Set([2900,2901,2902,2903,2904,2905,2906,2907,2908, 3150, 3154]); // custom mobs

// ---------- 1. index mobs (vnum -> {file, text, act}) and shop keepers ----------
const lst = fs.readFileSync(path.join(AREA, 'area.lst'), 'utf8')
  .split(/\r?\n/).map(s => s.trim()).filter(s => /\.are$/i.test(s));
const mobs = new Map();          // vnum -> {file, text, act}
const keepers = new Set();
for (const f of lst) {
  const p = path.join(AREA, f);
  if (!fs.existsSync(p)) continue;
  const txt = fs.readFileSync(p, 'utf8');
  // #MOBILES
  const mi = txt.indexOf('#MOBILES');
  if (mi >= 0) {
    const after = txt.slice(mi);
    const end = after.search(/\n#(OBJECTS|ROOMDATA|ROOMS|RESETS|SPECIALS|SHOPS|MOBPROGS|\$)/);
    const sec = (end < 0 ? after : after.slice(0, end)).split(/\r?\n/);
    for (let i = 0; i < sec.length; i++) {
      const m = sec[i].match(/^#(\d+)\s*$/);
      if (!m) continue;
      const vnum = parseInt(m[1], 10);
      const kw = (sec[i + 1] || '').replace(/~.*$/, '');
      const short = (sec[i + 2] || '').replace(/~.*$/, '');
      let act = 0;
      for (let j = i + 3; j < Math.min(i + 40, sec.length); j++) {
        const a = sec[j].match(/^(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+S\b/);
        if (a) { act = parseInt(a[1], 10) || 0; break; }
        if (/^#\d+\s*$/.test(sec[j])) break;
      }
      if (vnum > 0 && !mobs.has(vnum)) mobs.set(vnum, { file: f, text: (kw + ' ' + short).toLowerCase(), act });
    }
  }
  // #SHOPS keepers (first field)
  const si = txt.indexOf('#SHOPS');
  if (si >= 0) {
    const after = txt.slice(si);
    const end = after.search(/\n#(MOBPROGS|\$)/);
    const sec = (end < 0 ? after : after.slice(0, end)).split(/\r?\n/);
    for (const ln of sec) {
      const m = ln.match(/^(\d+)\s+\d+/);
      if (m) keepers.add(parseInt(m[1], 10));
    }
  }
}

// ---------- 2. parse the pack ----------
const lines = fs.readFileSync(PACK, 'utf8').split(/\r?\n/);
const perVnum = new Map();       // vnum -> [{keywords[], responses[]}]
const roleGroups = {};           // roleKey -> [{keywords, responses}]
const chatterGroups = [];        // [{keywords, responses}]

const ROLE_HDR = {
  'shopkeepers / merchants': 'shopkeeper',
  'trainers / practice mobs': 'trainer',
  'guards': 'guard',
  'healers / priests': 'healer',
  'prisoners / slaves': 'prisoner',
  'bards / gossips': 'bard',
  'innkeepers / bartenders / waitstaff': 'innkeeper',
  'druids / hermits / herbalists': 'druid',
  'civilians / children / townsfolk': 'civilian',
  'animals / pets (flavor only, tier 0)': 'animal',
};

let mode = null;        // 'roles' | 'chatter' | 'area'
let curRole = null;     // role key when mode==roles
let curVnums = [];      // area mode
let g = null;           // current {keywords, responses}
let inResp = false;

function pushGroup() {
  if (!g || !g.keywords.length || !g.responses.length) { g = null; inResp = false; return; }
  if (mode === 'roles' && curRole) (roleGroups[curRole] ||= []).push(g);
  else if (mode === 'chatter') chatterGroups.push(g);
  else if (mode === 'area' && curVnums.length) {
    for (const v of curVnums) (perVnum.get(v) || perVnum.set(v, []).get(v)).push(g);
  }
  g = null; inResp = false;
}

for (const raw of lines) {
  const line = raw.replace(/\t/g, '  ');
  let m;
  if ((m = line.match(/^##\s+(.+?)\s*$/))) {
    pushGroup();
    const t = m[1].toLowerCase();
    if (t.startsWith('global role templates')) { mode = 'roles'; curRole = null; }
    else if (t.startsWith('universal chatter')) { mode = 'chatter'; }
    else { mode = 'area'; curVnums = []; }
    continue;
  }
  if (mode === 'roles' && (m = line.match(/^###\s+(.+?)\s*$/))) {
    pushGroup(); curRole = ROLE_HDR[m[1].trim().toLowerCase()] || null; continue;
  }
  if (mode === 'area') {
    if ((m = line.match(/^\s*-?\s*vnum:\s*(\d+)\s*$/))) { pushGroup(); curVnums = [parseInt(m[1], 10)]; continue; }
    if ((m = line.match(/^\s*-?\s*vnums:\s*\[([0-9,\s]+)\]\s*$/))) { pushGroup(); curVnums = m[1].split(',').map(s => parseInt(s.trim(), 10)).filter(Boolean); continue; }
  }
  if ((m = line.match(/^\s*(?:-\s*group:\s*\S+\s*)?$/)) && /group:/.test(line)) { pushGroup(); continue; }
  if ((m = line.match(/^\s*keywords:\s*\[(.+)\]\s*$/))) {
    pushGroup();
    g = { keywords: m[1].split(',').map(s => s.trim().toLowerCase()).filter(k => /^[a-z][a-z'-]*$/.test(k)), responses: [] };
    inResp = false; continue;
  }
  if (/^\s*responses:\s*$/.test(line)) { inResp = true; if (g) g.responses = []; continue; }
  if (inResp && (m = line.match(/^\s*-\s*"(.*)"\s*$/))) { if (g) g.responses.push(m[1]); continue; }
  if (inResp && line.trim() && !/^\s*-\s*"/.test(line)) inResp = false;
}
pushGroup();

// ---------- 3. helpers ----------
function classify(vnum, info) {
  if (keepers.has(vnum)) return 'shopkeeper';
  if (info.act & (ACT_TRAIN | ACT_PRACTICE)) return 'trainer';
  const t = info.text;
  if (/\b(healer|priest|cleric|nurse|prelate|bishop|monk|acolyte)\b/.test(t)) return 'healer';
  if (/\b(guard|soldier|sentinel|sentry|watchman|warden|guardian|knight|legionnaire)\b/.test(t)) return 'guard';
  if (/\b(bard|minstrel|singer|gossip|herald|crier|jester)\b/.test(t)) return 'bard';
  if (/\b(innkeeper|barkeep|bartender|waiter|waitress|barmaid|host|hostess|cook|server)\b/.test(t)) return 'innkeeper';
  if (/\b(druid|hermit|herbalist|shaman|witch|sage|wise)\b/.test(t)) return 'druid';
  if (/\b(prisoner|slave|captive|hostage)\b/.test(t)) return 'prisoner';
  if (/\b(child|kid|boy|girl|citizen|peasant|townsfolk|villager|maid|beggar|farmer|servant|merchant|man|woman|lady|gentleman)\b/.test(t)) return 'civilian';
  if (/\b(dog|cat|rabbit|rat|bird|horse|cow|chicken|pig|sheep|fish|squirrel|deer|pony|kitten|puppy|mouse|hound|wolf|bear|owl)\b/.test(t)) return 'animal';
  return null;
}

const sanitize = (s) => s.replace(/~/g, '-');
function toCmd(resp) {
  // pack convention: $n = the NPC.  engine: $n = actor (player), $I = mob short_descr.
  let line = sanitize(resp).replace(/\$n\b/g, '$I').replace(/\$N\b/g, '$I');
  return `mpecho ${line}`;
}
function randLadder(responses) {
  const r = responses.map(toCmd);
  if (r.length === 1) return r[0] + '\n';
  let out = '';
  let depth = 0;
  for (let i = 0; i < r.length - 1; i++) {
    const pct = Math.round(100 / (r.length - i));
    out += `if rand(${pct})\n${r[i]}\nelse\n`;
    depth++;
  }
  out += r[r.length - 1] + '\n';
  out += 'endif\n'.repeat(depth);
  return out;
}

// ---------- 4. build per-mob dialogue (dedupe keywords by priority) ----------
// clear stale generated progs
for (const f of fs.readdirSync(MPDIR)) if (/^dlg_\d+\.prg$/.test(f)) fs.unlinkSync(path.join(MPDIR, f));

const mlinesByFile = new Map();
let gen = 0, withChatter = 0, withRole = 0, withSpecific = 0;

for (const [vnum, info] of mobs) {
  if (EXCLUDE.has(vnum)) continue;
  const groups = [];                                   // ordered by priority
  if (perVnum.has(vnum)) { groups.push(...perVnum.get(vnum)); }
  const role = classify(vnum, info);
  if (role && roleGroups[role]) groups.push(...roleGroups[role]);
  const aggressive = (info.act & ACT_AGGRESSIVE) !== 0;
  if (!aggressive && role !== 'animal') groups.push(...chatterGroups);
  if (!groups.length) continue;

  // keyword-level dedup: a keyword fires only its highest-priority block
  const used = new Set();
  const blocks = [];
  for (const grp of groups) {
    const kws = grp.keywords.filter(k => !used.has(k));
    if (!kws.length || !grp.responses.length) continue;
    kws.forEach(k => used.add(k));
    blocks.push({ kws, responses: grp.responses });
  }
  if (!blocks.length) continue;

  let prg = '';
  for (const b of blocks) prg += `>speech_prog ${b.kws.join(' ')}~\n${randLadder(b.responses)}~\n`;
  prg += '|\n';
  fs.writeFileSync(path.join(MPDIR, `dlg_${vnum}.prg`), prg);
  gen++;
  if (perVnum.has(vnum)) withSpecific++; if (role) withRole++; if (!aggressive && role !== 'animal') withChatter++;
  (mlinesByFile.get(info.file) || mlinesByFile.set(info.file, []).get(info.file)).push(`M ${vnum} dlg_${vnum}.prg`);
}

// ---------- 5. patch area files ----------
let patched = 0;
for (const [file, mlines] of mlinesByFile) {
  const p = path.join(AREA, file);
  let txt = fs.readFileSync(p, 'utf8');
  const eol = txt.includes('\r\n') ? '\r\n' : '\n';
  txt = txt.replace(new RegExp(`^M \\d+ dlg_\\d+\\.prg${eol}`, 'gm'), '');
  const block = mlines.join(eol);
  if (/^#MOBPROGS\s*$/m.test(txt)) txt = txt.replace(/^#MOBPROGS[ \t]*\r?\n/m, (h) => h + block + eol);
  else {
    const ins = `#MOBPROGS${eol}${block}${eol}S${eol}${eol}`;
    txt = /^#\$/m.test(txt) ? txt.replace(/^#\$/m, ins + '#$') : txt.trimEnd() + eol + ins;
  }
  fs.writeFileSync(p, txt);
  patched++;
}

console.log(`NPC dialogue v2: ${gen} mobs wired across ${patched} areas (specific:${withSpecific}, role:${withRole}, chatter:${withChatter}).`);
console.log(`Role templates: ${Object.keys(roleGroups).map(k => `${k}:${roleGroups[k].length}`).join(', ')}; chatter groups: ${chatterGroups.length}.`);
