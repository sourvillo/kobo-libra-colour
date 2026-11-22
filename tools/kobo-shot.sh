#!/usr/bin/env bash
# Capture a live portrait screenshot (1264x1680 BGRA -> PNG) from Kobo Libra Colour over SSH.
# Usage: KOBO_IP=192.168.x.x [KOBO_SSH_KEY=~/.ssh/id_ed25519] ./tools/kobo-shot.sh [out.png]
set -euo pipefail

KOBO_IP="${KOBO_IP:?Set KOBO_IP to your reader's LAN IP}"
OUT="${1:-/tmp/kobo-shot.png}"
SSH_OPTS=(-o BatchMode=yes -o ConnectTimeout=5)
if [ -n "${KOBO_SSH_KEY:-}" ]; then
    SSH_OPTS+=(-i "$KOBO_SSH_KEY" -o IdentitiesOnly=yes)
fi

ssh "${SSH_OPTS[@]}" root@"$KOBO_IP" 'dd if=/dev/fb0 bs=5056 count=1680 2>/dev/null' > "$OUT.raw"
python3 - "$OUT.raw" "$OUT" <<'PY'
from PIL import Image
import sys
data = open(sys.argv[1], "rb").read()
Image.frombuffer("RGBA", (1264, 1680), data, "raw", "BGRA", 0, 1).convert("RGB").save(sys.argv[2])
PY
rm -f "$OUT.raw"
echo "$OUT"
