#!/usr/bin/env bash
#
# EXAMPLE remote deploy — copy to deploy.sh and edit for your host.
#   cp deploy.example.sh deploy.sh   # deploy.sh is gitignored (it's yours)
#
# Deploys the whole stack (MUD + web) to a remote Docker host over SSH:
# packages this project (minus .git), copies it over, extracts into REMOTE_DIR,
# and rebuilds/restarts with docker compose. Named volumes (players, notes, …)
# are preserved across deploys.
#
# Prerequisites:
#   - An SSH host alias with key auth (set REMOTE below or pass REMOTE=user@host).
#   - tar, ssh, scp on PATH. On Windows run from WSL or Git Bash.
#
# Usage:
#   ./deploy.sh
#   REMOTE=user@host REMOTE_DIR=/srv/staticchaos ./deploy.sh
#
set -euo pipefail

REMOTE="${REMOTE:-your-ssh-host}"                 # SSH host alias or user@host
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
ssh "$REMOTE" "cd '$REMOTE_DIR'; docker compose ps; docker compose logs --no-log-prefix 2>/dev/null | grep -i 'ready to rock' | tail -1 || echo '(no boot line yet)'"

echo "==> done. Connect with: telnet <server> ${MUD_PORT:-4000}"
