# Coding Standard — companion site

Lean rules for this **static React SPA**. (The backend ports-&-adapters canon, Zod/Pino/
`tryCatch`, etc. do not apply — there is no server here.)

## TypeScript
- **Strict mode on.** No `any` — use `unknown` and narrow. `noUncheckedIndexedAccess`.
- **No enums** — use union types (`type Tab = 'items' | 'mobs'`) or `as const` objects.
- Prefer `const`, `readonly`, and immutable updates (`[...xs]`, `{...o}`).
- Type the shapes of the JSON we load once, in `src/lib/types.ts`; load through typed
  helpers in `src/lib/data.ts` (one `fetch` wrapper, no scattered `fetch` calls).

## React
- **Function components only.** Logic and data-loading go in **hooks** (`useXyz`);
  components stay presentational (props in, JSX out).
- **MUI** for layout and controls; style with the `sx` prop or theme — avoid ad-hoc CSS
  files and inline `style={}` beyond trivial cases.
- Keep pages **thin**: a page composes hooks + components for one route.
- Loading/empty/error states are explicit (a static `fetch` can still 404 in dev).

## Naming & files
- Files **kebab-case** (`map.page.tsx`, `data-table.tsx`, `use-world.ts`).
- Components/Pages **PascalCase** (`MapPage`); hooks `useThing`; plain fns camelCase.
- One page per route file, suffixed `.page.tsx`.

## Dependencies & hygiene
- **Pin versions** (exact) in `package.json`; commit `pnpm-lock.yaml`.
- No `console.log` left in committed code (a stray `console.warn` for a real fetch failure
  is fine).
- Keep the bundle lean: import only what you use from MUI/d3; large data stays in
  `public/` and is fetched, never bundled.

## Tests (light)
- Vitest is available if a piece of logic is worth a unit test (e.g. a data transform).
  This is a viewer — don't chase coverage; test the bits that can silently break.
