<!-- Generated 2026-07-16 by the firmware-analysis workbench (goal-directed pass 2).
     SPOT-VERIFIED on-binary: ColorUtil::colorToRGB@0x118bd20 = 7-branch switch (cmp r1,6; tbb);
     cssHighlightingClassName@0x118beec = 4-branch (cmp r1,3); Bookmark::getColor@0x95edcc reads
     pimpl+0x84; RGB literals CONFIRMED at exact addrs: Yellow #FFEF5D@0x148de50, Pink #FF95C8@0x148de58,
     Blue #82DDFF@0x148de60, Green #86FF7C@0x148de68, Gray #A0A0A0@0x148de70. Recipes below are grounded
     in these; the picker-builder CALLER is unlocated (indirect Qt wiring) — see §5 gaps. -->

# HIGHLIGHTS-COLORS.md — Kobo `nickel` 4.44.23552 (Libra Colour)

**Goal this doc serves:** add NEW highlight colors beyond the stock 4 (Yellow/Pink/Blue/Green).
**Scope:** all facts below are backed by static disassembly of `libnickel` 4.44.23552. Addresses are file/image virtual addresses (Thumb). Unverified items are flagged explicitly.

---

## 0. TL;DR

- A highlight's color is one `INTEGER` (`Bookmark.Color`) in `KoboReader.sqlite`, typed `HighlightColor` (a bounded 7-member enum, values `0..6`).
- Only `0..3` are "real" user colors (Yellow/Pink/Blue/Green): they alone get a reader CSS class **and** a cloud-sync hex. `4/5/6` (Gray, UnderlineDefaultColor, UnderlineBW) exist in the enum and have RGB but fall to default branches.
- **Reading/exporting colors needs no patch at all** — just read `Bookmark.Color` and map `0..6`.
- **Adding a genuinely new, pickable, on-screen color is hard:** it is a multi-site `libnickel` byte-patch across the color model, *plus* the color-picker builder that would add the 5th swatch **could not be located by exhaustive static analysis** (it is wired indirectly, likely Qt meta-object/`connect`/`std::function` or QML). That blocker makes **native injection the reliable path**, not a blind byte-patch.

---

## 1. The three layers

### Layer A — Storage: `Bookmark.Color`

| Symbol | Addr | Role |
|---|---|---|
| `Bookmark::getColor() const` | `0x0095edcc` | Reads color: `ldr r3,[r0,4]` (pimpl); `r0=*(r3+0x84)`. Color is a raw int at **pimpl+0x84**. No range check. |
| `Bookmark::setColor(HighlightColor)` | `0x00960d84` | COW-detach if `refcount!=1`, then `str r5,[r3+0x84]`. Writes raw int verbatim. **No clamp/validate.** |
| `Bookmark::setColorVariant(QVariant const&)` | `0x00960da4` | DB-load path: `QVariant->int` then stores. How Color is populated when a row is read. No bound check. |
| `Bookmark::getDbValues() const` | `0x009600dc` | Serializes fields (incl. Color as plain int `QVariant`) for INSERT/UPDATE. |
| `Bookmark::save(Device const&)` | `0x0095ee18` | Persists via `getDbValues` → DatabaseAdapter. End of write path. |
| `Migration175_AddBookmarkColor::migrate` | `0x006e65a8` | The DB migration that added the `Bookmark.Color` INTEGER column. |
| `VolumeManager::updateBookmark(...)` | `0x00a84cb8` | Bookmark upsert on a Volume (5-arg; 4-arg sibling `0x00a810e8`). Its `int` arg is **progress/position, not Color** — color travels inside the Bookmark object. |

**Confirmed:** `Bookmark.Color` lives at pimpl+`0x84` as a plain 32-bit int; `setColor` does a COW detach but **no** range validation (no `cmp` against min/max, no masking) — evidence: `getColor` `ldr r3,[r0,4]; ldr r0,[r3,0x84]`; `setColor` refcount check then unconditional `str r5,[r3,0x84]`.

### Layer B — Picker UI (how swatches are built)

| Symbol | Addr | Role |
|---|---|---|
| `SelectionMenuView::addColorSelectionButton(HighlightColor)` | `0x00efcba8` | **Per-swatch builder.** `operator new` of a `0x88`-byte swatch widget, sets objectName/stylesheet, appends to `QVector<ColorLabel*>` at **view+0x30**. Called **once per offered color**. This is the function you'd call an extra time to add a swatch. |
| `SelectionMenuView::showColorPan(bool)` | `0x00efc700` | Toggles color-panel visibility (`strb.w arg,[r0,0x38]`). Not the builder. |
| `SelectionMenuView::setSelectedColor(HighlightColor)` | `0x00efc85c` | Iterates the `QVector<ColorLabel*>` at view+0x30 (do/while begin..end), virtual slot per swatch. Confirms swatch count = dynamic vector size, **not** a fixed array. |
| `SelectionMenuView::colorSelected(HighlightColor)` | `0x00efc61c` | moc signal emitted when a swatch is tapped. |
| `SelectionMenuView::getSelectedColor()` | `0x00efc694` | Reads current selected color. |
| `QVector<ColorLabel*>::~QVector` / `reallocData` | `0x00efd618` / `0x00efd650` | Confirms swatch container type = `QVector<ColorLabel*>`, a `SelectionMenuView` member. |
| `ColorLabel::ColorLabel(HighlightColor,QWidget*)` | `0x00bd27d8` | Swatch label widget; stores HighlightColor at this+`0x84` (`str r6,[r4,0x84]`). `getColor()`@`0x00bd2824`. |
| `SelectionMenuController::onColorSelected(HighlightColor)` | `0x00ef8fd0` | Controller slot wired to the view's `colorSelected` signal; applies chosen color (`r1→r8`, routed onward). |
| `SelectionMenuController::getHighlightColor()` | `0x00efab0c` | Returns controller's current highlight color. |
| `SelectionMenuController::setupMainOptions()` | `0x00efab20` | Builds ONLY the text action menu (Highlight/Note/Copy/Share/Search/Define). Full r2dec (1436 lines) has **zero** color references — the color pan is NOT built here. |
| `HighlightingManager::setBookmarkColor(HighlightColor)` | `0x007a1e58` | Stores user-picked color into process global `*(0xf43cd4)` — the "pending highlight color". No clamp. |
| `HighlightingManager::getBookmarkColor()` | `0x007a1e78` | Reads that pending-color global. |
| `StylusToolMenuController::createBrushColorRows(...)` | `0x00f0be80` | SEPARATE builder for the stylus/pen color rows (Sage/Elipsa). **Not** the text-highlight picker. |

**Confirmed:** swatches are a **dynamic `QVector<ColorLabel*>` (view+0x30)**, filled one at a time by `addColorSelectionButton` — there is **no "count = 4" constant and no static color array** in the view.

**Confirmed (critical blocker):** the **caller** that issues the `addColorSelectionButton` calls **could not be located statically.** All four color-pan functions (`addColorSelectionButton`, `showColorPan`, `ColorLabel::ctor`, `colorToRGB`) have exactly one `GLOB_DAT` GOT slot each and are referenced by **no** direct branch, literal-pool GOT offset, or `movw/movt`. Evidence: all 501252 BL/BLX in `.text` were decoded (op_new@`0x67ee7c` = 14126 hits validates the decoder) → **0** direct calls to `0xefcba8`; GOT_base `0x16E23B0` derived and verified; the relevant offsets appear in no 4-byte literal word and no `movw` immediate. Interpretation: the address is taken for **indirect** wiring (Qt meta-object/`connect`/`std::function` or QML), not a direct call. Also confirmed: r2's own `axt` xref engine is unreliable on this Thumb-PIC binary (returns nothing even for `Volume::d`@`0xa78b18`), so the manual decode was necessary.

### Layer C — Rendering: Color int → on-screen RGB / CSS / sync

| Symbol | Addr | Role |
|---|---|---|
| `ColorUtil::colorToRGB(HighlightColor)` | `0x0118bd20` | **THE int→RGB map.** `cmp r1,6; bhi default`; 7-entry `tbb` @`0x118bd30` (bytes `1d 49 33 5f 04 04 04`). Returns a 7-char `#RRGGBB` QString per case. A **SWITCH, not an appendable table.** |
| `ColorUtil::cssHighlightingClassName(HighlightColor)` | `0x0118beec` | int→CSS class. `cmp r1,3; bhi default`; 4-case `tbb` @`0x118befa` (bytes `1a 12 0a 02`). 0..3 → `KBHighlightingYellow/Pink/Blue/Green`; `>3` → generic `KBHighlighting` (`0x135aed8`), which has no dedicated color rule. |
| `ColorUtil::cssBackgroundColor(QString,HighlightColor)` | `0x0118bf64` | Builds the stylesheet RULE: `"%1 { background-color : %2; }".arg(class).arg(colorToRGB(c))` (fmt `0x148decc`). This is the rule injected into the kepub page CSS. |
| `ColorUtil::cssBackgroundColor(HighlightColor)` | `0x0118c13c` | Inline-fragment variant `background-color: %1;` (fmt `0x148deec`). Non-class contexts (note popups/inline style). |
| `ColorUtil::colorToHighlightSyncHex(HighlightColor)` | `0x0118c26c` | int→hex for Kobo cloud sync. `cmp r1,3`; 4-case `tbb` → `#F6F3B3/#E8AFCF/#B2E1E8/#C6E09E`. Only 0..3 sync-mapped. |
| `ColorUtil::highlightSyncHexToColor(QString)` | `0x0118c2fc` | Inverse (hex→int) for incoming sync. |
| `ColorUtil::createColorButtonPixmap(HighlightColor,int,bool)` | `0x0118c770` | Renders a swatch pixmap (QColor variant @`0x0118c634`). Likely the concrete link picker→`colorToRGB`. |
| `ColorUtil::cssBackgroundColor` / `noteIconImagePath` | `0x0118bf64` / `0x0118c1f4` | Additional per-color switches that each need a new case for a 5th color. |
| `operator<<(QDebug, HighlightColor const&)` | `0x0118c390` | `cmp r3,6; bhi default`; 7-case `tbb` @`0x118c3a8`. Names every enumerator — proves the enum is bounded at 7. |
| `HighlightingManager::spanTemplateForBookmark(Bookmark const&)` | `0x007a1bfc` | Returns 1 of 3 static HTML span templates (globals `0x1727544/48/4c`, runtime-init) by two Bookmark flags. |
| color hex string literals | `0x148de50` | Consecutive 8-byte-spaced rodata: `#FFEF5D`(+0x00)/`#FF95C8`(+0x08)/`#82DDFF`(+0x10)/`#86FF7C`(+0x18)/`#A0A0A0`(+0x20). Each null-terminated 7 chars → **fixed-length byte-patch targets.** |

**Confirmed enum (7 members, values 0..6):**

| idx | Name | colorToRGB | Reader CSS class | Sync hex | First-class? |
|---|---|---|---|---|---|
| 0 | Yellow | `#FFEF5D` | `KBHighlightingYellow` | `#F6F3B3` | ✅ |
| 1 | Pink | `#FF95C8` | `KBHighlightingPink` | `#E8AFCF` | ✅ |
| 2 | Blue | `#82DDFF` | `KBHighlightingBlue` | `#B2E1E8` | ✅ |
| 3 | Green | `#86FF7C` | `KBHighlightingGreen` | `#C6E09E` | ✅ |
| 4 | Gray | `#A0A0A0` (shared) | `KBHighlighting` (generic) | — (default) | ❌ |
| 5 | UnderlineDefaultColor | `#A0A0A0` (shared) | `KBHighlighting` (generic) | — (default) | ❌ |
| 6 | UnderlineBW | `#A0A0A0` (shared) | `KBHighlighting` (generic) | — (default) | ❌ |
| ≥7 | (out of enum) | null/empty QString | `KBHighlighting` | — | n/a |

Enum-name evidence: `QDebug` case blocks → PC-relative rodata: case0 `0x148dfb8` `HighlightColor::Yellow`, case1 `0x148dfd0` `Pink`, case2 `0x148dfe8` `Blue`, case3 `0x148e000` `Green`, case4 `0x148e018` `Gray`, case5 `0x148e030` `UnderlineDefaultColor`, case6 `0x148e058` `UnderlineBW`.
RGB evidence: `colorToRGB` `tbb` @`0x118bd30` = `1d 49 33 5f 04 04 04` → case0=`0x118bd6a`, case1=`0x118bdc2`, case2=`0x118bd96`, case3=`0x118bdee`, cases 4/5/6 all byte `0x04` → shared `0x118bd38` loading `#A0A0A0`@`0x148de70`; literals dumped from rodata (`23 46 46 45 46 35 44 00` = `#FFEF5D`, etc.).

**Confirmed rendering is CLASS-based for kepub:** a span gets a class from `cssHighlightingClassName(color)`, and `cssBackgroundColor(className,color)` builds the `%1 { background-color : %2; }` rule where `%2 = colorToRGB(color)`. Because `cssHighlightingClassName`'s bound is the tighter `cmp r1,3`, any color `≥4` falls to the generic `KBHighlighting` class, which has **no dedicated per-color background rule**.

---

## 2. End-to-end flow (swatch tap → pixels), all typed as int, no re-mapping

`SelectionMenuView::colorSelected(HighlightColor)` `0xefc61c` → `SelectionMenuController::onColorSelected(HighlightColor)` `0xef8fd0` (keeps arg in r8) → `HighlightingManager::setBookmarkColor` `0x7a1e58` (pending-color global `*(0xf43cd4)`) → at save `Bookmark::setColor` `0x960d84` writes pimpl+`0x84` → `getDbValues`/`save` → `KoboReader.sqlite`.
On render: stored int → `cssHighlightingClassName` + `cssBackgroundColor`(→`colorToRGB`) produce the CSS injected into the page.

---

## 3. FEASIBILITY — can we add a 5th+ color?

Classified by the minimal change at each layer and its safety.

### Verdict by intent

| Intent | Minimal change | Layers touched | Safety |
|---|---|---|---|
| **Read / export existing colors (all 0..6)** | none | — | **DB-reversible (no change at all)** |
| **Re-use an existing enum value in DB (set Color=0..6)** | write integer to `Bookmark.Color` | A only | **DB-reversible** |
| **RECOLOR an existing swatch (change what e.g. Green looks like)** | overwrite a 7-byte hex literal in place (`0x148de50`+) | C data only | **patch-reversible** (length-preserving) |
| **ADD a genuinely new, pickable, painted 5th color** | multi-site code patch **+** picker-builder wiring | A (trivial) + B (**blocked statically**) + C (multi-site) | **injection (watchdog) — reliable path** |

### Why "add a new color" is hard, precisely

1. **DB (Layer A): free but insufficient.** Nothing clamps on read or write (`setColorVariant`/`getColor`/`setColor` have no bounds). You can store `Color=7,8,…` and it persists — but every render/format helper hits its **default branch**, so ≥7 shows **no distinct color**. Storing ≥7 is "unknown color," not a new one. So DB alone cannot *invent* a color.

2. **Rendering (Layer C): a SWITCH, not a data table.** `colorToRGB` is an ARM `tbb` jump table (`cmp r1,6`) — you cannot append a case by editing data. `colorToRGB` already *admits* idx 4 (returns gray `#A0A0A0`), so making idx4 a real color can be done data-only by overwriting `#A0A0A0`@`0x148de70`. **But** class-based rendering means you also need `cssHighlightingClassName` to give idx4 its own class — and that requires **raising the `cmp r1,3` bound (code patch)** plus extending its `tbb` and adding a class string. So a pure data-only edit is **not** sufficient to paint a genuinely new color. Additional per-color switches (`cssBackgroundColor`, `noteIconImagePath`, `colorToHighlightSyncHex`) each need a new case for full coverage.

3. **Picker (Layer B): the blocker.** To make the color *selectable*, something must call `addColorSelectionButton(newColor)` one extra time. **That caller is unlocated** (see Layer B confirmed blocker: 0 static references; indirect wiring). A **blind byte-patch of the swatch count is not possible** because there is no count constant and no static call site to patch. Options: (a) locate it dynamically on-device (gdb breakpoint on `0xefcba8` to catch its caller/count) then patch, or (b) **native injection** — hook `SelectionMenuController::loadView`@`0xefbf3c` or post-construction of the view and call `addColorSelectionButton(4)` yourself. (b) is safest given the builder is opaque.

**Overall feasibility: HARD, injection-backed.** For *export* (the liberation read goal) → trivial, DB-only, no change. For a *new displayable swatch* → not DB-only, not a single constant; it is a multi-site `libnickel` change whose UI half is not statically pinned, so native injection is the reliable route.

### Cloud-sync caveat
`colorToHighlightSyncHex` has no mapping for 4..6 (or a new color). A locally-set/new color may be **rejected or normalized** by Kobo cloud on next sync. Irrelevant for offline export or a device kept offline (liberation generally wants sync OFF).

---

## 4. RECIPE

Pick the track that matches your intent.

### Track 0 — EXPORT existing highlights + colors (no firmware change)
1. Copy `KoboReader.sqlite` off the device.
2. Read `SELECT Text, Color, ... FROM Bookmark`.
3. Map `Color` with the enum table in §1C: `0=Yellow #FFEF5D, 1=Pink #FF95C8, 2=Blue #82DDFF, 3=Green #86FF7C, 4=Gray #A0A0A0, 5/6=underline styles`.
   Done — no patch, fully reversible.

### Track 1 — RECOLOR an existing swatch (data-only byte-patch, reversible)
1. In `libnickel` (back it up), locate the target hex literal in rodata: Yellow `0x148de50`, Pink `0x148de58`, Blue `0x148de60`, Green `0x148de68`, Gray `0x148de70`.
2. Overwrite the 7 ASCII bytes `#RRGGBB` in place (keep length = 7, keep the trailing `\0`). Example: change Green by writing new bytes at `0x148de68`.
3. Re-deploy `libnickel`. This **replaces**, does not add, a color. Revert by restoring original bytes.

### Track 2 — Promote Gray (idx 4) into a real painted color (multi-site byte-patch)
> Makes idx4 a genuine color WITHOUT needing a new enum value. `colorToRGB` already admits idx4.
1. Backup `libnickel`.
2. **RGB:** overwrite `#A0A0A0`@`0x148de70` with your `#RRGGBB` (7 bytes) — now `colorToRGB(4)` returns your color. (Note: idx 5 and 6 share this literal, so they change too.)
3. **Reader CSS class:** patch `cssHighlightingClassName`@`0x0118beec`: raise the bound byte at `0x118bef2` `cmp r1,3` → `cmp r1,4`, extend the `tbb` @`0x118befa` to 5 entries, add a new `KBHighlighting<Name>` rodata class string, and point the new case at it. (Code patch — non-trivial; must add a rodata string.)
4. **Note/icon (optional):** add a case in `cssBackgroundColor`@`0x0118bf64` context and `noteIconImagePath`@`0x0118c1f4` so notes/icons for idx4 don't fall to default.
5. **Sync (optional, only if online):** `colorToHighlightSyncHex`@`0x0118c26c` raise `cmp r1,3`→`cmp r1,4` + 5th `tbb` case. Skip for local-only.
6. **Make it selectable:** you still need the picker to offer idx4 — go to Track 3 for the UI. (Until then, idx4 is only reachable by writing `Bookmark.Color=4` directly in the DB.)
7. Verify on-device: write `Color=4` to a Bookmark, open the book, confirm it paints your color (this consequence is **UNVERIFIED statically** — see gaps).

### Track 3 — ADD a selectable 5th swatch (native injection — reliable path)
> Recommended because the picker-builder caller is not statically locatable.
1. Do the Layer-C model work from Track 2 (steps 2–5) so a new color index actually renders. If reusing idx4 (Gray slot), the enum already permits it; if you want a truly 8th enum value you must also bump `colorToRGB`'s `cmp r1,6` and its `tbb` and add an RGB literal.
2. Inject native code (established imageformats-plugin vector from the Kobo-liberation toolkit) to:
   a. Hook `SelectionMenuController::loadView`@`0x00efbf3c` (or post-construction of `SelectionMenuView`), obtain the view pointer, and call `SelectionMenuView::addColorSelectionButton(view, <newColor>)`@`0x00efcba8` one extra time. This adds the swatch to the `QVector<ColorLabel*>` at view+0x30 without needing to find the original builder.
   b. Inject a matching CSS rule (`.KBHighlighting<Name>{background-color:#RRGGBB}`) into the reader webview stylesheet so the painted color is guaranteed regardless of the class-sheet builder.
3. Confirm the tap path (`colorSelected`→`onColorSelected`→`setBookmarkColor`→`setColor`) carries the new int unchanged (it does — no re-mapping) so the pick persists to `Bookmark.Color`.
4. Test end-to-end on device; keep sync OFF unless you completed Track 2 step 5.

**Do NOT** attempt a blind byte-patch of the swatch count — there is no count constant and no static call site. If you insist on a byte-patch-only UI, first locate the builder dynamically (gdb breakpoint on `0x00efcba8`, read the caller/count on-device), then patch.

---

## 5. Gaps / open questions (not statically resolvable — flag before acting)

1. **Picker-builder caller is UNLOCATED.** Exhaustive BL/BLX decode + GOT analysis found 0 static references to `addColorSelectionButton`/`showColorPan`/`ColorLabel::ctor`/`colorToRGB`. Wiring is indirect (Qt meta/connect/std::function/QML). Needs on-device dynamic tracing (breakpoint `0x00efcba8`) to know whether swatches come from an explicit call sequence or a loop, and to find any count site. **This is the main risk for a byte-patch approach.**
2. **Does `Color=4/5/6` actually paint on-page?** UNVERIFIED. Rendering is class-based and `cssHighlightingClassName(4)='KBHighlighting'` (generic, likely unstyled by the per-color sheet), so idx4 might render gray, as no background, or note-icon-only. Requires a device test (write `Color=4`, open book). No device available in this analysis.
3. **The stylesheet BUILDER** (the loop calling `cssBackgroundColor(cssHighlightingClassName(c),c)`) is UNPINNED — callers are PIC-indirect GOT loads; PLT stubs `0x670a3c/0x6a6c40/0x6b0c24` have 0 BL callers. Blocks confirming whether the sheet iterates only 0..3 (leaving `KBHighlighting` unstyled) or includes the generic class. Needs full r2 emulation or on-device tracing.
4. **Exact per-index `tbb` order** in `cssHighlightingClassName`/`colorToHighlightSyncHex` was not cross-decoded per index (the color SET is confirmed; the precise idx→class/hex pairing was inferred, not `tbb`-verified for those two).
5. **Are idx 4/5/6 ever present in real user rows,** or only transient for underline rendering? Needs a device DB sample.
6. **Cloud-sync behavior** for a locally-set/new color: `colorToHighlightSyncHex`/`highlightSyncHexToColor` located but their unknown-hex/≥4 handling not decompiled. Relevant only if the account syncs highlights.
7. **`createColorButtonPixmap`**@`0x0118c770` and its callers not traced — likely the concrete swatch-pixmap renderer linking builder→`colorToRGB`; could be a second on-device hook point.
8. **Precise runtime RGB hex** of any color is authoritative from rodata for the 5 dumped literals; but note `colorToRGB` also builds some strings from runtime-relocated `QStringData` (addresses `0x2317xxx` outside the file image) — if exact export colors matter, read from the live process.
