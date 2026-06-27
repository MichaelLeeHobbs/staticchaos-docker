#!/usr/bin/env bash
#
# watchdog.example.sh — self-heal watchdog for the Static Chaos MUD container.
#
# Why this exists: `restart: unless-stopped` only heals TRANSIENT crashes
# (crash -> restart -> boots clean). It cannot heal a DETERMINISTIC boot failure
# (bad data -> exit on load -> restart -> same exit), which just loops forever.
# This watchdog detects a sustained loop/outage and changes the inputs: it
# restores the newest area backup that actually boots, then restarts.
#
# Layered with the code fix (boot-time bad vnums are now non-fatal), this is a
# backstop for the rarer cases the code can't tolerate (e.g. a truncated area
# file from a crash mid-dump, a bad room vnum, or a non-data crash).
#
# Run from cron, e.g. /etc/cron.d/staticchaos-watchdog:
#   */2 * * * * root /opt/staticchaos/watchdog.sh
#
# Install (host-specific; the real copy is gitignored):
#   cp watchdog.example.sh watchdog.sh ; chmod +x watchdog.sh
#
# Alerts: host log only for now (STATE_DIR/watchdog.log). To add Discord/email,
# replace the alert() body.

set -uo pipefail

CONTAINER="staticchaos"
AREA_VOLUME="staticchaos_area"
BACKUP_DIR="/opt/backups/staticchaos"          # where backup-volumes.sh writes
STATE_DIR="/opt/backups/staticchaos"
STATE_FILE="$STATE_DIR/watchdog.state"
LOG="$STATE_DIR/watchdog.log"

GRACE_SECS=300        # how long trouble must persist before we act (ride out normal restarts/boots)
COOLDOWN_SECS=3600    # min seconds between auto-heal attempts (anti-thrash)
PORT=4000             # MUD port inside the container's network namespace (published on host)
VERIFY_WAIT=120       # seconds to wait for the port after a restore attempt
HELPER_IMG="alpine"   # tiny image used to read/write the volume

mkdir -p "$STATE_DIR"
now="$(date +%s)"

log()   { echo "$(date '+%F %T %Z') $*" >> "$LOG"; }
alert() { log "ALERT: $*"; }   # host-log only; swap for a webhook/email if desired

# --- state helpers (first_trouble / last_heal epochs) ---
first_trouble=0; last_heal=0
[ -f "$STATE_FILE" ] && . "$STATE_FILE" 2>/dev/null || true
save_state() { printf 'first_trouble=%s\nlast_heal=%s\n' "${first_trouble:-0}" "${last_heal:-0}" > "$STATE_FILE"; }

# --- port reachability (direct TCP connect; used to verify recovery) ---
port_up() { timeout 5 bash -c "exec 3<>/dev/tcp/127.0.0.1/$PORT" 2>/dev/null; }

# --- current container view ---
read_state() {
  docker inspect "$CONTAINER" \
    --format '{{.State.Status}}|{{if .State.Health}}{{.State.Health.Status}}{{else}}none{{end}}' \
    2>/dev/null || echo "missing|none"
}

restore_and_verify() {  # $1 = backup tarball path
  local tgz="$1"
  log "  trying restore: $(basename "$tgz")"
  docker stop "$CONTAINER" >/dev/null 2>&1 || true
  if ! docker run --rm -v "${AREA_VOLUME}:/dst" -v "${BACKUP_DIR}:/src:ro" "$HELPER_IMG" \
        sh -c "rm -rf /dst/* /dst/..?* /dst/.[!.]* 2>/dev/null; tar xzf '/src/$(basename "$tgz")' -C /dst" >/dev/null 2>&1; then
    log "  restore FAILED to extract $(basename "$tgz")"
    return 1
  fi
  docker start "$CONTAINER" >/dev/null 2>&1 || true
  local waited=0
  while [ "$waited" -lt "$VERIFY_WAIT" ]; do
    sleep 10; waited=$((waited+10))
    if port_up; then log "  RECOVERED via $(basename "$tgz") after ${waited}s"; return 0; fi
  done
  log "  no port after ${VERIFY_WAIT}s with $(basename "$tgz")"
  return 1
}

auto_heal() {
  alert "auto-heal starting: $CONTAINER unhealthy >= ${GRACE_SECS}s"
  # 1. Preserve the current (bad) world for forensics before we overwrite it.
  local stamp failed
  stamp="$(date +%Y-%m-%d_%H%M%S)"
  failed="area-FAILED-${stamp}.tgz"
  docker stop "$CONTAINER" >/dev/null 2>&1 || true
  docker run --rm -v "${AREA_VOLUME}:/src:ro" -v "${BACKUP_DIR}:/dst" "$HELPER_IMG" \
    tar czf "/dst/${failed}" -C /src . >/dev/null 2>&1 \
    && log "  saved failed world -> ${failed}" \
    || log "  WARN could not snapshot failed world"

  # 2. Walk backups newest -> oldest, restore the first that boots.
  local tgz
  for tgz in $(ls -t "${BACKUP_DIR}"/${AREA_VOLUME}-*.tgz 2>/dev/null); do
    if restore_and_verify "$tgz"; then
      last_heal="$now"; first_trouble=0; save_state
      alert "auto-heal SUCCESS: restored $(basename "$tgz"); failed copy kept as ${failed}"
      return 0
    fi
  done

  # 3. Nothing booted — leave it for a human, don't thrash.
  last_heal="$now"; save_state
  alert "auto-heal FAILED: no area backup booted. MANUAL INTERVENTION NEEDED. Failed world: ${failed}"
  return 1
}

# ------------------------------------------------------------------ main
IFS='|' read -r status health <<< "$(read_state)"

if [ "$status" = "running" ] && { [ "$health" = "healthy" ] || { [ "$health" = "none" ] && port_up; }; }; then
  # Healthy: clear any trouble timer.
  if [ "${first_trouble:-0}" -ne 0 ]; then log "recovered to healthy"; fi
  first_trouble=0; save_state
  exit 0
fi

# Boot grace: don't treat a still-starting container as trouble.
if [ "$health" = "starting" ]; then exit 0; fi

# Trouble (unhealthy / restarting / exited / missing).
if [ "${first_trouble:-0}" -eq 0 ]; then
  first_trouble="$now"; save_state
  log "trouble detected: status=$status health=$health (starting grace timer)"
  exit 0
fi

trouble_for=$(( now - first_trouble ))
if [ "$trouble_for" -lt "$GRACE_SECS" ]; then
  log "trouble ${trouble_for}s (< ${GRACE_SECS}s grace): status=$status health=$health"
  exit 0
fi

# Sustained trouble. Respect cooldown so a still-broken box isn't healed in a loop.
if [ "${last_heal:-0}" -ne 0 ] && [ $(( now - last_heal )) -lt "$COOLDOWN_SECS" ]; then
  alert "still unhealthy ${trouble_for}s but within cooldown since last heal; NOT re-healing (needs human)"
  exit 0
fi

auto_heal
