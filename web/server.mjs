#!/usr/bin/env node
/*
 * Minimal static file server for the built SPA (no nginx, no deps).
 * Serves dist/ and falls back to index.html for unknown paths (HashRouter
 * means this is rarely needed, but it keeps deep links / refreshes working).
 */
import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const DIST = path.resolve(path.dirname(fileURLToPath(import.meta.url)), 'dist');
const PORT = Number(process.env.PORT ?? 80);

const TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg': 'image/svg+xml',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.ico': 'image/x-icon',
  '.woff2': 'font/woff2',
  '.xml': 'application/xml; charset=utf-8',
  '.md': 'text/markdown; charset=utf-8',
  '.map': 'application/json; charset=utf-8',
};

function send(res, status, body, type) {
  res.writeHead(status, { 'Content-Type': type ?? 'text/plain; charset=utf-8' });
  res.end(body);
}

const server = http.createServer((req, res) => {
  try {
    const url = new URL(req.url ?? '/', 'http://localhost');
    let rel = decodeURIComponent(url.pathname);
    if (rel.endsWith('/')) rel += 'index.html';
    // resolve safely inside DIST
    let file = path.join(DIST, rel);
    if (!file.startsWith(DIST)) return send(res, 403, 'Forbidden');
    if (!fs.existsSync(file) || !fs.statSync(file).isFile()) {
      // SPA fallback: serve index.html for non-asset routes
      file = path.join(DIST, 'index.html');
    }
    const ext = path.extname(file).toLowerCase();
    const type = TYPES[ext] ?? 'application/octet-stream';
    const cache = ext === '.html' ? 'no-cache' : 'public, max-age=3600';
    res.writeHead(200, { 'Content-Type': type, 'Cache-Control': cache });
    fs.createReadStream(file).pipe(res);
  } catch {
    send(res, 500, 'Internal Server Error');
  }
});

server.listen(PORT, () => {
  console.log(`staticchaos-web serving ${DIST} on :${PORT}`);
});
