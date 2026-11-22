#!/bin/sh
# Small crash-durability primitives shared by safe-v3 rootfs controls.
# All persistent state is written to an adjacent temporary, globally synced,
# renamed, and synced again so neither file contents nor the directory rename are
# treated as durable before the next decision.

homerow_valid_sha256() {
    local value="$1"
    [ "${#value}" -eq 64 ] || return 1
    case "$value" in *[!0-9a-f]*) return 1 ;; esac
    return 0
}

homerow_valid_md5() {
    local value="$1"
    [ "${#value}" -eq 32 ] || return 1
    case "$value" in *[!0-9a-f]*) return 1 ;; esac
    return 0
}

homerow_sha256() {
    local result
    [ -f "$1" ] || return 1
    result="$(LC_ALL=C sha256sum "$1" 2>/dev/null)" || return 1
    result="${result%% *}"
    homerow_valid_sha256 "$result" || return 1
    printf '%s\n' "$result"
}

homerow_md5() {
    local result
    [ -f "$1" ] || return 1
    result="$(LC_ALL=C md5sum "$1" 2>/dev/null)" || return 1
    result="${result%% *}"
    homerow_valid_md5 "$result" || return 1
    printf '%s\n' "$result"
}

# Accept the three safe NickelHook inventory states: no file, one exact active
# libnm.so, or one exact inert libnm.so.failsafe. A same-directory rename between
# the latter two is atomic, so polling never needs to treat that transition as
# drift. Two simultaneous names, a broken symlink, or any hash drift is invalid.
homerow_optional_libnm_valid() {
    local active="$1"
    local failsafe="$2"
    local expected_md5="$3"
    local expected_sha="$4"
    local count=0
    local candidate=""
    if [ -e "$active" ] || [ -L "$active" ]; then
        count=$((count + 1))
        candidate="$active"
    fi
    if [ -e "$failsafe" ] || [ -L "$failsafe" ]; then
        count=$((count + 1))
        candidate="$failsafe"
    fi
    [ "$count" -le 1 ] || return 1
    [ "$count" -eq 1 ] || return 0
    [ "$(homerow_md5 "$candidate" 2>/dev/null)" = "$expected_md5" ] || return 1
    [ "$(homerow_sha256 "$candidate" 2>/dev/null)" = "$expected_sha" ] || return 1
    return 0
}

homerow_sync() {
    sync
}

homerow_durable_write_line() {
    local target="$1"
    local value="$2"
    local tmp="$target.new.$$"
    rm -f "$tmp" 2>/dev/null || true
    umask 077
    printf '%s\n' "$value" > "$tmp" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    homerow_sync || return 1
    mv -f "$tmp" "$target" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    homerow_sync || return 1
    return 0
}

homerow_durable_copy() {
    local source_file="$1"
    local target="$2"
    local mode="${3:-0600}"
    local tmp="$target.new.$$"
    [ -f "$source_file" ] || return 1
    rm -f "$tmp" 2>/dev/null || true
    cp "$source_file" "$tmp" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    chmod "$mode" "$tmp" 2>/dev/null || true
    homerow_sync || return 1
    mv -f "$tmp" "$target" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    homerow_sync || return 1
    return 0
}

homerow_durable_remove() {
    rm -f "$@" 2>/dev/null || true
    homerow_sync || return 1
    return 0
}

homerow_write_boot_record() {
    local target="$1"
    local sha="$2"
    local token="$3"
    local mode="$4"
    local tmp
    homerow_valid_sha256 "$sha" || return 1
    case "$token" in
        ''|*[!0-9a-f-]*) return 1 ;;
    esac
    [ "${#token}" -ge 8 ] && [ "${#token}" -le 64 ] || return 1
    case "$mode" in once|persistent) ;; *) return 1 ;; esac
    tmp="$target.new.$$"
    rm -f "$tmp" 2>/dev/null || true
    umask 077
    {
        printf '%s\n' 'protocol=homerow-safe-v3'
        printf 'sha256=%s\n' "$sha"
        printf 'token=%s\n' "$token"
        printf 'mode=%s\n' "$mode"
    } > "$tmp" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    homerow_sync || return 1
    mv -f "$tmp" "$target" || {
        rm -f "$tmp" 2>/dev/null || true
        return 1
    }
    homerow_sync || return 1
    return 0
}

homerow_record_sha() {
    local sha
    [ -f "$1" ] || return 1
    sha="$(sed -n '2s/^sha256=//p' "$1" 2>/dev/null)" || return 1
    homerow_valid_sha256 "$sha" || return 1
    printf '%s\n' "$sha"
}

homerow_read_sha_file() {
    local value
    [ -f "$1" ] || return 1
    value="$(sed -n '1p' "$1" 2>/dev/null)" || return 1
    homerow_valid_sha256 "$value" || return 1
    [ "$(wc -l < "$1" 2>/dev/null)" -eq 1 ] || return 1
    printf '%s\n' "$value"
}

homerow_record_mode() {
    local mode
    [ -f "$1" ] || return 1
    mode="$(sed -n '4s/^mode=//p' "$1" 2>/dev/null)" || return 1
    case "$mode" in once|persistent) printf '%s\n' "$mode" ;; *) return 1 ;; esac
}

homerow_record_valid() {
    local token
    [ -f "$1" ] || return 1
    [ "$(sed -n '1p' "$1" 2>/dev/null)" = 'protocol=homerow-safe-v3' ] || return 1
    homerow_record_sha "$1" >/dev/null || return 1
    token="$(sed -n '3s/^token=//p' "$1" 2>/dev/null)" || return 1
    case "$token" in ''|*[!0-9a-f-]*) return 1 ;; esac
    [ "${#token}" -ge 8 ] && [ "${#token}" -le 64 ] || return 1
    homerow_record_mode "$1" >/dev/null || return 1
    [ "$(wc -l < "$1" 2>/dev/null)" -eq 4 ] || return 1
    return 0
}
