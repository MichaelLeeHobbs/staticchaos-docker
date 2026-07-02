#!/usr/bin/env bash
#
# EXAMPLE web-only deploy — copy to web/deploy-web.sh and edit for your host.
#   cp web/deploy-web.example.sh web/deploy-web.sh   # deploy-web.sh is gitignored
#
# Deploys ONLY the static companion site beside the MUD. The web Docker build
# needs world-maps/, docs/, client/ as context, so it packages the whole repo
# (minus node_modules/dist/pdfs) and (re)builds the `web` compose service.
# Serves HTTP on :80.
#
set -euo pipefail
REMOTE="${REMOTE:-your-ssh-host}"                 # SSH host alias or user@host
REMOTE_DIR="${REMOTE_DIR:-/opt/staticchaos}"
SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"   # repo root (this script lives in web/)
TARBALL="$(mktemp "${TMPDIR:-/tmp}/staticchaos-web.XXXXXX.tgz")"
trap 'rm -f "$TARBALL"' EXIT

# world-maps/ is generated, not committed (#65) -- regenerate so the package ships
# fresh game data (the web Docker build COPYs world-maps/ from this context).
echo "==> generating world data (build:data)"
( cd "$SRC" && pnpm run build:data )

echo "==> packaging (excluding .git, node_modules, dist, pdfs)"
tar --exclude=./.git --exclude='**/node_modules' --exclude=./node_modules \
    --exclude='./web/node_modules' --exclude='./web/dist' --exclude='**/*.pdf' \
    -czf "$TARBALL" -C "$SRC" .

echo "==> copying to $REMOTE:$REMOTE_DIR"
ssh "$REMOTE" "mkdir -p '$REMOTE_DIR'"
scp -q "$TARBALL" "$REMOTE:/tmp/staticchaos-web.tgz"

echo "==> extract + build + up (web) on $REMOTE"
ssh "$REMOTE" "set -e; cd '$REMOTE_DIR'; tar xzf /tmp/staticchaos-web.tgz; rm -f /tmp/staticchaos-web.tgz; cd web; docker compose up -d --build 2>&1 | tail -8"

echo "==> status"
ssh "$REMOTE" "docker ps --format '{{.Names}}\t{{.Ports}}' | grep -E 'staticchaos-web|chaos' || true"
echo "==> done. Browse: http://<server>/"
