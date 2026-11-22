#!/bin/sh
# Install an explicit list of exactly 10 UTF-8 Nickel ContentIDs onto rootfs. This tool
# never opens KoboReader.sqlite and is never part of boot; IDs must be prepared or
# inspected separately while the feature is disabled.
set -u

BASE="${HOMEROW_BASE:-/usr/local/homerow-v3}"
STATE="${HOMEROW_STATE:-$BASE/state}"
DEST="${HOMEROW_CONTENT_IDS:-$BASE/content-ids.txt}"
ARM="${HOMEROW_ARM:-/tmp/homerow-v3/arm}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"
SOURCE="${1:-}"
EXPECTED_SHA256="${HOMEROW_CONTENT_IDS_SHA256:-14085cceae59b405ca91f329e8e810cbd5b487f7a391969b1428937e39ad2fae}"

[ -n "$SOURCE" ] && [ -f "$SOURCE" ] || {
    echo "usage: $0 /path/to/content-ids.txt" >&2
    exit 1
}
[ -r "$LIB" ] || { echo "homerow-v3: state library missing" >&2; exit 1; }
. "$LIB" || exit 1
mkdir -p "$BASE" "$STATE" || exit 1

if [ -f "$STATE/booting" ] || [ -f "$ARM" ]; then
    echo "homerow-v3: refusing to change IDs during an armed boot" >&2
    exit 1
fi

size="$(wc -c < "$SOURCE" 2>/dev/null | awk '{ print $1 }')" || size=0
case "$size" in ''|*[!0-9]*) size=0 ;; esac
[ "$size" -ge 1 ] && [ "$size" -le 4095 ] || {
    echo "homerow-v3: list must be 1..4095 bytes" >&2
    exit 1
}

if ! LC_ALL=C awk '
    BEGIN { good=1; n=0 }
    {
        sub(/\r$/, "")
        if (length($0)==0) next
        if (length($0)>511 || $0 ~ /[[:cntrl:]]/ || seen[$0]++) good=0
        n++
    }
    END { if (!good || n!=10) exit 1 }
' "$SOURCE"; then
    echo "homerow-v3: expected exactly 10 unique non-control IDs, each <=511 bytes" >&2
    exit 1
fi

source_sha="$(homerow_sha256 "$SOURCE")" || source_sha=""
[ "$source_sha" = "$EXPECTED_SHA256" ] || {
    echo "homerow-v3: refusing any list except the exact ordered audited ten IDs" >&2
    exit 1
}

homerow_durable_copy "$SOURCE" "$DEST" 0600 || exit 1
echo "homerow-v3: installed bounded rootfs ContentID list at $DEST"
exit 0
