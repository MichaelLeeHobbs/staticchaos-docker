# Lessons Learned: Vite 6.0.7 → 8.1.4 (web/)

**Date:** 2026-07-11
**Scope:** `web/` devDependency bump only (`vite@6.0.7` → `8.1.4`), applied on top of `main`
(pre-TS-7 upgrade — see `typescript-7-upgrade.md` for that separate bump).
**Trigger:** dependabot PR #113 (a two-major-version jump: 6 → 8).

## Outcome: clean upgrade, zero code changes required

- `pnpm run typecheck` — no new errors.
- `pnpm run build` (`sync-content.mjs && tsc -b && vite build`) — full production build succeeded;
  output is a normal Vite 8 bundle (new chunk hashes, "building client environment for production"
  log wording, otherwise unchanged shape). `@vitejs/plugin-react@5.2.0` (unchanged) resolved with no
  peer-dependency warnings against Vite 8.
- `pnpm run dev` — dev server boots, HMR client connects, and a Playwright smoke pass over `/`,
  `/map`, `/docs`, `/browser` loads each route cleanly with **no page errors and no failed network
  requests** from application code.
- The one console 404 seen during the smoke test (`/favicon.ico`) is the browser's own unconditional
  favicon probe — the site has never declared a `<link rel="icon">` or shipped a favicon file, on
  Vite 6 or Vite 8. Pre-existing, unrelated to this bump, left alone (out of scope for a dependency
  upgrade).

## Why this is worth recording anyway

A two-major jump (6 → 8) is exactly the kind of dependabot PR the sweep policy says never to
auto-merge — Vite majors have historically changed plugin APIs, `import.meta.env` behavior, and
default targets. This one turned out to be a no-op for this codebase, most likely because the app
uses only mainstream Vite/React-plugin surface (no custom plugins, no legacy Node API usage) — but
that had to be verified, not assumed, given the version distance.

## Checklist for the next major `vite` bump

1. Bump `vite` in `web/package.json`, `pnpm install`, and check `pnpm list vite @vitejs/plugin-react`
   for peer-dependency warnings (a real one would show up here, not just in the build log).
2. `pnpm run typecheck` first, then `pnpm run build` (needs `world-maps/` populated via
   `pnpm run build:data` from repo root, same as any web build).
3. Don't stop at the production build — also boot `pnpm run dev` and load the app in a real browser
   (or headless Chromium via Playwright), since Vite bugs concentrate in the dev server / HMR path
   more than in the static output. `/opt/pw-browsers/chromium` is preinstalled in this environment;
   no `playwright install` needed.
4. Diff failed network requests (`response.status() >= 400`) against a known-clean baseline before
   treating a console error as a regression — browsers fetch `/favicon.ico` unconditionally, and this
   site doesn't ship one, so that 404 is noise, not signal.
