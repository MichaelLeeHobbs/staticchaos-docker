#!/usr/bin/env node
/*
 * build-npc-dialogue.mjs -- convert the NPC keyword dialogue pack (a markdown/YAML
 * design spec) into engine-loadable MOBprog speech triggers.
 *
 * For each per-area `entries:` block in the pack it emits one .prg per mob vnum
 * (area/MOBProgs/dlg_<vnum>.prg) containing `>speech_prog <kw...>~` blocks, and
 * wires each mob up via `M <vnum> dlg_<vnum>.prg` in that area file's #MOBPROGS
 * section.  CRITICAL: load_mobprogs exit(1)s on a vnum that doesn't exist, so we
 * only attach to vnums actually defined in an area listed in area.lst.
 *
 * v1: one (first) response per keyword group via `mpecho` (emits the authored line
 * verbatim, $-codes expanded by act).  Response *rotation* is a future enhancement.
 *
 * Usage: node tools/build-npc-dialogue.mjs "tmp/static_chaos_npc_keyword_dialogue_pack V4.md"
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const AREA = path.join(ROOT, 'area');
const MPDIR = path.join(AREA, 'MOBProgs');
const PACK = process.argv[2] || path.join(ROOT, 'tmp', 'static_chaos_npc_keyword_dialogue_pack V4.md');

// ---- 1. index every mob vnum -> area file (only areas in area.lst) ----
const lst = fs.readFileSync(path.join(AREA, 'area.lst'), 'utf8')
  .split(/\r?\n/).map(s => s.trim()).filter(s => /\.are$/i.test(s));
const vnumToFile = new Map();         // vnum -> filename (e.g. midgaard.are)
for (const f of lst) {
  const p = path.join(AREA, f);
  if (!fs.existsSync(p)) continue;
  const txt = fs.readFileSync(p, 'utf8');
  // only the #MOBILES section
  const mobsIdx = txt.indexOf('#MOBILES');
  if (mobsIdx < 0) continue;
  const after = txt.slice(mobsIdx);
  const end = after.search(/\n#(OBJECTS|ROOMDATA|ROOMS|RESETS|SPECIALS|SHOPS|MOBPROGS|\$)/);
  const section = end < 0 ? after : after.slice(0, end);
  for (const m of section.matchAll(/^#(\d+)\s*$/gm)) {
    const v = parseInt(m[1], 10);
    if (v > 0 && !vnumToFile.has(v)) vnumToFile.set(v, f);
  }
}

// ---- 2. parse the pack -> map vnum -> [{keywords:[], response:""}] ----
const lines = fs.readFileSync(PACK, 'utf8').split(/\r?\n/);
const byVnum = new Map();             // vnum -> array of {keywords, response}
let curVnums = [];                    // current entry's vnum list
let curKeywords = null;
let curResponses = [];
let inResponses = false;

function flushEntry() {
  if (curVnums.length && curKeywords && curResponses.length) {
    for (const v of curVnums) {
      if (!byVnum.has(v)) byVnum.set(v, []);
      byVnum.get(v).push({ keywords: curKeywords, response: curResponses[0] });
    }
  }
  curKeywords = null; curResponses = []; inResponses = false;
}

for (const raw of lines) {
  const line = raw.replace(/\t/g, '  ');
  const vnumM = line.match(/^\s*-?\s*vnum:\s*(\d+)\s*$/);
  const vnumsM = line.match(/^\s*-?\s*vnums:\s*\[([0-9,\s]+)\]\s*$/);
  const kwM = line.match(/^\s*keywords:\s*\[(.+)\]\s*$/);
  const respHdr = /^\s*responses:\s*$/.test(line);
  const respItem = line.match(/^\s*-\s*"(.*)"\s*$/);

  if (vnumM || vnumsM) {
    // new entry begins -> flush previous
    flushEntry();
    curVnums = vnumM ? [parseInt(vnumM[1], 10)]
                     : vnumsM[1].split(',').map(s => parseInt(s.trim(), 10)).filter(Boolean);
    continue;
  }
  if (kwM) {
    // a new keyword group within the same vnum(s): flush the previous group but keep vnums
    if (curKeywords && curResponses.length) {
      for (const v of curVnums) {
        if (!byVnum.has(v)) byVnum.set(v, []);
        byVnum.get(v).push({ keywords: curKeywords, response: curResponses[0] });
      }
    }
    curKeywords = kwM[1].split(',').map(s => s.trim().toLowerCase()).filter(Boolean);
    curResponses = []; inResponses = false;
    continue;
  }
  if (respHdr) { inResponses = true; curResponses = []; continue; }
  if (inResponses && respItem) { curResponses.push(respItem[1]); continue; }
  // a non-list, non-empty line ends a responses block
  if (inResponses && line.trim() && !respItem) inResponses = false;
}
flushEntry();

// ---- 3. generate .prg files + collect M-lines per area file ----
if (!fs.existsSync(MPDIR)) fs.mkdirSync(MPDIR, { recursive: true });
const mlinesByFile = new Map();       // arefile -> [ "M <vnum> dlg_<vnum>.prg" ]
let gen = 0, skipped = [];
const sanitize = (s) => s.replace(/[~]/g, '-');          // ~ ends a prog field; never allow it raw

for (const [vnum, groups] of [...byVnum.entries()].sort((a, b) => a[0] - b[0])) {
  const file = vnumToFile.get(vnum);
  if (!file) { skipped.push(vnum); continue; }
  // dedupe keyword groups (last response wins per identical keyword set)
  const seen = new Map();
  for (const g of groups) {
    const kw = g.keywords.filter(k => /^[a-z][a-z'-]*$/.test(k)).join(' ');
    if (!kw) continue;
    seen.set(kw, sanitize(g.response));
  }
  if (!seen.size) { skipped.push(vnum); continue; }
  let prg = '';
  for (const [kw, resp] of seen) {
    prg += `>speech_prog ${kw}~\n`;
    prg += `mpecho ${resp}\n`;
    prg += `~\n`;
  }
  prg += `|\n`;
  fs.writeFileSync(path.join(MPDIR, `dlg_${vnum}.prg`), prg);
  gen++;
  if (!mlinesByFile.has(file)) mlinesByFile.set(file, []);
  mlinesByFile.get(file).push(`M ${vnum} dlg_${vnum}.prg`);
}

// ---- 4. patch each area file's #MOBPROGS section ----
let patched = 0;
for (const [file, mlines] of mlinesByFile) {
  const p = path.join(AREA, file);
  let txt = fs.readFileSync(p, 'utf8');
  const eol = txt.includes('\r\n') ? '\r\n' : '\n';
  const block = mlines.join(eol);
  // strip any prior dlg_ lines we added before (idempotent re-runs)
  txt = txt.replace(new RegExp(`^M \\d+ dlg_\\d+\\.prg${eol}`, 'gm'), '');
  if (/^#MOBPROGS\s*$/m.test(txt)) {
    // insert our M lines right after the #MOBPROGS header
    txt = txt.replace(/^#MOBPROGS[ \t]*\r?\n/m, (h) => h + block + eol);
  } else {
    // no section: create one before the file-end marker #$
    const ins = `#MOBPROGS${eol}${block}${eol}S${eol}${eol}`;
    if (/^#\$/m.test(txt)) txt = txt.replace(/^#\$/m, ins + '#$');
    else txt = txt.trimEnd() + eol + ins;
  }
  fs.writeFileSync(p, txt);
  patched++;
}

// ---- 5. report ----
console.log(`NPC dialogue: ${gen} mobs wired across ${patched} area files; ${skipped.length} pack vnums skipped (not in a loaded area).`);
console.log(`Per-area: ` + [...mlinesByFile].map(([f, m]) => `${f}:${m.length}`).join(', '));
if (skipped.length) console.log(`Skipped vnums: ${skipped.slice(0, 40).join(', ')}${skipped.length > 40 ? ' …' : ''}`);
