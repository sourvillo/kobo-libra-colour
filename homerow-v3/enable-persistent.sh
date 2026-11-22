#!/bin/sh
# Deliberate promotion of one exact canary-tested ELF. The operator must type the
# approved SHA on the command line after reviewing real-device stability logs.
set -u

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
APPROVED="${HOMEROW_APPROVED:-$BASE/approved}"
ARM="${HOMEROW_ARM:-/tmp/homerow-v3/arm}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"

[ -r "$LIB" ] || { echo "homerow-v3: state library missing" >&2; exit 1; }
. "$LIB" || exit 1

approved_sha="$(homerow_read_sha_file "$STATE/approved.sha256")" || {
    echo "homerow-v3: no exact canary-approved artifact" >&2
    exit 1
}
canary_sha="$(homerow_read_sha_file "$STATE/canary-ok")" || canary_sha=""
artifact="$APPROVED/$approved_sha.so"
artifact_sha="$(homerow_sha256 "$artifact")" || artifact_sha=""

if [ "$approved_sha" != "$canary_sha" ] || [ "$approved_sha" != "$artifact_sha" ]; then
    echo "homerow-v3: approval, canary, and preserved artifact hashes differ" >&2
    exit 1
fi
if [ -f "$STATE/disable" ]; then
    echo "homerow-v3: emergency disable is set; request a new one-shot instead" >&2
    exit 1
fi
if [ -f "$STATE/booting" ] || [ -f "$ARM" ]; then
    echo "homerow-v3: current boot is/was armed; return to stock before promotion" >&2
    exit 1
fi
if [ "${1:-}" != "--confirm-sha" ] || [ "${2:-}" != "$approved_sha" ]; then
    echo "homerow-v3: promotion requires explicit confirmation:" >&2
    echo "  $0 --confirm-sha $approved_sha" >&2
    exit 1
fi

homerow_durable_remove "$STATE/request-once" || exit 1
homerow_durable_write_line "$STATE/enabled" "$approved_sha" || exit 1
echo "homerow-v3: persistent next-boot request bound to sha256=$approved_sha"
echo "homerow-v3: reboot manually; any unclean armed boot disables and invalidates it"
exit 0
