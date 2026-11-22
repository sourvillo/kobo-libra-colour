#!/bin/sh
# Host-only crash/hang/clean-shutdown tests.  All paths are redirected to a
# temporary root and reboot is explicitly suppressed.
set -eu
export LC_ALL=C

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOTS=""

cleanup() {
    for root in $ROOTS; do
        if [ -r "$root/watchdog.pid" ]; then
            pid="$(sed -n '1p' "$root/watchdog.pid" 2>/dev/null)"
            case "$pid" in ''|*[!0-9]*) ;; *)
                kill -CONT "$pid" 2>/dev/null || true
                kill -KILL "$pid" 2>/dev/null || true
                ;;
            esac
        fi
        rm -rf "$root"
    done
}
trap cleanup EXIT HUP INT TERM

fail() {
    echo "[FAIL] $*" >&2
    exit 1
}

ok() {
    echo "[OK] $*"
}

new_root() {
    ROOT="$(mktemp -d "${TMPDIR:-/tmp}/homerow-v3-watchdog-test.XXXXXX")"
    ROOTS="$ROOTS $ROOT"
    mkdir -p "$ROOT/base/state" "$ROOT/base/approved" "$ROOT/imageformats" "$ROOT/tmp"
    cp "$HERE/libryfront-safe-v3.so" "$ROOT/imageformats/libryfront.so"
    printf '%s\n' 'known-nickelmenu' > "$ROOT/imageformats/libnm.so"
    SHA="$(sha256sum "$ROOT/imageformats/libryfront.so" | awk '{print $1}')"

    HOMEROW_BASE="$ROOT/base"
    HOMEROW_STATE="$ROOT/base/state"
    HOMEROW_APPROVED="$ROOT/base/approved"
    HOMEROW_ACTIVE="$ROOT/imageformats/libryfront.so"
    HOMEROW_IMAGEFORMATS="$ROOT/imageformats"
    HOMEROW_LIBNM="$ROOT/imageformats/libnm.so"
    HOMEROW_LIBNM_FAILSAFE="$ROOT/imageformats/libnm.so.failsafe"
    HOMEROW_LIBNM_MD5="$(md5sum "$HOMEROW_LIBNM" | awk '{ print $1 }')"
    HOMEROW_LIBNM_SHA256="$(sha256sum "$HOMEROW_LIBNM" | awk '{ print $1 }')"
    HOMEROW_CONTENT_IDS="$ROOT/base/content-ids.txt"
    printf '%s\n' exact-ten-selection > "$HOMEROW_CONTENT_IDS"
    HOMEROW_CONTENT_IDS_SHA256="$(sha256sum "$HOMEROW_CONTENT_IDS" | awk '{ print $1 }')"
    HOMEROW_TMPDIR="$ROOT/tmp"
    HOMEROW_ARM="$ROOT/tmp/arm"
    HOMEROW_HEARTBEAT="$ROOT/tmp/heartbeat"
    HOMEROW_PID_FILE="$ROOT/watchdog.pid"
    HOMEROW_WATCHDOG_LOG="$ROOT/watchdog.log"
    HOMEROW_LIB="$HERE/state-lib.sh"
    HOMEROW_WATCHDOG_POLL_SECONDS=1
    HOMEROW_WATCHDOG_FIRST_HEARTBEAT_POLLS=4
    HOMEROW_WATCHDOG_RUNTIME_POLLS=8
    HOMEROW_WATCHDOG_STALL_POLLS=2
    HOMEROW_WATCHDOG_REQUIRE_MAPS=0
    HOMEROW_WATCHDOG_NO_REBOOT=1
    export HOMEROW_BASE HOMEROW_STATE HOMEROW_APPROVED HOMEROW_ACTIVE \
        HOMEROW_IMAGEFORMATS HOMEROW_LIBNM HOMEROW_LIBNM_FAILSAFE \
        HOMEROW_LIBNM_MD5 HOMEROW_LIBNM_SHA256 \
        HOMEROW_CONTENT_IDS HOMEROW_CONTENT_IDS_SHA256 \
        HOMEROW_TMPDIR HOMEROW_ARM HOMEROW_HEARTBEAT HOMEROW_PID_FILE \
        HOMEROW_WATCHDOG_LOG HOMEROW_LIB HOMEROW_WATCHDOG_POLL_SECONDS \
        HOMEROW_WATCHDOG_FIRST_HEARTBEAT_POLLS \
        HOMEROW_WATCHDOG_RUNTIME_POLLS HOMEROW_WATCHDOG_STALL_POLLS \
        HOMEROW_WATCHDOG_REQUIRE_MAPS HOMEROW_WATCHDOG_NO_REBOOT

    . "$HERE/state-lib.sh"
    homerow_write_boot_record "$HOMEROW_STATE/booting" "$SHA" \
        '11111111-1111-4111-8111-111111111111' once
    cp "$HOMEROW_STATE/booting" "$HOMEROW_ARM"
}

run_failure() {
    set +e
    sh "$HERE/watchdog.sh" "$@"
    RC=$?
    set -e
    [ "$RC" -eq 70 ] || fail "watchdog failure path returned $RC, expected 70"
    [ -s "$HOMEROW_STATE/disable" ] || fail "watchdog failure did not persist disable"
    [ ! -e "$HOMEROW_ACTIVE" ] || fail "watchdog failure left active image plugin"
    [ ! -e "$HOMEROW_STATE/booting" ] || fail "watchdog failure left reusable boot record"
}

new_root
run_failure sh -c 'exit 9'
grep -Fq 'first Nickel PID exited' "$HOMEROW_WATCHDOG_LOG" || \
    fail "immediate crash reason missing"
ok "first-PID crash disables, removes the active ELF, and breaks a restart loop"

new_root
run_failure sh -c '
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
    printf "%s\n" 1 > "$HOMEROW_HEARTBEAT"
    while :; do :; done
'
grep -Fq 'GUI heartbeat stalled after runtime acknowledgement' "$HOMEROW_WATCHDOG_LOG" || \
    fail "post-runtime hang reason missing"
ok "post-runtime GUI hang is externally detected and disabled"

new_root
set +e
sh "$HERE/watchdog.sh" sh -c '
    i=0
    while [ "$i" -lt 4 ]; do
        i=$((i + 1))
        printf "%s\n" "$i" > "$HOMEROW_HEARTBEAT.new"
        mv -f "$HOMEROW_HEARTBEAT.new" "$HOMEROW_HEARTBEAT"
        [ "$i" -ne 2 ] || cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
        sleep 1
    done
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/clean-shutdown"
    kill -STOP $$
'
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "normal stopped-PID shutdown returned $RC"
[ ! -e "$HOMEROW_STATE/disable" ] || fail "normal shutdown was falsely disabled"
pid="$(sed -n '1p' "$HOMEROW_PID_FILE")"
kill -CONT "$pid" 2>/dev/null || true
kill -TERM "$pid" 2>/dev/null || true
grep -Fq 'clean shutdown accepted' "$HOMEROW_WATCHDOG_LOG" || \
    fail "clean shutdown acceptance missing"
ok "matching runtime+clean records for the stopped first PID exit without reboot"

new_root
printf '%s\n' 'drifted-nickelmenu' > "$HOMEROW_LIBNM"
set +e
sh "$HERE/watchdog.sh" sh -c 'exit 0'
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "libnm pre-launch stock fallback returned $RC"
[ -s "$HOMEROW_STATE/disable" ] || fail "libnm drift did not persist disable"
[ ! -e "$HOMEROW_ACTIVE" ] || fail "libnm drift left active ryfront ELF"
grep -Fq 'optional NickelMenu libnm inventory conflicts or its exact MD5/SHA-256 drifted' \
    "$HOMEROW_WATCHDOG_LOG" || fail "libnm drift reason missing"
ok "armed launcher refuses native start when exact known libnm.so drifts"

new_root
mv "$HOMEROW_LIBNM" "$HOMEROW_LIBNM_FAILSAFE"
failsafe_sha="$(sha256sum "$HOMEROW_LIBNM_FAILSAFE" | awk '{ print $1 }')"
run_failure sh -c 'exit 9'
grep -Fq 'first Nickel PID exited' "$HOMEROW_WATCHDOG_LOG" || \
    fail "stock-only inventory did not launch the armed first PID"
[ ! -e "$HOMEROW_LIBNM" ] || fail "watchdog restored absent libnm.so"
[ "$(sha256sum "$HOMEROW_LIBNM_FAILSAFE" | awk '{ print $1 }')" = "$failsafe_sha" ] || \
    fail "watchdog mutated inert libnm.so.failsafe"
ok "failsafe-only inventory is accepted and never restored by safe-v3"

new_root
set +e
sh "$HERE/watchdog.sh" sh -c '
    i=0
    while [ "$i" -lt 6 ]; do
        i=$((i + 1))
        [ "$i" -ne 2 ] || mv "$HOMEROW_LIBNM" "$HOMEROW_LIBNM_FAILSAFE"
        [ "$i" -ne 4 ] || mv "$HOMEROW_LIBNM_FAILSAFE" "$HOMEROW_LIBNM"
        printf "%s\n" "$i" > "$HOMEROW_HEARTBEAT.new"
        mv -f "$HOMEROW_HEARTBEAT.new" "$HOMEROW_HEARTBEAT"
        [ "$i" -ne 2 ] || cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
        sleep 1
    done
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/clean-shutdown"
    kill -STOP $$
'
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "exact libnm rename transition returned $RC"
[ ! -e "$HOMEROW_STATE/disable" ] || fail "exact libnm rename transition falsely disabled"
[ -e "$HOMEROW_LIBNM" ] && [ ! -e "$HOMEROW_LIBNM_FAILSAFE" ] || \
    fail "exact libnm transition did not finish restored"
pid="$(sed -n '1p' "$HOMEROW_PID_FILE")"
kill -CONT "$pid" 2>/dev/null || true
kill -TERM "$pid" 2>/dev/null || true
ok "watchdog accepts exact atomic libnm.so -> .failsafe -> .so transition"

new_root
cp "$HOMEROW_LIBNM" "$HOMEROW_LIBNM_FAILSAFE"
set +e
sh "$HERE/watchdog.sh" sh -c 'exit 0'
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "libnm conflict stock fallback returned $RC"
[ -s "$HOMEROW_STATE/disable" ] || fail "libnm conflict did not persist disable"
[ ! -e "$HOMEROW_ACTIVE" ] || fail "libnm conflict left active ryfront ELF"
grep -Fq 'optional NickelMenu libnm inventory conflicts' \
    "$HOMEROW_WATCHDOG_LOG" || fail "libnm conflict reason missing"
ok "simultaneous exact active+failsafe libnm names fail closed"

new_root
printf '%s\n' drift >> "$HOMEROW_CONTENT_IDS"
set +e
sh "$HERE/watchdog.sh" sh -c 'exit 0'
RC=$?
set -e
[ "$RC" -eq 0 ] || fail "ContentID pre-launch stock fallback returned $RC"
[ -s "$HOMEROW_STATE/disable" ] || fail "ContentID drift did not persist disable"
[ ! -e "$HOMEROW_ACTIVE" ] || fail "ContentID drift left active ryfront ELF"
grep -Fq 'exact ordered ten-book ContentID list drifted' \
    "$HOMEROW_WATCHDOG_LOG" || fail "ContentID drift reason missing"
ok "armed launcher refuses native start when the exact ten-book selection drifts"

new_root
rm -f "$HOMEROW_STATE/booting" "$HOMEROW_ARM"
printf '%s\n' '#!/bin/sh' 'exit 1' > "$ROOT/no-rm"
chmod 0755 "$ROOT/no-rm"
mkdir "$ROOT/fake-bin"
cp "$ROOT/no-rm" "$ROOT/fake-bin/rm"
set +e
PATH="$ROOT/fake-bin:/usr/bin:/bin:/usr/sbin:/sbin" \
    sh "$HERE/watchdog.sh" sh -c 'printf launched > "$HOMEROW_TMPDIR/stock-launched"'
RC=$?
set -e
[ "$RC" -eq 70 ] || fail "unsafe stock_exec returned $RC instead of 70"
[ ! -e "$HOMEROW_TMPDIR/stock-launched" ] || \
    fail "stock_exec launched Nickel while active ELF survived cleanup"
[ -e "$HOMEROW_ACTIVE" ] || fail "stock_exec failure fixture unexpectedly removed active ELF"
grep -Fq 'stock_exec refused: active ryfront or arm survived cleanup' \
    "$HOMEROW_WATCHDOG_LOG" || fail "stock_exec cleanup refusal reason missing"
ok "watchdog never launches stock Nickel if active-ELF cleanup fails"

echo "PASS: safe-v3 external-watchdog matrix"
