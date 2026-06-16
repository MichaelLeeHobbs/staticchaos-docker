#!/usr/bin/env bash
#
# EXAMPLE safe MUD-only redeploy — copy to mud-redeploy.sh and edit for your host.
#   cp mud-redeploy.example.sh mud-redeploy.sh   # mud-redeploy.sh is gitignored
#
# Rebuilds + redeploys ONLY the MUD container (32-bit). Aborts if players are
# connected, and builds (verify) before restarting so a compile error never
# kills the running game. Use this for source/area changes; use deploy.sh for a
# full-stack deploy and web/deploy-web.sh for web-only.
#
set -euo pipefail
REMOTE="${REMOTE:-your-ssh-host}"                 # SSH host alias or user@host
REMOTE_DIR="${REMOTE_DIR:-/opt/staticchaos}"
SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TARBALL="$(mktemp "${TMPDIR:-/tmp}/sc.XXXXXX.tgz")"
trap 'rm -f "$TARBALL"' EXIT

N=$(ssh "$REMOTE" "ss -tn state established '( sport = :4000 )' 2>/dev/null | grep -c ':4000' || true")
N=${N:-0}
echo "==> players on :4000 = $N"
if [ "$N" -gt 0 ]; then echo "ABORT: players are online — not redeploying the MUD."; exit 1; fi

echo "==> packaging src (no web/node_modules/pdfs)"
tar --exclude=./.git --exclude='**/node_modules' --exclude=./node_modules --exclude=./web --exclude='**/*.pdf' \
    -czf "$TARBALL" -C "$SRC" .
ssh "$REMOTE" "mkdir -p '$REMOTE_DIR'"
scp -q "$TARBALL" "$REMOTE:/tmp/sc-deploy.tgz"

echo "==> extract + build (verify) then up"
ssh "$REMOTE" "set -e; cd '$REMOTE_DIR'; tar xzf /tmp/sc-deploy.tgz; rm -f /tmp/sc-deploy.tgz; docker compose build 2>&1 | tail -6 && docker compose up -d 2>&1 | tail -4"
echo "==> boot line:"
ssh "$REMOTE" "cd '$REMOTE_DIR'; sleep 3; docker compose logs --no-log-prefix 2>/dev/null | grep -i 'ready to rock' | tail -1 || echo '(no boot line yet)'"
