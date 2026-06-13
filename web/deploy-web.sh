#!/usr/bin/env bash
# Deploy the static companion site beside the MUD. Packages the repo (the web
# Docker build needs world-maps/, docs/, client/ as context), ships it, and
# (re)builds the `web` compose service on the remote host. Serves HTTP on :80.
set -euo pipefail
REMOTE="${REMOTE:-ubersrc01}"
REMOTE_DIR="${REMOTE_DIR:-/opt/staticchaos}"
SRC="/mnt/c/Users/mhobb/WebstormProjects/_experimental/staticchaos-docker"
TARBALL="$(mktemp /tmp/staticchaos-web.XXXXXX.tgz)"
trap 'rm -f "$TARBALL"' EXIT

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
ssh "$REMOTE" "docker ps --format '{{.Names}}\t{{.Ports}}' | grep -E 'staticchaos-web|chaos'"
echo "==> done. Browse: http://<server>/"
