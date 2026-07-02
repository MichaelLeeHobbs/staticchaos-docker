#!/usr/bin/env node
/*
 * lint-c.mjs -- footgun linter for the legacy C in src/.
 *
 * The Merc/Diku idiom of indexing parallel arrays by bare integer literals is
 * the direct source of a recurring bug class (see issues #44/#45/#48/#49): a
 * wrong-but-adjacent index that the compiler can't catch and a quick read misses.
 * Once a magic index has a name (#52 gave weapons[] the WP_* set), any *new* bare
 * index is a regression -- this linter makes that enforceable instead of relying
 * on a later audit to find it.
 *
 * Exits non-zero (with file:line:col) if any rule matches. Add rules to RULES as
 * more index families get named. Keep rules precise -- a false positive on
 * legit legacy code trains people to ignore the linter.
 *
 * Usage:  node tools/lint-c.mjs   (wired as `pnpm lint:c`)
 */

import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const SRC = path.join(ROOT, 'src');

/*
 * Each rule: { id, desc, re, hint }. `re` must have the /g flag; it is tested
 * per line so we can report line/column. Only add a rule once the corresponding
 * named constants exist -- otherwise it flags code that has nothing to migrate to.
 */
const RULES = [
  {
    id: 'weapons-magic-index',
    desc: 'bare integer index into pcdata->weapons[] (use a WP_* name)',
    re: /weapons\[\s*\d+\s*\]/g,
    hint: 'weapons[] slots are named WP_HIT..WP_BLOW in merc.h (#52). ' +
          'Computed indices (weapons[dt-TYPE_HIT], weapons[i]) are fine.',
  },
  // Future (enable once the target constants exist / patterns are precise):
  //  - MAX_* inclusive loop bounds -> off-by-one OOB (see #47)
  //  - per-school damage case using a mismatched gsn_*_ward (see #48)
];

const cFiles = fs.readdirSync(SRC)
  .filter((f) => f.endsWith('.c'))
  .map((f) => path.join(SRC, f));

let violations = 0;
for (const file of cFiles) {
  const lines = fs.readFileSync(file, 'utf8').split('\n');
  const rel = path.relative(ROOT, file).replace(/\\/g, '/');
  lines.forEach((line, i) => {
    for (const rule of RULES) {
      rule.re.lastIndex = 0;
      let m;
      while ((m = rule.re.exec(line)) !== null) {
        violations++;
        console.error(`${rel}:${i + 1}:${m.index + 1}: [${rule.id}] ${rule.desc}`);
        console.error(`    ${m[0].trim()}   -- ${rule.hint}`);
      }
    }
  });
}

if (violations > 0) {
  console.error(`\nlint-c: ${violations} violation(s) found.`);
  process.exit(1);
}
console.log(`lint-c: clean (${cFiles.length} files, ${RULES.length} rule(s)).`);
