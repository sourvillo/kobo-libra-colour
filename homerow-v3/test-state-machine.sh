#!/bin/sh
# Host-only transition tests. Every path is redirected into mktemp; this never
# writes a Kobo mount and never invokes emergency-disable or reboot.
set -eu
export LC_ALL=C

HERE="$(cd "$(dirname "$0")" && pwd)"
ARTIFACT_SHA="$(sed -n '1p' "$HERE/artifact.sha256")"
ROOTS=""

cleanup() {
    for root in $ROOTS; do rm -rf "$root"; done
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
    ROOT="$(mktemp -d "${TMPDIR:-/tmp}/homerow-v3-test.XXXXXX")"
    ROOTS="$ROOTS $ROOT"
    mkdir -p "$ROOT/base/state" "$ROOT/base/approved" "$ROOT/imageformats"
    printf '%s\n' 'known-nickelmenu' > "$ROOT/imageformats/libnm.so"
    i=1
    : > "$ROOT/base/content-ids.txt"
    while [ "$i" -le 10 ]; do
        printf 'urn:homerow-test:book-%s\n' "$i" >> "$ROOT/base/content-ids.txt"
        i=$((i + 1))
    done
    printf '%s\n' '11111111-1111-4111-8111-111111111111' > "$ROOT/uuid"

    export HOMEROW_BASE="$ROOT/base"
    export HOMEROW_STATE="$ROOT/base/state"
    export HOMEROW_APPROVED="$ROOT/base/approved"
    export HOMEROW_STAGED="$HERE/libryfront-safe-v3.so"
    export HOMEROW_MANIFEST="$HERE/artifact.sha256"
    export HOMEROW_CONTENT_IDS="$ROOT/base/content-ids.txt"
    export HOMEROW_ACTIVE="$ROOT/imageformats/libryfront.so"
    export HOMEROW_IMAGEFORMATS="$ROOT/imageformats"
    export HOMEROW_LIBNM="$ROOT/imageformats/libnm.so"
    export HOMEROW_LIBNM_FAILSAFE="$ROOT/imageformats/libnm.so.failsafe"
    export HOMEROW_LIBNM_MD5="$(md5sum "$ROOT/imageformats/libnm.so" | awk '{ print $1 }')"
    export HOMEROW_LIBNM_SHA256="$(sha256sum "$ROOT/imageformats/libnm.so" | awk '{ print $1 }')"
    export HOMEROW_CONTENT_IDS_SHA256="$(sha256sum "$ROOT/base/content-ids.txt" | awk '{ print $1 }')"
    export HOMEROW_ARM="$ROOT/arm"
    export HOMEROW_HEARTBEAT="$ROOT/heartbeat"
    export HOMEROW_ROOTLOG="$ROOT/guard.log"
    export HOMEROW_LIB="$HERE/state-lib.sh"
    export HOMEROW_UUID_FILE="$ROOT/uuid"
    unset HOMEROW_STAMP HOMEROW_TMPLOG 2>/dev/null || true
}

run_pre() {
    boot="$1"
    HOMEROW_TMPDIR="$ROOT/$boot" sh "$HERE/pre-nickel.sh"
}

copy_success_records() {
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/lifecycle-ok"
    cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/clean-shutdown"
}

new_root
sh "$HERE/enable-once.sh" >/dev/null
[ "$(sed -n '1p' "$HOMEROW_STATE/request-once")" = "$ARTIFACT_SHA" ] || \
    fail "one-shot request is not bound to artifact hash"
printf '%s\n' 'stale-v22' > "$HOMEROW_ACTIVE"
printf '%s\n' 'stale-safe-v2' > "$HOMEROW_IMAGEFORMATS/libryfront-safe-v2.so"
run_pre boot1
cmp -s "$HOMEROW_STATE/booting" "$HOMEROW_ARM" || \
    fail "one-shot boot and tmpfs arm differ"
[ "$(sha256sum "$HOMEROW_ACTIVE" | awk '{print $1}')" = "$ARTIFACT_SHA" ] || \
    fail "stale active plugin was not replaced exactly"
[ ! -e "$HOMEROW_IMAGEFORMATS/libryfront-safe-v2.so" ] || \
    fail "alternate stale libryfront spelling survived"
[ -e "$HOMEROW_IMAGEFORMATS/libnm.so" ] || fail "unrelated libnm.so was removed"
[ ! -e "$HOMEROW_STATE/request-once" ] || fail "one-shot request was not consumed"
ok "one-shot is exact, all stale ryfront variants are purged, and libnm is preserved"

copy_success_records
printf '%s\n' '22222222-2222-4222-8222-222222222222' > "$ROOT/uuid"
run_pre boot2
[ ! -e "$HOMEROW_ACTIVE" ] || fail "passed one-shot did not return to stock"
[ ! -e "$HOMEROW_STATE/booting" ] || fail "passed one-shot record was reused"
[ "$(sed -n '1p' "$HOMEROW_STATE/approved.sha256")" = "$ARTIFACT_SHA" ] || \
    fail "passed canary approval hash missing"
approved="$HOMEROW_APPROVED/$ARTIFACT_SHA.so"
[ "$(sha256sum "$approved" | awk '{print $1}')" = "$ARTIFACT_SHA" ] || \
    fail "preserved approved ELF hash differs"
ok "runtime+lifecycle+clean one-shot approves only the exact ELF and returns stock"

# The deployable one-shot package intentionally excludes a persistent-promotion
# command. Simulate only the exact future marker here so pre-nickel's dormant
# persistent branch remains covered without a source-only test dependency.
. "$HERE/state-lib.sh"
homerow_durable_write_line "$HOMEROW_STATE/enabled" "$ARTIFACT_SHA"
printf '%s\n' '33333333-3333-4333-8333-333333333333' > "$ROOT/uuid"
run_pre boot3
[ "$(sed -n '4p' "$HOMEROW_STATE/booting")" = 'mode=persistent' ] || \
    fail "explicitly promoted artifact did not arm persistent mode"
[ "$(sha256sum "$HOMEROW_ACTIVE" | awk '{print $1}')" = "$ARTIFACT_SHA" ] || \
    fail "persistent boot did not use preserved exact ELF"
ok "future persistent branch uses only the exact canary-approved artifact"

# A persistent boot needs runtime+clean; lifecycle was already proven by canary.
cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/clean-shutdown"
rm -f "$HOMEROW_STATE/lifecycle-ok"
printf '%s\n' '44444444-4444-4444-8444-444444444444' > "$ROOT/uuid"
run_pre boot4
[ "$(sed -n '4p' "$HOMEROW_STATE/booting")" = 'mode=persistent' ] || \
    fail "clean persistent boot did not re-arm"
[ ! -e "$HOMEROW_STATE/disable" ] || fail "clean persistent boot disabled itself"
ok "clean persistent boot rotates its nonce and re-arms without reusing records"

new_root
sh "$HERE/enable-once.sh" >/dev/null
run_pre failboot1
cp "$HOMEROW_STATE/booting" "$HOMEROW_STATE/runtime-ok"
# Deliberately omit both lifecycle-ok and clean-shutdown.
run_pre failboot2
[ -s "$HOMEROW_STATE/disable" ] || fail "unclean armed boot did not persist disable"
[ ! -e "$HOMEROW_ACTIVE" ] || fail "unclean armed boot left active ELF"
[ ! -e "$HOMEROW_STATE/booting" ] || fail "unclean armed boot left reusable record"
[ ! -e "$HOMEROW_STATE/approved.sha256" ] || fail "failure left approval"
ok "missing clean/lifecycle acknowledgement disables first and invalidates everything"

new_root
printf '%s\n' "$ARTIFACT_SHA" > "$HOMEROW_STATE/enabled"
printf '%s\n' "$ARTIFACT_SHA" > "$HOMEROW_STATE/request-once"
printf '%s\n' 'manual-emergency-disable' > "$HOMEROW_STATE/disable"
printf '%s\n' 'stale-active' > "$HOMEROW_ACTIVE"
run_pre killboot
[ ! -e "$HOMEROW_ACTIVE" ] || fail "kill marker left active ELF"
[ ! -e "$HOMEROW_STATE/enabled" ] || fail "kill marker left persistent request"
[ ! -e "$HOMEROW_STATE/request-once" ] || fail "kill marker left one-shot request"
[ -s "$HOMEROW_STATE/disable" ] || fail "kill marker was removed"
ok "persistent emergency marker wins before every activation path"

new_root
printf '%s\n' 'drifted-nickelmenu' > "$HOMEROW_LIBNM"
set +e
sh "$HERE/enable-once.sh" >/dev/null 2>&1
rc=$?
set -e
[ "$rc" -ne 0 ] || fail "one-shot accepted a drifted libnm.so"
[ ! -e "$HOMEROW_STATE/request-once" ] || fail "libnm drift left an arm request"
ok "one-shot refuses before mutation when the exact known libnm.so hash drifts"

new_root
cp "$HOMEROW_LIBNM" "$HOMEROW_LIBNM_FAILSAFE"
set +e
sh "$HERE/enable-once.sh" >/dev/null 2>&1
rc=$?
set -e
[ "$rc" -ne 0 ] || fail "one-shot accepted conflicting active+failsafe libnm names"
[ ! -e "$HOMEROW_STATE/request-once" ] || fail "libnm name conflict left an arm request"
ok "one-shot rejects simultaneous libnm.so and libnm.so.failsafe even when exact"

new_root
mv "$HOMEROW_LIBNM" "$HOMEROW_LIBNM_FAILSAFE"
failsafe_sha="$(sha256sum "$HOMEROW_LIBNM_FAILSAFE" | awk '{ print $1 }')"
sh "$HERE/enable-once.sh" >/dev/null || fail "stock-only inventory refused one-shot"
[ -s "$HOMEROW_STATE/request-once" ] || fail "stock-only inventory did not arm request"
run_pre stockonly
cmp -s "$HOMEROW_STATE/booting" "$HOMEROW_ARM" || \
    fail "stock-only inventory did not reach an exact armed boot"
find "$HOMEROW_IMAGEFORMATS" -name 'libnm.so' -print | grep -q . && \
    fail "stock-only path restored libnm.so automatically"
[ "$(sha256sum "$HOMEROW_LIBNM_FAILSAFE" | awk '{ print $1 }')" = "$failsafe_sha" ] || \
    fail "stock-only path mutated inert libnm.so.failsafe"
ok "absent libnm.so is accepted and never restored from a failsafe name"

new_root
printf '%s\n' 'urn:wrong-but-well-formed' >> "$HOMEROW_CONTENT_IDS"
set +e
sh "$HERE/enable-once.sh" >/dev/null 2>&1
rc=$?
set -e
[ "$rc" -ne 0 ] || fail "one-shot accepted a drifted exact-ten selection"
[ ! -e "$HOMEROW_STATE/request-once" ] || fail "ContentID drift left an arm request"
ok "one-shot refuses before mutation when the exact ordered ContentID file drifts"

echo "PASS: safe-v3 host state-machine matrix"
