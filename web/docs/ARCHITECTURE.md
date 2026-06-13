# Static Chaos — Companion Site (architecture)

A **static** single-page app that presents everything we've generated for the MUD:
the docs, the world atlas/maps, an interactive world map, a searchable data browser
(items / mobs / shops / spawns), and the GMCP Mudlet client files.

**No backend. No database. No login.** It is HTML/JS/CSS plus a folder of prebuilt
data files, served by a tiny static file server in one Docker container, **beside the
MUD** on the LAN server, on **port 80 (plain HTTP — no TLS, no cert)**.

## Stack

- **React 19 + Vite + TypeScript** (strict).
- **MUI** for UI (matches our other apps).
- **React Router** in **HashRouter** mode — no server-side route config needed, so any
  dumb static server works.
- **d3** (npm) for the interactive world map.
- **react-markdown + remark-gfm** for docs (our markdown is GFM tables), **mermaid** for
  the area-map diagrams embedded in the atlas.

Server state / API libraries (TanStack Query, Zod, etc.) are **not** used — there is no
server to talk to. All data is static and loaded with `fetch()` at runtime.

## Where the content comes from

The repo's `tools/` already generate the content. A **sync step copies the latest into
`web/public/`** so the SPA can fetch it (content updates without touching components):

```
web/public/data/   <- world-maps/*.json   (world, items, bestiary, shops, spawns, classes)
web/public/docs/   <- docs/*.md + world-maps/*.md (atlas, per-area maps, gearing guide…)
web/public/gmcp/   <- client/*.xml + client/README.md (Mudlet packages)
```

Run `pnpm sync` (a small script in `web/`) after regenerating data with `pnpm build:data`
at the repo root. Pages `fetch('/data/world.json')` etc. — no bundling of large JSON.

## Layout

It's a small app, so it stays flat — pages + a few shared components/hooks, no
ports/adapters/feature-canon (that's for our backends, not a static viewer).

```
web/
  public/            data/ docs/ gmcp/  (synced, git-ignored except a .gitkeep)
  src/
    app/             main.tsx, app.tsx (HashRouter + theme + layout)
    components/      app-layout.tsx, nav, shared bits (markdown-view, data-table…)
    lib/             data.ts (typed fetch loaders), types.ts (shapes of the JSON)
    pages/
      home.page.tsx
      docs.page.tsx        (renders any /docs/*.md by route param; mermaid + GFM)
      map.page.tsx         (d3 interactive world map from world.json)
      browser.page.tsx     (searchable tables: items/mobs/shops/spawns)
      downloads.page.tsx    (GMCP client files + install notes)
  scripts/sync-content.mjs  (copies world-maps/docs/client -> public/)
  Dockerfile  docker-compose.yml  vite.config.ts  tsconfig.json
```

Files are **kebab-case**; React components/pages are **PascalCase**; hooks are `useXyz`
(repo-wide rule). Pages are thin; data-loading lives in `lib/` + small hooks.

## Build & deploy

- **Build:** `pnpm build` → `web/dist/` (static assets).
- **Container:** multi-stage Dockerfile — stage 1 builds with Node+pnpm; stage 2 is a tiny
  Node static server that serves `dist/` with **SPA fallback** (any unknown path → `index.html`)
  on **port 80**. No nginx.
- **Compose:** one service (`web`) mapping host **80 → container 80**, alongside the MUD's
  compose. Deployed to the server with the same tar+ssh+`docker compose up -d --build` flow
  as the MUD (`deploy.sh`).

Access: `http://<server>/` on the LAN.
