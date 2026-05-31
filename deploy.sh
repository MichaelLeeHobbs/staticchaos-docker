#!/usr/bin/env bash
#
# Deploy Static Chaos to a remote Docker host over SSH.
#
# Packages this project (minus .git), copies it to the remote host, extracts it
# into REMOTE_DIR, and rebuilds/restarts the stack with docker compose. Game data
# in the named volumes (players, notes, etc.) is preserved across deploys.
#
# Prerequisites:
#   - An SSH host alias with key auth already set up (default: "ubersrc01").
#     See README.docker.md / the ~/.ssh/config entry.
#   - tar, ssh, and scp on PATH. On Windows, run this from WSL or Git Bash.
#
# Usage:
#   ./deploy.sh
#   REMOTE=myhost REMOTE_DIR=/srv/staticchaos ./deploy.sh
#
set -euo pipefail

REMOTE="${REMOTE:-ubersrc01}"          # SSH host (alias or user@host)
REMOTE_DIR="${REMOTE_DIR:-/opt/staticchaos}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

TARBALL="$(mktemp "${TMPDIR:-/tmp}/staticchaos.XXXXXX.tgz")"
trap 'rm -f "$TARBALL"' EXIT

echo "==> packaging project: $SCRIPT_DIR"
tar --exclude=./.git -czf "$TARBALL" -C "$SCRIPT_DIR" .

echo "==> copying to $REMOTE:$REMOTE_DIR"
ssh "$REMOTE" "mkdir -p '$REMOTE_DIR'"
scp -q "$TARBALL" "$REMOTE:/tmp/staticchaos-deploy.tgz"

echo "==> extracting and (re)building on $REMOTE"
ssh "$REMOTE" "set -e; cd '$REMOTE_DIR'; tar xzf /tmp/staticchaos-deploy.tgz; rm -f /tmp/staticchaos-deploy.tgz; docker compose up -d --build"

echo "==> status"
ssh "$REMOTE" "cd '$REMOTE_DIR'; docker compose ps; docker compose logs --no-log-prefix 2>/dev/null | grep -i 'ready to rock' | tail -1 || echo '(no boot line yet -- check: ssh $REMOTE \"cd $REMOTE_DIR && docker compose logs\")'"

echo "==> done. Connect with: telnet <server-ip> \${MUD_PORT:-4000}"
