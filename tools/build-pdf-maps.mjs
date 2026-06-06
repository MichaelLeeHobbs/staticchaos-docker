#!/usr/bin/env node
/*
 * build-pdf-maps.mjs -- render the Mermaid world maps into a single PDF.
 *
 * md-to-pdf alone can't render Mermaid, so this pre-renders every diagram to
 * SVG with @mermaid-js/mermaid-cli (mmdc), then prints the result to PDF:
 *
 *   1. gather the world-overview diagram (WORLD-MAP.md) + each per-area
 *      diagram (world-maps/<area>.md) into one combined markdown
 *   2. mmdc renders all Mermaid blocks to SVG (edge cap raised; big areas
 *      like hitower/ultima/sewer exceed Mermaid's default 500-edge limit)
 *   3. md-to-pdf prints the SVG-embedded markdown to world-maps/WORLD-MAPS.pdf
 *      (A3 landscape so the wide auto-laid-out graphs have room)
 *
 * Requires: pnpm add -D @mermaid-js/mermaid-cli, and a puppeteer Chrome
 * (same one build:pdf uses). Run build:world first.
 *
 * Usage:  node tools/build-pdf-maps.mjs
 */

import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { execFileSync } from 'node:child_process';
import { mdToPdf } from 'md-to-pdf';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const OUT = path.join(ROOT, 'world-maps');
const TMP = path.join(OUT, '.maps-tmp');
const MMDC = path.join(ROOT, 'node_modules', '@mermaid-js', 'mermaid-cli', 'src', 'cli.js');

const mermaidBlock = (md) => (md.match(/```mermaid\n([\s\S]*?)```/) || [])[1] || null;

// Locate a puppeteer-installed Chrome (mmdc's puppeteer doesn't auto-resolve it here).
function findChrome() {
  if (process.env.PUPPETEER_EXECUTABLE_PATH) return process.env.PUPPETEER_EXECUTABLE_PATH;
  const root = path.join(process.env.PUPPETEER_CACHE_DIR || path.join(os.homedir(), '.cache', 'puppeteer'), 'chrome');
  if (!fs.existsSync(root)) return null;
  const rel = process.platform === 'win32' ? ['chrome-win64', 'chrome.exe']
    : process.platform === 'darwin' ? ['chrome-mac-x64', 'Google Chrome for Testing.app', 'Contents', 'MacOS', 'Google Chrome for Testing']
      : ['chrome-linux64', 'chrome'];
  for (const dir of fs.readdirSync(root).sort().reverse()) {
    const exe = path.join(root, dir, ...rel);
    if (fs.existsSync(exe)) return exe;
  }
  return null;
}

async function main() {
  if (!fs.existsSync(path.join(OUT, 'WORLD-MAP.md'))) { console.error('Run build:world first (WORLD-MAP.md missing).'); process.exit(1); }
  fs.rmSync(TMP, { recursive: true, force: true });
  fs.mkdirSync(TMP, { recursive: true });

  const world = JSON.parse(fs.readFileSync(path.join(OUT, 'world.json'), 'utf8'));

  // ---- assemble one markdown with every diagram ----
  const parts = ['# Static Chaos — World Maps', ''];
  const overview = mermaidBlock(fs.readFileSync(path.join(OUT, 'WORLD-MAP.md'), 'utf8'));
  if (overview) parts.push('## World overview (area connectivity)', '', '```mermaid', overview.trimEnd(), '```', '');
  let count = overview ? 1 : 0;
  for (const a of world.areas) {
    const f = path.join(OUT, `${a.id}.md`);
    if (!fs.existsSync(f)) continue;
    const block = mermaidBlock(fs.readFileSync(f, 'utf8'));
    if (!block) continue;
    parts.push('<div style="page-break-before: always"></div>', '', `## ${a.name}  (${a.id}) — ${a.roomCount} rooms`, '', '```mermaid', block.trimEnd(), '```', '');
    count++;
  }
  const combined = path.join(TMP, 'maps.md');
  fs.writeFileSync(combined, parts.join('\n') + '\n');

  // ---- mermaid + puppeteer config ----
  const mmCfg = path.join(TMP, 'mermaid.json');
  fs.writeFileSync(mmCfg, JSON.stringify({ maxEdges: 5000, maxTextSize: 500000, flowchart: { useMaxWidth: true, htmlLabels: true } }));
  const ppCfg = path.join(TMP, 'puppeteer.json');
  const chrome = findChrome();
  if (!chrome) { console.error('No puppeteer Chrome found. Run: npx @puppeteer/browsers install chrome@149.0.7827.22 --path ~/.cache/puppeteer'); process.exit(1); }
  fs.writeFileSync(ppCfg, JSON.stringify({ executablePath: chrome, args: ['--no-sandbox'] }));

  // ---- render all diagrams to SVG ----
  const rendered = path.join(TMP, 'maps.rendered.md');
  console.log(`rendering ${count} diagrams with mmdc … (this launches headless Chrome)`);
  execFileSync(process.execPath, [MMDC, '-i', combined, '-o', rendered, '-c', mmCfg, '-p', ppCfg, '-e', 'svg'], { stdio: 'inherit' });

  // ---- print to PDF ----
  const dest = path.join(OUT, 'WORLD-MAPS.pdf');
  await mdToPdf(
    { path: rendered },
    {
      dest,
      basedir: TMP,
      css: 'img{max-width:100%;height:auto;display:block;margin:0 auto} h2{font-size:14px} body{font-family:system-ui,sans-serif}',
      pdf_options: { format: 'A3', landscape: true, printBackground: true, margin: { top: '10mm', bottom: '10mm', left: '8mm', right: '8mm' } },
    },
  );

  fs.rmSync(TMP, { recursive: true, force: true });
  console.log(`maps pdf : ${count} diagrams -> world-maps/WORLD-MAPS.pdf`);
}
main().catch((e) => { console.error(e); process.exit(1); });
