#!/usr/bin/env node
/*
 * test-area-parser.mjs -- golden-output (characterization) test for the
 * hand-written .are parser in tools/lib/area.mjs.
 *
 * area.mjs mirrors the authoritative binary/text format in src/core/db.c. When the C
 * format changes and the parser is not updated it drifts *silently* -- it keeps
 * producing output, just wrong output. There is no runtime error to catch it.
 *
 * This test parses a small, known-good fixture (tools/__fixtures__/sample.are)
 * and deep-compares the result to a committed golden snapshot
 * (tools/__fixtures__/sample.expected.json). Any change in parser behaviour --
 * a shifted field, a dropped section, a flag that stops decoding -- flips this
 * test red, forcing a deliberate re-baseline (regenerate the golden) instead of
 * shipping wrong maps/items.
 *
 * To re-baseline after an *intentional* parser change:
 *   node -e "import('node:fs').then(fs=>import('./tools/lib/area.mjs').then(m=>\
 *     fs.writeFileSync('tools/__fixtures__/sample.expected.json',\
 *     JSON.stringify(m.parseAreaFile('tools/__fixtures__/sample.are'),null,2)+'\n')))"
 * ...then eyeball the diff before committing.
 */

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { parseAreaFile } from './lib/area.mjs';

const here = path.dirname(fileURLToPath(import.meta.url));
const fixture = path.join(here, '__fixtures__', 'sample.are');
const golden = path.join(here, '__fixtures__', 'sample.expected.json');

/* Deep compare that reports the first differing path (dot/bracket notation). */
function firstDiff(a, b, p = '') {
  if (a === b) return null;
  const ta = typeofDeep(a);
  const tb = typeofDeep(b);
  if (ta !== tb) return { path: p || '(root)', expected: b, actual: a };
  if (ta === 'array') {
    if (a.length !== b.length) {
      return { path: `${p}.length`, expected: b.length, actual: a.length };
    }
    for (let i = 0; i < a.length; i++) {
      const d = firstDiff(a[i], b[i], `${p}[${i}]`);
      if (d) return d;
    }
    return null;
  }
  if (ta === 'object') {
    const keys = new Set([...Object.keys(a), ...Object.keys(b)]);
    for (const k of keys) {
      const d = firstDiff(a[k], b[k], p ? `${p}.${k}` : k);
      if (d) return d;
    }
    return null;
  }
  return { path: p || '(root)', expected: b, actual: a };
}

function typeofDeep(v) {
  if (Array.isArray(v)) return 'array';
  if (v === null) return 'null';
  return typeof v;
}

function fail(msg, diff) {
  console.error(`FAIL: area parser drift test\n  ${msg}`);
  if (diff) {
    console.error(`  first differing path: ${diff.path}`);
    console.error(`  expected (golden): ${JSON.stringify(diff.expected)}`);
    console.error(`  actual   (parsed): ${JSON.stringify(diff.actual)}`);
    console.error(
      '\n  If this change is intentional, re-baseline the golden ' +
      '(see header of tools/test-area-parser.mjs) and eyeball the diff.',
    );
  }
  process.exit(1);
}

if (!fs.existsSync(golden)) {
  fail(`missing golden snapshot: ${golden}`);
}

// JSON round-trip the parsed output so we compare like-for-like with the golden
// (drops undefined, normalises to plain JSON types).
const parsed = JSON.parse(JSON.stringify(parseAreaFile(fixture)));
const expected = JSON.parse(fs.readFileSync(golden, 'utf8'));

const diff = firstDiff(parsed, expected);
if (diff) {
  fail('parsed output does not match golden snapshot', diff);
}

const counts = `rooms=${parsed.rooms.length} mobiles=${parsed.mobiles.length} ` +
  `objects=${parsed.objects.length} resets=${parsed.resets.length} ` +
  `shops=${parsed.shops.length}`;
console.log(`PASS: area parser matches golden snapshot (${counts})`);
process.exit(0);
