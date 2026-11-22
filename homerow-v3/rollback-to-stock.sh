#!/bin/sh
# Durable full rollback: remove every active ryfront/arm first, restore the exact
# official rcS/rcK backups, and reboot so no NODELETE mapping can survive.
set -u
export LC_ALL=C

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
ACTIVE="${HOMEROW_ACTIVE:-/usr/local/Kobo/imageformats/libryfront.so}"
IMAGEFORMATS="${HOMEROW_IMAGEFORMATS:-$(dirname "$ACTIVE")}"
TMPDIR="${HOMEROW_TMPDIR:-/tmp/homerow-v3}"
RCS="${HOMEROW_RCS:-/etc/init.d/rcS}"
RCK="${HOMEROW_RCK:-/etc/init.d/rcK}"
STOCK_RCS="${HOMEROW_STOCK_RCS:-$BASE/stock-4.44.23552/rcS}"
STOCK_RCK="${HOMEROW_STOCK_RCK:-$BASE/stock-4.44.23552/rcK}"
VERSION_FILE="${HOMEROW_VERSION_FILE:-/mnt/onboard/.kobo/version}"
HOST_TEST="${HOMEROW_HOST_TEST:-0}"
SERIAL="${KOBO_SERIAL:-}"
FIRMWARE=4.44.23552
STOCK_RCS_SHA=0b3d65342ed2ba0e7715a40d9773ae09c6cc80bcf7248172ce46d1db426215a9
STOCK_RCK_SHA=20b060d54b262169a47eb4ba41a8ebc67ecb6508c75fd3fe27a898467ce1a070

fail() {
    echo "HOMEROW SAFE-V3 STOCK ROLLBACK REFUSED: $*" >&2
    exit 1
}

sha_file() {
    sha256sum "$1" 2>/dev/null | awk '{ print $1 }'
}

[ "$HOST_TEST" = 1 ] || [ "$(id -u)" = 0 ] || fail "root is required"
version="$(sed -n '1p' "$VERSION_FILE" 2>/dev/null)" || fail "device version unreadable"
version_serial=${version%%,*}
version_tail=${version#*,}
version_tail=${version_tail#*,}
version_firmware=${version_tail%%,*}
[ "$version_serial" = "$SERIAL" ] || fail "serial is $version_serial, expected $SERIAL"
[ "$version_firmware" = "$FIRMWARE" ] || \
    fail "firmware is $version_firmware, expected $FIRMWARE"
[ "$(sha_file "$STOCK_RCS")" = "$STOCK_RCS_SHA" ] || fail "exact stock rcS backup missing"
[ "$(sha_file "$STOCK_RCK")" = "$STOCK_RCK_SHA" ] || fail "exact stock rcK backup missing"

# Disable is the first mutation. A reset from here onward cannot arm safe-v3.
mkdir -p "$STATE" "$TMPDIR" || fail "cannot create disable state"
umask 077
disable_tmp="$STATE/disable.new.$$"
printf '%s\n' 'manual-full-stock-rollback' > "$disable_tmp" || fail "cannot stage disable"
sync || fail "cannot sync staged disable"
mv -f "$disable_tmp" "$STATE/disable" || fail "cannot commit disable"
sync || fail "cannot sync committed disable"

rm -f "$STATE/enabled" "$STATE/request-once" "$STATE/approved.sha256" \
    "$STATE/canary-ok" "$STATE/booting" "$STATE/runtime-ok" \
    "$STATE/runtime-ok.new" "$STATE/lifecycle-ok" "$STATE/lifecycle-ok.new" \
    "$STATE/clean-shutdown" "$STATE/clean-shutdown.new" \
    "$BASE/approved"/*.so "$IMAGEFORMATS"/libryfront* \
    "$TMPDIR/arm" "$TMPDIR/arm.new" "$TMPDIR/heartbeat" \
    "$TMPDIR/heartbeat.new" 2>/dev/null || true
sync

unsafe=0
for plugin in "$IMAGEFORMATS"/libryfront*; do
    [ ! -e "$plugin" ] && [ ! -L "$plugin" ] || unsafe=1
done
[ ! -e "$TMPDIR/arm" ] && [ ! -L "$TMPDIR/arm" ] || unsafe=1
[ "$unsafe" -eq 0 ] || fail "active ryfront or arm survived; init scripts left unchanged"

restore_one() {
    source_file=$1
    target=$2
    expected=$3
    temporary="$target.stock-rollback.new.$$"
    rm -f "$temporary" 2>/dev/null || true
    cp "$source_file" "$temporary" || return 1
    chmod 0755 "$temporary" || return 1
    [ "$(sha_file "$temporary")" = "$expected" ] || return 1
    sync || return 1
    mv -f "$temporary" "$target" || return 1
    sync || return 1
    [ "$(sha_file "$target")" = "$expected" ] || return 1
}

# With active code already proven absent, either intermediate hybrid is safe.
restore_one "$STOCK_RCK" "$RCK" "$STOCK_RCK_SHA" || fail "rcK restore failed"
restore_one "$STOCK_RCS" "$RCS" "$STOCK_RCS_SHA" || fail "rcS restore failed"
[ "$(sha_file "$RCK")" = "$STOCK_RCK_SHA" ] || fail "final rcK verification failed"
[ "$(sha_file "$RCS")" = "$STOCK_RCS_SHA" ] || fail "final rcS verification failed"
sync

echo "HOMEROW_SAFE_V3_ROLLED_BACK_TO_EXACT_STOCK_INIT"
echo "libnm.so and libnm.so.failsafe were not moved, restored, or modified."
if [ "$HOST_TEST" = 1 ]; then
    exit 0
fi
/sbin/reboot 2>/dev/null || reboot 2>/dev/null || true
sleep 2
/sbin/reboot -f 2>/dev/null || reboot -f 2>/dev/null || true
exit 1
