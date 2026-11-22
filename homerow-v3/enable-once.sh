#!/bin/sh
# Request exactly one v3 canary boot. No reboot is performed here.
set -u

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
APPROVED="${HOMEROW_APPROVED:-$BASE/approved}"
STAGED="${HOMEROW_STAGED:-$BASE/libryfront-safe-v3.so}"
MANIFEST="${HOMEROW_MANIFEST:-$BASE/artifact.sha256}"
CONTENT_IDS="${HOMEROW_CONTENT_IDS:-$BASE/content-ids.txt}"
ACTIVE="${HOMEROW_ACTIVE:-/usr/local/Kobo/imageformats/libryfront.so}"
IMAGEFORMATS="${HOMEROW_IMAGEFORMATS:-$(dirname "$ACTIVE")}"
LIBNM="${HOMEROW_LIBNM:-$IMAGEFORMATS/libnm.so}"
LIBNM_FAILSAFE="${HOMEROW_LIBNM_FAILSAFE:-$LIBNM.failsafe}"
LIBNM_MD5="${HOMEROW_LIBNM_MD5:-0fa9c5c0a4eca91eb3b04f4881ae4cfb}"
LIBNM_SHA256="${HOMEROW_LIBNM_SHA256:-49f9f4dc312467d7bb096a5f5032b3cd79b369ee299061b184fb7e6d5d4f6475}"
CONTENT_IDS_SHA256="${HOMEROW_CONTENT_IDS_SHA256:-14085cceae59b405ca91f329e8e810cbd5b487f7a391969b1428937e39ad2fae}"
ARM="${HOMEROW_ARM:-/tmp/homerow-v3/arm}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"

[ -r "$LIB" ] || { echo "homerow-v3: state library missing" >&2; exit 1; }
. "$LIB" || exit 1
mkdir -p "$STATE" "$APPROVED" || exit 1

if [ -f "$STATE/booting" ] || [ -f "$ARM" ]; then
    echo "homerow-v3: current boot is/was armed; reboot to stock before a new request" >&2
    exit 1
fi

manifest_sha="$(homerow_read_sha_file "$MANIFEST")" || {
    echo "homerow-v3: invalid package artifact.sha256" >&2
    exit 1
}
staged_sha="$(homerow_sha256 "$STAGED")" || {
    echo "homerow-v3: staged plugin missing or unreadable" >&2
    exit 1
}
if ! homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
        "$LIBNM_MD5" "$LIBNM_SHA256"; then
    echo "homerow-v3: optional libnm inventory conflicts or has unknown MD5/SHA-256" >&2
    exit 1
fi
if [ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" != "$CONTENT_IDS_SHA256" ]; then
    echo "homerow-v3: exact ordered ten-book ContentID list is required" >&2
    exit 1
fi
if [ "$manifest_sha" != "$staged_sha" ]; then
    echo "homerow-v3: staged plugin does not match its package manifest" >&2
    exit 1
fi

ids_size="$(wc -c < "$CONTENT_IDS" 2>/dev/null | awk '{ print $1 }')" || ids_size=0
ids_lines="$(awk 'length($0)>0 { n++ } END { print n+0 }' "$CONTENT_IDS" 2>/dev/null)" || ids_lines=0
case "$ids_size:$ids_lines" in *[!0-9:]*) ids_size=0; ids_lines=0 ;; esac
if [ ! -s "$CONTENT_IDS" ] || [ "$ids_size" -gt 4095 ] || [ "$ids_lines" -ne 10 ]; then
    echo "homerow-v3: content-ids.txt must contain exactly 10 bounded rootfs IDs" >&2
    exit 1
fi

# Invalidate every prior approval before removing disable. Request is written last.
homerow_durable_remove "$STATE/enabled" "$STATE/request-once" \
    "$STATE/approved.sha256" "$STATE/canary-ok" "$STATE/runtime-ok" \
    "$STATE/lifecycle-ok" "$STATE/clean-shutdown" "$APPROVED"/*.so \
    "$IMAGEFORMATS"/libryfront* || exit 1
homerow_durable_remove "$STATE/disable" "$STATE/disabled-reason" || exit 1
homerow_durable_write_line "$STATE/request-once" "$staged_sha" || exit 1

echo "homerow-v3: one-shot requested for sha256=$staged_sha"
echo "homerow-v3: reboot manually; persistent mode is not automatic"
exit 0
