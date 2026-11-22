#!/bin/sh
# rcK calls this immediately before its stock SIGSTOP of Nickel. A clean record is
# granted only to the exact boot/artifact whose GUI-thread heartbeat advances. The
# script stops that same Nickel PID and verifies state T *before* committing the
# record, so a reset cannot land between a success marker and the actual stop.

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
ACTIVE="${HOMEROW_ACTIVE:-/usr/local/Kobo/imageformats/libryfront.so}"
IMAGEFORMATS="${HOMEROW_IMAGEFORMATS:-$(dirname "$ACTIVE")}"
LIBNM="${HOMEROW_LIBNM:-$IMAGEFORMATS/libnm.so}"
LIBNM_FAILSAFE="${HOMEROW_LIBNM_FAILSAFE:-$LIBNM.failsafe}"
LIBNM_MD5="${HOMEROW_LIBNM_MD5:-0fa9c5c0a4eca91eb3b04f4881ae4cfb}"
LIBNM_SHA256="${HOMEROW_LIBNM_SHA256:-49f9f4dc312467d7bb096a5f5032b3cd79b369ee299061b184fb7e6d5d4f6475}"
CONTENT_IDS="${HOMEROW_CONTENT_IDS:-$BASE/content-ids.txt}"
CONTENT_IDS_SHA256="${HOMEROW_CONTENT_IDS_SHA256:-14085cceae59b405ca91f329e8e810cbd5b487f7a391969b1428937e39ad2fae}"
TMPDIR="${HOMEROW_TMPDIR:-/tmp/homerow-v3}"
ARM="${HOMEROW_ARM:-$TMPDIR/arm}"
HEARTBEAT="${HOMEROW_HEARTBEAT:-$TMPDIR/heartbeat}"
PIDFILE="${HOMEROW_PID_FILE:-$TMPDIR/watchdog-nickel.pid}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"
PROBE_SECONDS="${HOMEROW_PROBE_SECONDS:-3}"

[ -r "$LIB" ] || exit 0
. "$LIB" || exit 0
homerow_durable_remove "$STATE/clean-shutdown" "$STATE/clean-shutdown.new" || exit 0

[ ! -f "$STATE/disable" ] || exit 0
homerow_record_valid "$STATE/booting" || exit 0
cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null || exit 0
cmp -s "$STATE/booting" "$ARM" 2>/dev/null || exit 0
sha="$(homerow_record_sha "$STATE/booting")" || exit 0
[ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" = "$sha" ] || exit 0
homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
    "$LIBNM_MD5" "$LIBNM_SHA256" || exit 0
[ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" = "$CONTENT_IDS_SHA256" ] || exit 0

set -- $(pidof nickel 2>/dev/null)
[ "$#" -eq 1 ] || exit 0
nickel_pid="$1"
case "$nickel_pid" in ''|*[!0-9]*) exit 0 ;; esac
watchdog_pid="$(sed -n '1p' "$PIDFILE" 2>/dev/null)"
case "$watchdog_pid" in ''|*[!0-9]*) exit 0 ;; esac
[ "$watchdog_pid" = "$nickel_pid" ] || exit 0

first="$(sed -n '1p' "$HEARTBEAT" 2>/dev/null)"
case "$first" in ''|*[!0-9]*) exit 0 ;; esac
sleep "$PROBE_SECONDS"
second="$(sed -n '1p' "$HEARTBEAT" 2>/dev/null)"
case "$second" in ''|*[!0-9]*) exit 0 ;; esac
[ "$second" -gt "$first" ] || exit 0

# Recheck every binding after the probe, then stop the exact process before success.
[ ! -f "$STATE/disable" ] || exit 0
cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null || exit 0
cmp -s "$STATE/booting" "$ARM" 2>/dev/null || exit 0
[ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" = "$sha" ] || exit 0
homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
    "$LIBNM_MD5" "$LIBNM_SHA256" || exit 0
[ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" = "$CONTENT_IDS_SHA256" ] || exit 0
if ! kill -STOP "$nickel_pid" 2>/dev/null; then
    exit 0
fi
tries=0
process_state=""
while [ "$tries" -lt 3 ]; do
    process_state="$(awk '/^State:/ { print $2; exit }' "/proc/$nickel_pid/status" 2>/dev/null)"
    case "$process_state" in T|t) break ;; esac
    tries=$((tries + 1))
    sleep 1
done
case "$process_state" in
    T|t) ;;
    *) exit 0 ;;
esac

# The marker is an exact durable copy and is granted only while that PID is stopped.
homerow_durable_copy "$STATE/booting" "$STATE/clean-shutdown" 0600 || exit 0
[ ! -f "$STATE/disable" ] || {
    homerow_durable_remove "$STATE/clean-shutdown" || true
    exit 0
}
cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null || {
    homerow_durable_remove "$STATE/clean-shutdown" || true
    exit 0
}
cmp -s "$STATE/booting" "$ARM" 2>/dev/null || {
    homerow_durable_remove "$STATE/clean-shutdown" || true
    exit 0
}
process_state="$(awk '/^State:/ { print $2; exit }' "/proc/$nickel_pid/status" 2>/dev/null)"
case "$process_state" in
    T|t) ;;
    *) homerow_durable_remove "$STATE/clean-shutdown" || true; exit 0 ;;
esac

printf '%s\n' "homerow-v3: exact clean-shutdown record committed; Nickel $nickel_pid stopped" \
    >> "$TMPDIR/guard.log" 2>/dev/null || true
exit 0
