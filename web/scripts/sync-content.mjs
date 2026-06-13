#!/usr/bin/env node
/*
 * sync-content.mjs -- copy the repo's generated content into web/public/ so the
 * static SPA can fetch it. Run by `pnpm sync` / `pnpm build` (and inside Docker).
 *
 *   world-maps/*.json            -> public/data/
 *   docs/*.md + world-maps/*.md  -> public/docs/   (+ docs/index.json manifest)
 *   client/*.xml + README.md     -> public/gmcp/   (+ gmcp/index.json manifest)
 */
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const WEB = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const ROOT = path.resolve(WEB, '..');
const PUB = path.join(WEB, 'public');
const DATA = path.join(PUB, 'data');
const DOCS = path.join(PUB, 'docs');
const GMCP = path.join(PUB, 'gmcp');

const reset = (d) => { fs.rmSync(d, { recursive: true, force: true }); fs.mkdirSync(d, { recursive: true }); };
const cp = (src, dst) => fs.copyFileSync(src, dst);
const titleOf = (file) => {
  const txt = fs.readFileSync(file, 'utf8');
  const m = txt.match(/^#\s+(.+?)\s*$/m);
  const t = (m ? m[1] : path.basename(file, '.md')).replace(/[`*_]/g, '').trim();
  return t || path.basename(file, '.md');
};
const ls = (dir, re) =>
  fs.existsSync(dir) ? fs.readdirSync(dir).filter((f) => re.test(f)).sort() : [];

reset(DATA);
reset(DOCS);
reset(GMCP);

// ---- data (JSON) ----
const wm = path.join(ROOT, 'world-maps');
let nData = 0;
for (const f of ls(wm, /\.json$/)) { cp(path.join(wm, f), path.join(DATA, f)); nData++; }

// ---- docs (markdown) + manifest ----
const docsIndex = [];
for (const f of ls(path.join(ROOT, 'docs'), /\.md$/)) {
  const src = path.join(ROOT, 'docs', f);
  cp(src, path.join(DOCS, f));
  docsIndex.push({ file: f, title: titleOf(src), group: 'Guides' });
}
for (const f of ls(wm, /\.md$/)) {
  const src = path.join(wm, f);
  cp(src, path.join(DOCS, f));
  const group = /^[A-Z]/.test(f) ? 'Reference' : 'Atlas (areas)';
  docsIndex.push({ file: f, title: titleOf(src), group });
}
const groupOrder = { Guides: 0, Reference: 1, 'Atlas (areas)': 2 };
docsIndex.sort((a, b) => (groupOrder[a.group] - groupOrder[b.group]) || a.title.localeCompare(b.title));
fs.writeFileSync(path.join(DOCS, 'index.json'), JSON.stringify(docsIndex, null, 2));

// ---- gmcp (Mudlet packages) + manifest ----
const GMCP_DESC = {
  'StaticChaos.xml': 'All-in-one bundle: gauges + auto-mapper + tab-completion (recommended).',
  'StaticChaos-Gauges.xml': 'HP / Mana / Move status gauges (Char.Vitals).',
  'StaticChaos-Mapper.xml': 'Auto-mapper fed by Room.Info.',
  'StaticChaos-Complete.xml': 'Context-aware Tab completion (game.Commands / game.Chants).',
  'README.md': 'Install instructions for the Mudlet packages.',
};
const client = path.join(ROOT, 'client');
const gmcpIndex = [];
for (const f of ls(client, /\.(xml|md)$/)) {
  const src = path.join(client, f);
  cp(src, path.join(GMCP, f));
  gmcpIndex.push({ file: f, bytes: fs.statSync(src).size, desc: GMCP_DESC[f] ?? '' });
}
fs.writeFileSync(path.join(GMCP, 'index.json'), JSON.stringify(gmcpIndex, null, 2));

console.log(`sync: ${nData} json, ${docsIndex.length} docs, ${gmcpIndex.length} gmcp -> web/public/`);
