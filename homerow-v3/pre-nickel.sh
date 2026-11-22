#!/bin/sh
# Synchronous safe-v3 gate. This must be called by rcS immediately before Nickel.
# It always exits zero so a gate defect cannot prevent the stock reader UI.

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
TMPDIR="${HOMEROW_TMPDIR:-/tmp/homerow-v3}"
STAMP="${HOMEROW_STAMP:-$TMPDIR/pre-nickel.done}"
ARM="${HOMEROW_ARM:-$TMPDIR/arm}"
HEARTBEAT="${HOMEROW_HEARTBEAT:-$TMPDIR/heartbeat}"
TMPLOG="${HOMEROW_TMPLOG:-$TMPDIR/guard.log}"
ROOTLOG="${HOMEROW_ROOTLOG:-$BASE/guard.log}"
LIB="${HOMEROW_LIB:-$BASE/state-lib.sh}"
UUID_FILE="${HOMEROW_UUID_FILE:-/proc/sys/kernel/random/uuid}"

[ -e "$STAMP" ] && exit 0

# Background: refresh the Reading-Life stat-block sidecar once the DB is mounted (never blocks boot,
# never affects the gate). Detached so it survives this script's exit.
if [ -x /usr/local/koboviz/koboviz-gen.sh ]; then
    nohup sh -c 'n=0; while [ $n -lt 60 ] && [ ! -r /mnt/onboard/.kobo/KoboReader.sqlite ]; do sleep 1; n=$((n+1)); done; [ -r /mnt/onboard/.kobo/KoboReader.sqlite ] && sh /usr/local/koboviz/koboviz-gen.sh' >/dev/null 2>&1 &
fi

mkdir -p "$TMPDIR" 2>/dev/null || {
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    sync
    exit 0
}
rm -f "$ARM" "$ARM.new" "$HEARTBEAT" "$HEARTBEAT.new" 2>/dev/null || true
: > "$STAMP" 2>/dev/null || {
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    sync
    exit 0
}

if [ ! -r "$LIB" ]; then
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    sync
    exit 0
fi
. "$LIB" || {
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    sync
    exit 0
}

mkdir -p "$STATE" "$APPROVED" 2>/dev/null || {
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    homerow_sync
    exit 0
}
homerow_sync

log() {
    local line="homerow-v3: $*"
    printf '%s\n' "$line" >> "$TMPLOG" 2>/dev/null || true
    printf '%s\n' "$line" >> "$ROOTLOG" 2>/dev/null || true
}

remove_active() {
    # Remove every historical ryfront spelling, but preserve unrelated known
    # plugins such as NickelMenu's libnm.so and the stock libkimg.so.
    rm -f "$IMAGEFORMATS"/libryfront* 2>/dev/null || true
    homerow_sync
}

invalidate_approval() {
    rm -f "$STATE/enabled" "$STATE/request-once" "$STATE/approved.sha256" \
          "$STATE/canary-ok" "$STATE/booting" "$STATE/runtime-ok" \
          "$STATE/pre-runtime-strikes" "$STATE/unclean-strikes" \
          "$STATE/runtime-ok.new" "$STATE/lifecycle-ok" \
          "$STATE/lifecycle-ok.new" "$STATE/clean-shutdown" \
          "$STATE/clean-shutdown.new" "$APPROVED"/*.so 2>/dev/null || true
    homerow_sync
}

disable_all() {
    local reason="$1"
    # Persist the kill switch first. A reset during later cleanup still boots stock.
    homerow_durable_write_line "$STATE/disable" "auto:$reason" || true
    homerow_durable_write_line "$STATE/disabled-reason" "$reason" || true
    rm -f "$ARM" "$HEARTBEAT" 2>/dev/null || true
    remove_active
    invalidate_approval
    log "disabled: $reason"
}

read_sha_marker() {
    homerow_read_sha_file "$1"
}

# Emergency disable wins over every acknowledgement or activation request.
if [ -f "$STATE/disable" ]; then
    rm -f "$STATE/enabled" "$STATE/request-once" 2>/dev/null || true
    remove_active
    log "persistent disable marker present; starting stock Nickel"
    exit 0
fi

# NickelHook may atomically rename an exact libnm.so to libnm.so.failsafe while
# loaded NODELETE, then restore it later. Accept none or exactly one exact name;
# never restore it ourselves, and reject drift or two conflicting names.
if ! homerow_optional_libnm_valid "$LIBNM" "$LIBNM_FAILSAFE" \
        "$LIBNM_MD5" "$LIBNM_SHA256"; then
    disable_all "optional NickelMenu libnm inventory conflicts or its exact MD5/SHA-256 drifted"
    exit 0
fi
if [ "$(homerow_sha256 "$CONTENT_IDS" 2>/dev/null)" != "$CONTENT_IDS_SHA256" ]; then
    disable_all "exact ordered ten-book ContentID list drifted"
    exit 0
fi

canary_passed=0
if [ -f "$STATE/booting" ]; then
    previous_mode="$(homerow_record_mode "$STATE/booting")" || {
        disable_all "previous boot record has invalid mode"
        exit 0
    }
    if ! homerow_record_valid "$STATE/booting"; then
        disable_all "previous boot record is invalid"
        exit 0
    fi

    previous_sha="$(homerow_record_sha "$STATE/booting")" || {
        disable_all "previous boot record has invalid artifact hash"
        exit 0
    }
    active_sha="$(homerow_sha256 "$ACTIVE")" || active_sha=""
    if [ "$active_sha" != "$previous_sha" ]; then
        disable_all "active artifact no longer matches the completed boot record"
        exit 0
    fi

    # Note: genuine crashes/stalls are caught immediately by watchdog.sh (which writes $STATE/disable
    # before reboot). Reaching here without runtime-ok in persistent mode usually means power died or
    # reboot happened during the first 30s of boot; allow up to 3 strikes before disabling permanently.
    if ! cmp -s "$STATE/booting" "$STATE/runtime-ok" 2>/dev/null; then
        if [ "$previous_mode" = "persistent" ]; then
            pr_strikes="$(sed -n '1p' "$STATE/pre-runtime-strikes" 2>/dev/null)"
            case "$pr_strikes" in ''|*[!0-9]*) pr_strikes=0 ;; esac
            pr_strikes=$((pr_strikes + 1))
            if [ "$pr_strikes" -ge 3 ]; then
                disable_all "previous armed boot did not reach healthy runtime (3 consecutive strikes)"
                exit 0
            fi
            homerow_durable_write_line "$STATE/pre-runtime-strikes" "$pr_strikes" || true
            log "persistent boot interrupted before runtime-ok; tolerated (strike $pr_strikes of 3)"
        else
            disable_all "previous armed boot did not reach healthy runtime"
            exit 0
        fi
    elif [ "$previous_mode" = "once" ]; then
        # Strict canary: promotion still requires BOTH a clean shutdown AND the Home-away-Home lifecycle.
        if ! cmp -s "$STATE/booting" "$STATE/clean-shutdown" 2>/dev/null || \
           ! cmp -s "$STATE/booting" "$STATE/lifecycle-ok" 2>/dev/null; then
            disable_all "one-shot canary did not complete clean shutdown and lifecycle"
            exit 0
        fi
        approved_file="$APPROVED/$previous_sha.so"
        if ! homerow_durable_copy "$ACTIVE" "$approved_file" 0644 || \
           [ "$(homerow_sha256 "$approved_file" 2>/dev/null)" != "$previous_sha" ] || \
           ! homerow_durable_write_line "$STATE/approved.sha256" "$previous_sha" || \
           ! homerow_durable_write_line "$STATE/canary-ok" "$previous_sha"; then
            disable_all "failed to preserve exact canary-tested artifact"
            exit 0
        fi
        canary_passed=1
        rm -f "$STATE/request-once" "$STATE/enabled" 2>/dev/null || true
        remove_active
        log "one-shot passed runtime plus clean shutdown; exact artifact approved"
    else
        rm -f "$STATE/pre-runtime-strikes" 2>/dev/null || true
        # PERSISTENT (already canary-approved): reaching healthy runtime is enough to re-arm. A missing
        # clean-shutdown (unclean power-off / dead battery / forced reboot) is TOLERATED so the pager
        # does not vanish on every hard power-off. A bounded strike counter still halts a genuine
        # crash-loop that somehow reaches runtime each time; a clean shutdown resets the counter.
        if cmp -s "$STATE/booting" "$STATE/clean-shutdown" 2>/dev/null; then
            rm -f "$STATE/unclean-strikes" 2>/dev/null || true
            log "persistent boot passed runtime plus clean shutdown"
        else
            strikes="$(sed -n '1p' "$STATE/unclean-strikes" 2>/dev/null)"
            case "$strikes" in ''|*[!0-9]*) strikes=0 ;; esac
            strikes=$((strikes + 1))
            if [ "$strikes" -ge 8 ]; then
                disable_all "persistent mode: too many consecutive unclean shutdowns"
                exit 0
            fi
            homerow_durable_write_line "$STATE/unclean-strikes" "$strikes" || true
            log "persistent boot reached runtime; unclean shutdown tolerated (strike $strikes of 8)"
        fi
    fi
fi

# Consume prior-boot records as one durable batch. A later reset cannot reuse them.
homerow_durable_remove "$STATE/booting" "$STATE/runtime-ok" \
    "$STATE/runtime-ok.new" "$STATE/lifecycle-ok" "$STATE/lifecycle-ok.new" \
    "$STATE/clean-shutdown" "$STATE/clean-shutdown.new" || true

if [ "$canary_passed" -eq 1 ]; then
    log "returning to stock after one-shot; promotion remains manual"
    exit 0
fi

mode="none"
sha=""
source_file=""

if [ -f "$STATE/request-once" ]; then
    mode="once"
    sha="$(read_sha_marker "$STATE/request-once")" || {
        disable_all "one-shot request has invalid artifact hash"
        exit 0
    }
    manifest_sha="$(read_sha_marker "$MANIFEST")" || manifest_sha=""
    staged_sha="$(homerow_sha256 "$STAGED")" || staged_sha=""
    if [ "$sha" != "$manifest_sha" ] || [ "$sha" != "$staged_sha" ]; then
        disable_all "one-shot request, package manifest, and staged artifact differ"
        exit 0
    fi
    source_file="$STAGED"
    rm -f "$STATE/request-once" "$STATE/enabled" 2>/dev/null || true
    homerow_sync
elif [ -f "$STATE/enabled" ]; then
    mode="persistent"
    sha="$(read_sha_marker "$STATE/enabled")" || {
        disable_all "persistent marker has invalid artifact hash"
        exit 0
    }
    approved_sha="$(read_sha_marker "$STATE/approved.sha256")" || approved_sha=""
    canary_sha="$(read_sha_marker "$STATE/canary-ok")" || canary_sha=""
    source_file="$APPROVED/$sha.so"
    source_sha="$(homerow_sha256 "$source_file")" || source_sha=""
    if [ "$sha" != "$approved_sha" ] || [ "$sha" != "$canary_sha" ] || \
       [ "$sha" != "$source_sha" ]; then
        disable_all "persistent mode is not bound to one exact approved artifact"
        exit 0
    fi
fi

if [ "$mode" = "none" ]; then
    remove_active
    log "disabled by default; starting stock Nickel"
    exit 0
fi

ids_size="$(wc -c < "$CONTENT_IDS" 2>/dev/null | awk '{ print $1 }')" || ids_size=0
case "$ids_size" in ''|*[!0-9]*) ids_size=0 ;; esac
if [ ! -s "$CONTENT_IDS" ] || [ "$ids_size" -gt 4095 ]; then
    disable_all "rootfs content-ids.txt is missing, empty, or oversized"
    exit 0
fi
if ! LC_ALL=C awk '
    BEGIN { good=1; n=0 }
    { sub(/\r$/, ""); if (length($0)==0) next;
      if (length($0)>511 || $0 ~ /[[:cntrl:]]/ || seen[$0]++) good=0; n++ }
    END { if (!good || n!=10) exit 1 }
' "$CONTENT_IDS"; then
    disable_all "content-ids.txt is not exactly ten unique bounded IDs"
    exit 0
fi

# Always replace ACTIVE, even if a stale non-empty v22/v2 library is present.
active_tmp="$ACTIVE.new.$$"
for legacy in "$IMAGEFORMATS"/libryfront*; do
    [ "$legacy" = "$ACTIVE" ] || rm -f "$legacy" 2>/dev/null || true
done
if ! cp "$source_file" "$active_tmp" 2>/dev/null; then
    disable_all "failed to stage exact active artifact"
    exit 0
fi
chmod 0644 "$active_tmp" 2>/dev/null || true
homerow_sync
if ! mv -f "$active_tmp" "$ACTIVE" 2>/dev/null; then
    disable_all "failed to atomically replace active artifact"
    exit 0
fi
homerow_sync
if [ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" != "$sha" ]; then
    disable_all "post-replace active artifact hash mismatch"
    exit 0
fi

token="$(tr -d '\n' < "$UUID_FILE" 2>/dev/null)"
case "$token" in
    ''|*[!0-9a-f-]*) token="" ;;
esac
if [ "${#token}" -lt 8 ] || [ "${#token}" -gt 64 ]; then
    disable_all "kernel boot nonce unavailable"
    exit 0
fi

rm -f "$STATE/runtime-ok" "$STATE/lifecycle-ok" "$STATE/clean-shutdown" 2>/dev/null || true
homerow_sync
if ! homerow_write_boot_record "$STATE/booting" "$sha" "$token" "$mode"; then
    disable_all "failed to persist exact boot record"
    exit 0
fi

# Arm tmpfs last, after a final emergency check and a second active-hash check.
if [ -f "$STATE/disable" ] || \
   [ "$(homerow_sha256 "$ACTIVE" 2>/dev/null)" != "$sha" ]; then
    disable_all "disable race or active hash changed before arming"
    exit 0
fi
if ! cp "$STATE/booting" "$ARM.new" 2>/dev/null || \
   ! mv -f "$ARM.new" "$ARM" 2>/dev/null || \
   ! cmp -s "$STATE/booting" "$ARM" 2>/dev/null; then
    disable_all "failed to create exact per-boot tmpfs arm record"
    exit 0
fi
if [ -f "$STATE/disable" ]; then
    disable_all "emergency marker appeared during final arm"
    exit 0
fi

log "armed $mode boot for sha256=$sha"
exit 0
