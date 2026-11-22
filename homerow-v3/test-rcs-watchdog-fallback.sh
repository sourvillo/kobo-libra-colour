#!/bin/sh
# Host-only execution matrix for the exact generated rcS watchdog fallback.
# Absolute device paths are rewritten into mktemp before the block is executed.
set -eu
export LC_ALL=C

HERE="$(cd "$(dirname "$0")" && pwd)"
RCS_PATCH="$HERE/rcS-hook.patch"
ROOTS=""

cleanup() {
    for root in $ROOTS; do
        chmod 0755 "$root/imageformats" 2>/dev/null || true
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
    ROOT="$(mktemp -d "${TMPDIR:-/tmp}/homerow-v3-rcs-test.XXXXXX")"
    ROOTS="$ROOTS $ROOT"
    mkdir -p "$ROOT/base" "$ROOT/imageformats" "$ROOT/tmp" "$ROOT/bin"
    printf '%s\n' '#!/bin/sh' 'printf launched > "$HOMEROW_TEST_STOCK_MARKER"' \
        > "$ROOT/nickel"
    printf '%s\n' '#!/bin/sh' 'printf rebooted > "$HOMEROW_TEST_REBOOT_MARKER"' \
        > "$ROOT/reboot"
    chmod 0755 "$ROOT/nickel" "$ROOT/reboot"
    export HOMEROW_TEST_STOCK_MARKER="$ROOT/stock-launched"
    export HOMEROW_TEST_REBOOT_MARKER="$ROOT/rebooted"

    sed -n 's/^+//p' "$RCS_PATCH" | \
        sed -n '/^# HOMEROW_WATCHDOG_FALLBACK_BEGIN$/,/^# HOMEROW_WATCHDOG_FALLBACK_END$/p' | sed \
        -e "s#/usr/local/homerow-v3#$ROOT/base#g" \
        -e "s#/usr/local/Kobo/imageformats#$ROOT/imageformats#g" \
        -e "s#/tmp/homerow-v3#$ROOT/tmp#g" \
        -e "s#/usr/local/Kobo/nickel#$ROOT/nickel#g" \
        -e "s#/sbin/reboot#$ROOT/reboot#g" \
        > "$ROOT/block.sh"
    [ "$(grep -c 'HOMEROW_WATCHDOG_FALLBACK_BEGIN' "$ROOT/block.sh")" -eq 1 ] || \
        fail "generated rcS fallback markers are missing"
    sh -n "$ROOT/block.sh" || fail "rewritten rcS fallback has invalid syntax"
}

prime_armed() {
    printf stale-native > "$ROOT/imageformats/libryfront.so"
    printf exact-arm > "$ROOT/tmp/arm"
}

run_block() {
    sh "$ROOT/block.sh"
    tries=0
    while [ ! -e "$ROOT/stock-launched" ] && [ ! -e "$ROOT/rebooted" ] && \
          [ "$tries" -lt 100 ]; do
        sleep 0.1
        tries=$((tries + 1))
    done
}

assert_clean_stock() {
    [ -s "$ROOT/base/state/disable" ] || fail "$1 did not persist disable"
    [ ! -e "$ROOT/imageformats/libryfront.so" ] || fail "$1 left active ELF"
    [ ! -e "$ROOT/tmp/arm" ] || fail "$1 left tmpfs arm"
    [ -e "$ROOT/stock-launched" ] || fail "$1 did not launch stock Nickel"
    [ ! -e "$ROOT/rebooted" ] || fail "$1 rebooted despite successful cleanup"
}

new_root
prime_armed
run_block
assert_clean_stock "missing watchdog"
ok "missing watchdog disables, removes active+arm, then launches stock only"

new_root
prime_armed
printf '%s\n' '#!/bin/sh' 'exit 99' > "$ROOT/base/watchdog.sh"
chmod 0644 "$ROOT/base/watchdog.sh"
run_block
assert_clean_stock "non-executable watchdog"
ok "non-executable watchdog follows the same fail-closed stock path"

new_root
prime_armed
printf '%s\n' '#!/bin/sh' 'exit 1' > "$ROOT/bin/rm"
chmod 0755 "$ROOT/bin/rm"
PATH="$ROOT/bin:/usr/bin:/bin:/usr/sbin:/sbin" run_block
PATH=/usr/bin:/bin:/usr/sbin:/sbin
export PATH
[ ! -e "$ROOT/stock-launched" ] || fail "cleanup failure launched Nickel with active ELF"
[ -e "$ROOT/rebooted" ] || fail "cleanup failure did not request reboot"
[ -e "$ROOT/imageformats/libryfront.so" ] || fail "failure fixture unexpectedly removed ELF"
ok "surviving active ELF forbids Nickel launch and requests reboot"

echo "PASS: generated rcS missing/nonexec-watchdog matrix"
