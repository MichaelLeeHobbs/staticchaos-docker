# Lessons Learned: TypeScript 6.0.3 → 7.0.2 (web/)

**Date:** 2026-07-11
**Scope:** `web/` devDependency bump only (`typescript@6.0.3` → `7.0.2`). No `vite` change in this pass.
**Trigger:** dependabot PR #112.

## Outcome: clean upgrade, zero code changes required

- `pnpm run typecheck` (`tsc -b --noEmit`) — passed with no new errors under the existing
  `tsconfig.app.json` (`strict`, `noUnusedLocals`, `noUnusedParameters`,
  `noUncheckedIndexedAccess`, `noFallthroughCasesInSwitch` all on).
- `pnpm run build` (`sync-content.mjs && tsc -b && vite build`) — full project-reference build
  and Vite production build both succeeded, output unchanged in shape.
- `npx tsc --version` inside `web/` confirms `7.0.2` actually resolved (not silently pinned to
  the old 6.x via a stale lockfile/cache).

## Why this is worth recording anyway

TypeScript is dev-only here (types are erased before Vite ships anything to the browser), so a
clean `tsc` bump carries no runtime risk once typecheck+build are green — but major-version jumps
in `tsc` are exactly the kind of thing that silently start rejecting previously-valid code (new
strictness defaults, DOM lib type changes, stricter inference on generics/overloads). This repo's
config already opts into most of the stricter flags, which is likely why nothing broke: there was no
slack of "types that were technically wrong but tolerated by looser inference" for the new compiler
to catch.

## Checklist for the next major `tsc` bump

1. Bump `typescript` in `web/package.json`, `pnpm install`.
2. Run `pnpm run typecheck` first (fast signal) before the full `pnpm run build`.
3. `pnpm run build` needs `world-maps/` populated first (`pnpm run build:data` from repo root) —
   its absence fails the build with an unrelated-looking sync-content error; don't mistake that for
   a TS regression.
4. If typecheck fails: fix at the call site rather than loosening a compiler flag — this repo relies
   on `strict`/`noUncheckedIndexedAccess` etc. being on.
5. No dev-server/browser smoke test is needed for a `typescript`-only bump — it doesn't affect
   shipped output, only the build-time gate.
