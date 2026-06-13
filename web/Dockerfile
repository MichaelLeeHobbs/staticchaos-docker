# Build context is the REPO ROOT (so we can sync world-maps/docs/client).
# Build: npm install -> sync content -> vite build. Serve: tiny node static server on :80.
#
# We use npm (not pnpm) inside the image on purpose: pnpm's store hardlink/SQLite
# steps hit EPERM / disk-I/O errors on the server's storage driver. npm does plain
# copy installs, which work here. Local dev still uses pnpm (pnpm-lock.yaml).

FROM node:22-alpine AS build
WORKDIR /app/web

# deps first (layer cache); package scripts call node/tsc/vite directly, not pnpm
COPY web/package.json ./
RUN npm install --no-audit --no-fund

# app + the content the sync step pulls in
COPY web/ ./
COPY world-maps/ /app/world-maps/
COPY docs/ /app/docs/
COPY client/ /app/client/
RUN npm run build

FROM node:22-alpine AS serve
WORKDIR /app
COPY --from=build /app/web/server.mjs ./server.mjs
COPY --from=build /app/web/dist ./dist
ENV PORT=80
EXPOSE 80
CMD ["node", "server.mjs"]
