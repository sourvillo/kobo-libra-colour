#!/bin/sh
# External safe-v3 Nickel launcher/watchdog.  On an exact armed boot this shell
# owns the first Nickel PID, detects exit or a stalled GUI heartbeat, persists
# disable before cleanup, stops that PID, and reboots.  When v3 is not armed it
# removes any stale active image plugin and execs stock Nickel directly.

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
APPROVED="${HOMEROW_APPROVED:-$BASE/approved}"
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
LOG="${HOMEROW_WATCHDOG_LOG:-$TMPDIR/watchdog.log}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"
POLL_SECONDS="${HOMEROW_WATCHDOG_POLL_SECONDS:-2}"
FIRST_HEARTBEAT_POLLS="${HOMEROW_WATCHDOG_FIRST_HEARTBEAT_POLLS:-20}"
RUNTIME_POLLS="${HOMEROW_WATCHDOG_RUNTIME_POLLS:-50}"
# Kobo can legitimately block its GUI thread for a while when opening a large
# EPUB/PDF or refreshing the e-ink panel.  Keep the external kill switch, but
# require 40 seconds of a frozen heartbeat before treating that as a hang.
STALL_POLLS="${HOMEROW_WATCHDOG_STALL_POLLS:-20}"
REQUIRE_MAPS="${HOMEROW_WATCHDOG_REQUIRE_MAPS:-1}"
NO_REBOOT="${HOMEROW_WATCHDOG_NO_REBOOT:-0}"

[ "$#" -ge 1 ] || exit 64

log() {
    printf '%s\n' "homerow-v3-watchdog: $*" >> "$LOG" 2>/dev/null || true
}

valid_uint() {
    case "$1" in ''|*[!0-9]*) return 1 ;; esac
    [ "$1" -ge 1 ] 2>/dev/null
}

stock_exec() {
    rm -f "$IMAGEFORMATS"/libryfront* "$ARM" \
        "$HEARTBEAT" "$HEARTBEAT.new" 2>/dev/null || true
    sync
    unsafe=0
    for plugin in "$IMAGEFORMATS"/libryfront*; do
        [ ! -e "$plugin" ] && [ ! -L "$plugin" ] || unsafe=1
    done
    [ ! -e "$ARM" ] && [ ! -L "$ARM" ] || unsafe=1
    if [ "$unsafe" -ne 0 ]; then
        log "stock_exec refused: active ryfront or arm survived cleanup"
        if [ "$NO_REBOOT" = 1 ]; then
            exit 70
        fi
        /sbin/reboot 2>/dev/null || reboot 2>/dev/null || true
        sleep 2
        /sbin/reboot -f 2>/dev/null || reboot -f 2>/dev/null || true
        exit 70
    fi
    log "stock_exec: v3 unarmed"
    exec "$@"
}

if [ ! -r "$LIB" ]; then
    stock_exec "$@"
fi
. "$LIB" 2>/dev/null || stock_exec "$@"

mkdir -p "$STATE" "$APPROVED" "$TMPDIR" 2>/dev/null || stock_exec "$@"

# Fresh /tmp makes this per boot.  A duplicate supervisor must never launch a
# second Nickel or disturb the first supervisor's exact arm.
if ! mkdir "$TMPDIR/watchdog.lock" 2>/dev/null; then
    log "duplicate launcher refused"
    exit 0
fi

for value in "$POLL_SECONDS" "$FIRST_HEARTBEAT_POLLS" "$RUNTIME_POLLS" "$STALL_POLLS"; do
    valid_uint "$value" || stock_exec "$@"
done

disable_before_launch() {
    reason="$1"
    homerow_durable_write_line "$STATE/disable" "watchdog:$reason" || true
    homerow_durable_write_line "$STATE/disabled-reason" "$reason" || true
    homerow_durable_remove "$STATE/enabled" "$STATE/request-once" \
        "$STATE/approved.sha256" "$STATE/canary-ok" "$STATE/booting" \
        "$STATE/runtime-ok" "$STATE/lifecycle-ok" "$STATE/clean-shutdown" \
        "$APPROVED"/*.so || true
    rm -f "$IMAGEFORMATS"/libryfront* "$ARM" "$HEARTBEAT" \
        "$HEARTBEAT.new" 2>/dev/null || true
    homerow_sync || true
    log "disabled before launch: $reason"
}

if [ -f "$STATE/disable" ]; then
    stock_exec "$@"
fi

if [ ! -e "$STATE/booting" ] && [ ! -e "$ARM" ]; then
    stock_exec "$@"
fi

if ! homerow_record_valid "$STATE/booting" || \
   ! cmp -s "$STATE/booting" "$ARM" 2>/dev/null; then
    disable_before_launch "booting and tmpfs arm records are not one exact valid record"
    stock_exec "$@"
fi

sha="$(homerow_record_sha "$STATE/booting")" || {
    disable_before_launch "armed record has invalid artifact hash"
    stock_exec "$@"
}
if [ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" != "$sha" ]; then
    disable_before_launch "active image plugin differs from armed artifact"
    stock_exec "$@"
fi
if ! homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
        "$LIBNM_MD5" "$LIBNM_SHA256"; then
    disable_before_launch "optional NickelMenu libnm inventory conflicts or its exact MD5/SHA-256 drifted"
    stock_exec "$@"
fi
if [ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" != "$CONTENT_IDS_SHA256" ]; then
    disable_before_launch "exact ordered ten-book ContentID list drifted"
    stock_exec "$@"
fi

"$@" &
nickel_pid=$!
case "$nickel_pid" in ''|*[!0-9]*)
    disable_before_launch "launcher did not obtain the first Nickel PID"
    stock_exec "$@"
    ;;
esac
printf '%s\n' "$nickel_pid" > "$PIDFILE.new.$$" 2>/dev/null && \
    mv -f "$PIDFILE.new.$$" "$PIDFILE" 2>/dev/null || true
log "armed_pid=$nickel_pid sha256=$sha"

process_state() {
    if [ -r "/proc/$nickel_pid/status" ]; then
        awk '/^State:/ { print $2; exit }' "/proc/$nickel_pid/status" 2>/dev/null
    else
        ps -o state= -p "$nickel_pid" 2>/dev/null | awk '{ print substr($1,1,1); exit }'
    fi
}

stop_exact_pid() {
    kill -TERM "$nickel_pid" 2>/dev/null || true
    tries=0
    while kill -0 "$nickel_pid" 2>/dev/null && [ "$tries" -lt 3 ]; do
        sleep 1
        tries=$((tries + 1))
    done
    kill -KILL "$nickel_pid" 2>/dev/null || true
}

panic() {
    reason="$1"
    # The durable kill switch is committed before removing the on-disk module or
    # touching the process.  A reset at any later instruction therefore boots stock.
    homerow_durable_write_line "$STATE/disable" "watchdog:$reason" || true
    homerow_durable_write_line "$STATE/disabled-reason" "$reason" || true
    rm -f "$ARM" "$HEARTBEAT" "$HEARTBEAT.new" 2>/dev/null || true
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    homerow_durable_remove "$STATE/enabled" "$STATE/request-once" \
        "$STATE/approved.sha256" "$STATE/canary-ok" "$STATE/booting" \
        "$STATE/runtime-ok" "$STATE/runtime-ok.new" "$STATE/lifecycle-ok" \
        "$STATE/lifecycle-ok.new" "$STATE/clean-shutdown" \
        "$STATE/clean-shutdown.new" "$APPROVED"/*.so || true
    homerow_sync || true
    log "PANIC pid=$nickel_pid reason=$reason"
    stop_exact_pid
    sync
    if [ "$NO_REBOOT" = 1 ]; then
        return 70
    fi
    /sbin/reboot 2>/dev/null || reboot 2>/dev/null || true
    sleep 2
    /sbin/reboot -f 2>/dev/null || reboot -f 2>/dev/null || true
    return 70
}

last_heartbeat=""
first_waits=0
runtime_waits=0
stall_waits=0
runtime_seen=0

while :; do
    sleep "$POLL_SECONDS"

    # rcK grants this only after stopping this exact Nickel process.  Check it
    # before heartbeat failure so a normal shutdown cannot race the watchdog.
    if cmp -s "$STATE/booting" "$STATE/clean-shutdown" 2>/dev/null && \
       cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null && \
       cmp -s "$STATE/booting" "$ARM" 2>/dev/null && \
       [ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" = "$sha" ] && \
       homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
           "$LIBNM_MD5" "$LIBNM_SHA256" && \
       [ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" = "$CONTENT_IDS_SHA256" ]; then
        case "$(process_state)" in
            T|t) log "clean shutdown accepted for stopped pid=$nickel_pid"; exit 0 ;;
        esac
    fi

    [ ! -f "$STATE/disable" ] || { panic "persistent disable appeared while armed"; exit $?; }
    homerow_record_valid "$STATE/booting" || { panic "boot record disappeared or became invalid"; exit $?; }
    cmp -s "$STATE/booting" "$ARM" 2>/dev/null || { panic "tmpfs arm diverged from boot record"; exit $?; }
    [ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" = "$sha" ] || { panic "active artifact hash changed"; exit $?; }
    homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
        "$LIBNM_MD5" "$LIBNM_SHA256" || {
        panic "optional NickelMenu libnm inventory conflicted or MD5/SHA-256 changed"; exit $?
    }
    [ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" = "$CONTENT_IDS_SHA256" ] || {
        panic "exact ordered ten-book ContentID list changed"; exit $?
    }

    state="$(process_state)"
    case "$state" in
        ''|Z|X|x) panic "first Nickel PID exited"; exit $? ;;
    esac

    current="$(sed -n '1p' "$HEARTBEAT" 2>/dev/null)"
    case "$current" in ''|*[!0-9]*) current="" ;; esac
    if [ -n "$current" ] && \
       { [ -z "$last_heartbeat" ] || [ "$current" -gt "$last_heartbeat" ]; }; then
        last_heartbeat="$current"
        stall_waits=0
    else
        stall_waits=$((stall_waits + 1))
    fi

    if [ "$runtime_seen" -eq 0 ]; then
        runtime_waits=$((runtime_waits + 1))
        if [ -z "$last_heartbeat" ]; then
            first_waits=$((first_waits + 1))
            if [ "$first_waits" -ge "$FIRST_HEARTBEAT_POLLS" ]; then
                panic "GUI heartbeat never started"; exit $?
            fi
        elif [ "$stall_waits" -ge "$STALL_POLLS" ]; then
            panic "GUI heartbeat stalled before runtime acknowledgement"; exit $?
        fi

        if cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null; then
            if [ "$REQUIRE_MAPS" = 1 ] && \
               ! grep -Fq "$ACTIVE" "/proc/$nickel_pid/maps" 2>/dev/null; then
                panic "runtime marker exists but exact plugin is absent from first Nickel PID"; exit $?
            fi
            runtime_seen=1
            stall_waits=0
            log "runtime acknowledgement accepted"
        elif [ "$runtime_waits" -ge "$RUNTIME_POLLS" ]; then
            panic "runtime acknowledgement deadline expired"; exit $?
        fi
    else
        cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null || {
            panic "runtime acknowledgement disappeared or diverged"; exit $?
        }
        if [ "$stall_waits" -ge "$STALL_POLLS" ]; then
            panic "GUI heartbeat stalled after runtime acknowledgement"; exit $?
        fi
    fi
done
