#!/usr/bin/env bash
#
# backup-volumes.example.sh — template for the periodic off-host backup of the
# Static Chaos docker game-state volumes (area / player / notes / finger / log).
#
# Why: `area` is auto-dumped to disk every 30 min (PULSE_DB_DUMP -> do_asave),
# and in-game OLC builds write there too. The named volumes survive container
# recreate, but NOT host disk loss or an accidental `docker compose down -v`.
# This script snapshots each volume to a tarball with date-based rotation.
#
# Install (host-specific; the real copy is gitignored):
#   cp backup-volumes.example.sh backup-volumes.sh    # adjust paths if needed
#   chmod +x backup-volumes.sh
#   # then schedule it, e.g. /etc/cron.d/staticchaos-backup:
#   #   15 4 * * * root /opt/staticchaos/backup-volumes.sh
#
# IMPORTANT: BACKUP_ROOT must live OUTSIDE the docker build context
# (the project dir), or `COPY . /mud` will bundle the tarballs into every image.

set -euo pipefail

BACKUP_ROOT="/opt/backups/staticchaos"   # keep OUTSIDE /opt/staticchaos (build context)
VOLUMES="staticchaos_area staticchaos_player staticchaos_notes staticchaos_finger staticchaos_log"
RETAIN_DAYS=14                            # delete tarballs older than this

stamp="$(date +%Y-%m-%d_%H%M%S)"
mkdir -p "$BACKUP_ROOT"
log="$BACKUP_ROOT/backup.log"

{
  echo "=== $(date '+%F %T %Z') backup start ==="
  for v in $VOLUMES; do
    if ! docker volume inspect "$v" >/dev/null 2>&1; then
      echo "  SKIP $v (missing)"; continue
    fi
    out="${v}-${stamp}.tgz"
    # Read-only mount of the volume; tar its contents into BACKUP_ROOT.
    # Note: taken live (no downtime); a worst case catches one file mid-write,
    # which the next run corrects. Stop the container first if you need a
    # guaranteed-consistent snapshot.
    if docker run --rm -v "${v}:/src:ro" -v "${BACKUP_ROOT}:/dst" alpine \
         tar czf "/dst/${out}" -C /src . ; then
      echo "  OK   ${out} ($(du -h "${BACKUP_ROOT}/${out}" | cut -f1))"
    else
      echo "  FAIL ${v}"
    fi
  done
  # Rotation: prune archives older than RETAIN_DAYS.
  find "$BACKUP_ROOT" -maxdepth 1 -type f -name '*.tgz' -mtime +"$RETAIN_DAYS" -print -delete \
    | sed 's/^/  pruned /' || true
  echo "=== $(date '+%F %T %Z') backup done ==="
} >> "$log" 2>&1
