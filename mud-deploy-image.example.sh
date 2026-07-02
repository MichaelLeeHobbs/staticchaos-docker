#!/usr/bin/env bash
#
# EXAMPLE image-based MUD deploy (#64) — copy to mud-deploy-image.sh and edit.
#   cp mud-deploy-image.example.sh mud-deploy-image.sh   # the real one is gitignored
#
# Deploys the CI-published GHCR image instead of building on the host: pull a tag,
# then `up -d` with docker-compose.image.yml. Player data (staticchaos_player etc.)
# lives in named volumes and is untouched by the image swap. Rollback = re-run with
# MUD_IMAGE pointing at a previous tag.
#
# Prereqs: the CI `publish-image` job has pushed the tag to GHCR, and the package is
# pullable from the host (public, or `docker login ghcr.io` done once on the host).
#
set -euo pipefail
REMOTE="${REMOTE:-your-ssh-host}"                 # SSH host alias or user@host
REMOTE_DIR="${REMOTE_DIR:-/opt/staticchaos}"
IMAGE_REPO="${IMAGE_REPO:-ghcr.io/michaelleehobbs/staticchaos-docker/mud}"
TAG="${1:-latest}"                                # e.g. sha-<gitsha>, or latest
MUD_IMAGE="${IMAGE_REPO}:${TAG}"

# 1. Never deploy over connected players.
N=$(ssh "$REMOTE" "ss -tn state established '( sport = :4000 )' 2>/dev/null | grep -c ':4000' || true")
N=${N:-0}
echo "==> players on :4000 = $N ; target image = $MUD_IMAGE"
if [ "$N" -gt 0 ]; then echo "ABORT: players are online — not deploying."; exit 1; fi

# 2. Ship the image compose file (only file the host needs for an image deploy).
ssh "$REMOTE" "mkdir -p '$REMOTE_DIR'"
scp -q docker-compose.image.yml "$REMOTE:$REMOTE_DIR/docker-compose.image.yml"

# 3. Pull the tag and bring it up. `docker compose config -q` validates first.
ssh "$REMOTE" "set -e; cd '$REMOTE_DIR'; \
  export MUD_IMAGE='$MUD_IMAGE'; \
  docker compose -f docker-compose.image.yml config -q; \
  docker compose -f docker-compose.image.yml pull 2>&1 | tail -3; \
  docker compose -f docker-compose.image.yml up -d 2>&1 | tail -4"

# 4. Verify boot + that volumes (player data) are still attached.
echo "==> boot line + health:"
ssh "$REMOTE" "cd '$REMOTE_DIR'; sleep 4; \
  docker compose -f docker-compose.image.yml ps --format '{{.Name}} {{.Status}}'; \
  docker compose -f docker-compose.image.yml logs --no-log-prefix 2>/dev/null | grep -i 'ready to rock' | tail -1 || echo '(no boot line yet)'"
echo "==> done. Rollback: $0 <previous-tag>"
