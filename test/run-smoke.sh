#!/usr/bin/env bash
# Boot the MUD (from the already-built `mud` image), run the integration smoke
# test against it, then tear it down. Used by CI (#69) and locally. Assumes the
# image is built (`docker compose build mud`); does not rebuild.
#
#   bash test/run-smoke.sh
#
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."

cleanup() { docker compose down >/dev/null 2>&1 || true; }
trap cleanup EXIT

echo "==> starting mud"
docker compose up -d mud >/dev/null

echo "==> waiting for healthy"
for i in $(seq 1 40); do
  st=$(docker inspect --format '{{.State.Health.Status}}' staticchaos 2>/dev/null || echo none)
  [ "$st" = healthy ] && break
  # fail fast if it died / boot-looped
  running=$(docker inspect --format '{{.State.Running}}' staticchaos 2>/dev/null || echo false)
  [ "$running" = "true" ] || { echo "container not running"; docker compose logs mud | tail -20; exit 1; }
  sleep 2
done

echo "==> running smoke test"
python3 test/mud_smoke.py "$@"

echo "==> running password-hashing migration test"
python3 test/password_migration.py "$@"
