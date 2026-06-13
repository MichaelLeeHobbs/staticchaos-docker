# Build context is the REPO ROOT (so we can sync world-maps/docs/client).
# Build: pnpm install -> sync content -> vite build. Serve: tiny node static server on :80.

FROM node:22-alpine AS build
RUN corepack enable
WORKDIR /app/web

# deps first (layer cache)
COPY web/package.json web/pnpm-lock.yaml ./
RUN pnpm install --frozen-lockfile

# app + the content the sync step pulls in
COPY web/ ./
COPY world-maps/ /app/world-maps/
COPY docs/ /app/docs/
COPY client/ /app/client/
RUN pnpm build

FROM node:22-alpine AS serve
WORKDIR /app
COPY --from=build /app/web/server.mjs ./server.mjs
COPY --from=build /app/web/dist ./dist
ENV PORT=80
EXPOSE 80
CMD ["node", "server.mjs"]
