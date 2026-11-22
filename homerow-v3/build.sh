#!/bin/sh
# Build the freestanding headerless Qt5/Nickel plugin (libryfront-safe-v3.so)
# for Kobo Libra Colour firmware 4.44.23552 using Zig's C++ cross-compiler.
#
# Usage:
#   ./build.sh
#   KOBO_ROOTFS=/path/to/extracted-4.44.23552-rootfs ./build.sh   # optional symbol audit
set -eu
export LC_ALL=C

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"
OUT="${OUT:-libryfront-safe-v3.so}"

zig c++ -target arm-linux-gnueabihf.2.19 -mcpu=cortex_a9 \
    -shared -fPIC -O2 -fno-exceptions -fno-rtti -nostdlib++ \
    -Wall -Wextra -Wpedantic -Werror \
    -Wl,--allow-shlib-undefined -Wl,-z,lazy -Wl,-z,nodelete \
    -Wl,-T,"$HERE/legacy-loader-layout.ld" \
    -fdebug-prefix-map="$HERE"=/src/homerow-v3 \
    -o "$OUT" home.cpp

python3 - "$OUT" <<'PY'
import hashlib, pathlib, sys
p = pathlib.Path(sys.argv[1])
data = p.read_bytes()
assert data[:4] == b"\x7fELF" and data[4] == 1 and data[18] == 40, "expected 32-bit ARM ELF"
sha = hashlib.sha256(data).hexdigest()
pathlib.Path("artifact.sha256").write_text(sha + "\n")
print(f"[OK] Built {p.name} ({len(data)} bytes, sha256={sha})")
PY
