# Kobo Libra Colour (FW 4.44.23552) — Native Home Pager, Russian UI & Live Stats

Custom native layer for the **Kobo Libra Colour** (`mt8113t-ntx`, firmware **4.44.23552**): a freestanding headerless C++ plugin for stock Nickel, a full Russian interface pack, live on-device reading statistics and highlights inside native *Activity*, a 5th highlight color swatch, `kobopatch` recipes ported to `4.44.23552`, and a crash-durable watchdog gate that prevents bootloops.

> Made by an owner, for an owner's own device. Kobo® and Rakuten Kobo® are trademarks of their respective holders; this project is neither affiliated with nor endorsed by them.

## What it does on top of stock Nickel

1. **Two-page swipeable cover row on stock Home (`HomePageView`)** — reuses Nickel's own `FourBookHomeWidget` and `SwipeGestureRecognizer` to page between two 4-book strips (8 local books, no Kobo Store account or cloud sync needed; tapping a cover opens the book through Nickel's stock `SingleBookHomeWidget::tapped()` path).
2. **Full Russian interface for firmware 4.44.23552** — combines a compiled Qt translation table (`langpack/trans_ru.qm`) with live in-memory Russification of widgets that Kobo leaves untranslated or formats dynamically (`ElidedLabel` home tiles, pluralized remaining-time estimates in hours/minutes/seconds, inline dictionary language selector, and *Reading Life* headers).
3. **Live library statistics & highlights in native *Activity* (`StatsView`)** — `koboviz/koboviz-gen.sh` queries `KoboReader.sqlite` into lightweight rootfs sidecars (`/usr/local/koboviz/stats.txt` and `highlights.txt`), and the plugin injects native `FontSizeAdjustingLabel` / word-wrapped `QLabel` blocks straight into Nickel's *Activity* and *My Words* views.
4. **Fifth highlight color swatch** — appends the hidden 5th `HighlightColor` index (`purple`) to `SelectionMenuView` whenever the in-book highlight popup builds its 4-swatch row.
5. **Kaleido 3 `CFA Mode = G2` LUT mapping** — exact `[DeveloperSettings]` `ForceCfaMode` constants (`163` / `0xa3` for `G2`) disassembled from `N3SettingsDeveloperOptionsView` for richer cover and highlight saturation.
6. **Fail-closed watchdog & canary gate** — external `/etc/init.d/rcS` / `rcK` supervisor (`pre-nickel.sh` + `watchdog.sh` + `shutdown-check.sh`) with a 2-second GUI-thread heartbeat, one-shot canary verification (`runtime-ok` + `lifecycle-ok` + `clean-shutdown`), and bounded strike counters so a bad binary falls back to stock Nickel automatically instead of bootlooping.

## What's here

```
homerow-v3/                 native plugin + fail-closed boot/shutdown supervisor
  home.cpp                    freestanding C++ plugin (zero Qt/Nickel headers; links only libc.so.6)
  legacy-loader-layout.ld     linker layout compatible with Kobo's ARM dynamic loader
  build.sh                    cross-compiles home.cpp -> libryfront-safe-v3.so via `zig c++`
  pre-nickel.sh               synchronous boot gate run before Nickel
  watchdog.sh                 external Nickel PID + tmpfs GUI heartbeat supervisor
  shutdown-check.sh           clean-shutdown verifier hooked into /etc/init.d/rcK
  state-lib.sh                fsync + atomic rename state primitives
  enable-once.sh              arm a single supervised canary boot
  enable-persistent.sh        promote an exact canary-verified SHA-256 to persistent mode
  emergency-disable.sh        immediate kill switch back to stock Nickel
  rollback-to-stock.sh        full removal of hooks and plugin artifacts
  set-content-ids.sh          install a pinned 10-book ContentID list
  content-ids.example.txt     example ContentID manifest
  rcS-hook.patch              minimal diff for /etc/init.d/rcS
  rcK-hook.patch              minimal diff for /etc/init.d/rcK
  test-*.sh                   host-side state-machine and watchdog test suites
langpack/
  trans_ru.qm                 compiled Qt .qm hash-table translation for FW 4.44.23552
  build_qm_audit.py           Capstone script to extract QCoreApplication::translate contexts
                              from your own local copy of libnickel.so.1.0.0
koboviz/
  koboviz-gen.sh              on-device SQLite -> stats.txt / highlights.txt generator
patches/
  kobopatch.yaml              kobopatch configuration for firmware 4.44.23552
  libnickel.so.1.0.0.yaml     ported & custom YAML patches (telemetry off, offline browser, UI cleanup)
tools/
  kobo-tap.c                  inject portrait touchscreen taps (/dev/input/event1) over SSH
  kobo-shot.sh                capture 1264x1680 BGRA framebuffer screenshots over SSH
docs/
  ABI-FACTS.md                verified 32-bit ARM Qt5.2 / Nickel ABI notes for 4.44.23552
  CFA-DISPLAY-MODES.md        ForceCfaMode hardware LUT table (Default, S4..S9, G0..G2)
  DB-LOAD-RECIPE.md           safe SQLite inspection & sideload notes
  HIGHLIGHTS-COLORS.md        5-color highlight internals on Kobo Colour
  LIBRARY-DISPLAY.md          HomePageView & FourBookHomeWidget reverse-engineering notes
  VOLUME-CONTENT-MODEL.md     VolumeManager & Content pimpl layout notes
```

## Building & Running the Host Tests

Requires `zig` (tested with Zig 0.13–0.16) and `python3` on macOS or Linux. No Kobo SDK or Qt headers are needed to compile the plugin:

```sh
cd homerow-v3
./build.sh

# Run the state-machine and supervisor tests on the host:
./test-state-machine.sh
./test-watchdog.sh
./test-rcs-watchdog-fallback.sh
```

To build a patched `KoboRoot.tgz` from `patches/`, place your own official `kobo-update-4.44.23552.zip` in `patches/src/` and run [`kobopatch`](https://github.com/pgaskin/kobopatch) against `patches/kobopatch.yaml`.

## What's deliberately *not* here

- **No Rakuten Kobo firmware binaries or rootfs files** (`libnickel.so.1.0.0`, `KoboRoot.tgz`, `kobo-update-4.44.23552.zip`, or full `/etc/init.d/rcS` scripts). `home.cpp` resolves Qt and Nickel symbols dynamically at runtime and links only `libc.so.6`; rootfs integration is distributed strictly as small `.patch` diffs and YAML recipes.
- **No books, dictionaries, SSH keys, or personal `KoboReader.sqlite` snapshots.**

## Safety

All ABI offsets, mangled symbols, and vtable indices in `home.cpp` and `patches/` target **Kobo Libra Colour firmware `4.44.23552` only**. Do not load this binary on a different firmware build without re-verifying every symbol offset (block automatic OTA updates via `patches/kobopatch.yaml` before deploying). Modifying `/etc/init.d/rcS` on an e-reader carries real risk; keep a copy of the official `4.44.23552` `KoboRoot.tgz` handy for USB recovery.

## Credits & License

- Base `kobopatch` tooling and upstream patch patterns are the work of Patrick Gaskin ([`pgaskin/kobopatch`](https://github.com/pgaskin/kobopatch), [`pgaskin/kobopatch-patches`](https://github.com/pgaskin/kobopatch-patches), MIT License) and the MobileRead community.
- All original code, scripts, patches, and documentation in this repository are released under the [MIT License](LICENSE).
