#!/bin/sh
# Root/UBB emergency stop. Because the image plugin is linked NODELETE, deleting
# its ELF cannot unload code from a live Nickel. This command therefore ALWAYS
# requests a reboot and forcibly stops Nickel if the reboot path does not progress.

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
APPROVED="${HOMEROW_APPROVED:-$BASE/approved}"
ACTIVE="${HOMEROW_ACTIVE:-/usr/local/Kobo/imageformats/libryfront.so}"
IMAGEFORMATS="${HOMEROW_IMAGEFORMATS:-$(dirname "$ACTIVE")}"
TMPDIR="${HOMEROW_TMPDIR:-/tmp/homerow-v3}"

mkdir -p "$STATE" 2>/dev/null || {
    echo "homerow-v3: cannot create persistent disable state" >&2
    exit 1
}

tmp="$STATE/disable.new.$$"
rm -f "$tmp" 2>/dev/null || true
umask 077
printf '%s\n' 'manual-emergency-disable' > "$tmp" || exit 1
sync || exit 1
mv -f "$tmp" "$STATE/disable" || exit 1
sync || exit 1

reason_tmp="$STATE/disabled-reason.new.$$"
printf '%s\n' 'manual emergency disable; live NODELETE module requires reboot' > "$reason_tmp" 2>/dev/null && \
    sync && mv -f "$reason_tmp" "$STATE/disabled-reason" 2>/dev/null && sync || true

rm -f "$STATE/enabled" "$STATE/request-once" "$STATE/approved.sha256" \
      "$STATE/canary-ok" "$STATE/booting" "$STATE/runtime-ok" \
      "$STATE/runtime-ok.new" "$STATE/lifecycle-ok" \
      "$STATE/lifecycle-ok.new" "$STATE/clean-shutdown" \
      "$STATE/clean-shutdown.new" "$APPROVED"/*.so \
      "$IMAGEFORMATS"/libryfront* \
      "$TMPDIR/arm" "$TMPDIR/arm.new" "$TMPDIR/heartbeat" \
      "$TMPDIR/heartbeat.new" 2>/dev/null || true
sync

echo "homerow-v3: persistent disable committed; reboot is mandatory and requested"
/sbin/reboot 2>/dev/null || reboot 2>/dev/null || true

# A normal reboot usually terminates this shell. If it returns and Nickel remains,
# stop the NODELETE process before issuing the force-reboot fallback.
sleep 2
pids="$(pidof nickel 2>/dev/null)"
if [ -n "$pids" ]; then
    kill -TERM $pids 2>/dev/null || true
    sleep 1
    pids="$(pidof nickel 2>/dev/null)"
    [ -n "$pids" ] && kill -KILL $pids 2>/dev/null || true
fi
sync
/sbin/reboot -f 2>/dev/null || reboot -f 2>/dev/null || true
exit 1
