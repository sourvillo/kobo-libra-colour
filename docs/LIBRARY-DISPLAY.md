<!-- Generated 2026-07-16 by the firmware-analysis workbench (goal-directed pass 2).
     SPOT-VERIFIED on-binary: GridLibraryController<Volume>::columnCount@0xb03114 = ldr r0,[r0,0x40]
     (bytes 00 6c) — the 2-byte density patch target; config keys CONFIRMED present in .rodata:
     LastLibraryViewType@0x13a081c, LastLibrarySorter@0x13a0dd8 (resolves the "key name unverified"
     flag; section + value strings still need a one-time device peek). -->

# LIBRARY-DISPLAY.md — How the Kobo library screen displays books (nickel 4.44.23552)

**Goal of this doc:** change how books are *arranged/sized/moded* in the library (grid vs list vs detail, columns per row, cover size). All facts below are backed by disassembly of `libnickel.so.1.0.0` (4.44.23552). Speculative points are flagged **[SPECULATIVE]**.

---

## 1. The triad: Builder → Controller → View → DataSource

The library screen is **NOT** a `QAbstractItemModel`. It is a hand-rolled pipeline. A per-screen **Builder** holds the config (ViewType, sorter, filter) and its `buildController()` factory instantiates a **Controller** + **View** + **DataSource** triad.

| Leg | Symbol | Addr | Role |
|-----|--------|------|------|
| Factory | `AbstractBuilder<Volume>::buildController()` | `0xb2f448` | Virtual-dispatches builder vtable slot **+0x34** (build source/provider) then **+0x3c** (createController); stores source into controller (+0x30/+0x34 refcount pair); returns a `QSharedPointer` (0x10-byte refcounted block `{ref, controller, builder}`). |
| Main-lib dispatch | `LibraryBuilder::newController(ViewType, QSharedPointer<LibraryDataSource<Volume>>)` | `0xe4dd48` | **The single point where the ViewType enum turns into a concrete controller.** Branches on ViewType: `==0`, `==6`, `==7`, else default → three distinct controller-construction paths. |
| Builder entry | `AbstractLibraryBuilder::buildController(int)` / `(QString const&)` | `0xb28c64` / `0xb28c30` | Overloads that drive the virtual dispatch (int overload ≈ ViewType/page index). |
| Controller base | `AbstractLibraryController<Volume>::setSource(QSharedPointer<LibraryDataSource<Volume>>, bool)` | `0xb0815c` | Binds a source: source ptr → `controller+0x14`, QSharedPointer refcount → `+0x18` (ldrex/strex incref); clears `+0x38` at entry. If the trailing `bool` ≠ 0, virtual-dispatches through the view (`this+0xc`) = "reload now". |
| Grid controller ctor | `NickelGridLibraryControllerBase::NickelGridLibraryControllerBase(int rows, int cols, QSharedPointer<...>)` | `0xe66e18` | rows → `+0x3c`, cols → `+0x40`; source → `+0x14`/`+0x18` (same slots as setSource). **Grid geometry is a ctor param, not a runtime-mutable field here.** |
| View construction (template) | `GridAndListLibraryController<GridLibraryView>::loadView()` | `0xe4f314` | `operator new(0x84)` for the `GridLibraryView` widget, installs it. |
| " | `GridAndListLibraryController<DragonListLibraryView>::loadView()` | `0xe4f120` | List instantiation. |
| " | `GridAndListLibraryController<QuickAccessLibraryView>::loadView()` | `0xf6b268` | Quick-access instantiation. |
| View base | `AbstractLibraryController<Volume>::loadView()` | `0xb04034` | Section/blueprint-keyed base view builder; concrete overrides build grid vs list. |
| Grid view ctor | `GridLibraryView::GridLibraryView(int rows, int cols)` | `0xe4c790` | rows → `[this+0x78]`, cols → `[this+0x7c]`; nested loops pre-allocate `rows*cols` cell widgets (each **0xD0 bytes**, alloc via `0x67ee7c`). |
| Volume view ctor | `GenericLibraryView<Volume>::GenericLibraryView()` | `0xcf034c` | Ctor of the `*LibraryView<Volume>` QWidget subtree. **Cleanest native-injection anchor** (mirrors the HomePageView pattern). |
| Provider leg | `LinearLibraryDataSource<Volume>::ctor` / `contentForRowInSection` | `0xb174e4` / `0xb17250` | Wraps a `LibraryDataProvider<Volume>` (e.g. `AllVolumesLibraryDataProvider@0xb33588`) as `QSharedPointer<LibraryDataSource<Volume>>`; maps (row,section)→Volume. This is what's handed to `setSource`/the ctor. |
| Per-book cell | `BookCoverView` (ctor) | `0xc7df58` | The grid cell/delegate. `setMaximumCoverSize(QSize)@0xc820b8`, `updateSize()@0xc82064`, `setContentProgress@0xc7deb8`, `setImage@0xc820f4`. **Cover pixel size is assigned by the grid (canvasWidth/columns), not fixed in the widget.** |

**Two library grid code paths exist** — confirm which one the live Books screen uses before patching:
- **`GridLibraryController<Volume>`** — columns are a **runtime field** at `+0x40`.
- **`Grid2LibraryController<Volume>`** — `columnCount()` returns a **constant 1**. Also `Grid2LibraryController<Bookmark/Shortcover>@0xaf93b4/0xbb0fd0` exist.

---

## 2. What controls the display MODE and layout

### 2a. Display MODE = one enum `AbstractLibraryBuilder::ViewType` at `builder+0x1c`

Every `viewBy*()` setter just writes an int to `builder+0x1c`:

| Setter | Addr | Writes | Meaning |
|--------|------|--------|---------|
| `viewByMxNCoverGrid()` | `0xb27afc` | `str #1,[builder,0x1c]` | ViewType 1 = M×N cover grid |
| `viewByMxNDetailGrid()` | `0xb27b78` | `str #2` | detail grid (cover + metadata rows) |
| `viewByTextTable()` | `0xb27bf4` | `str #3` | text table / plain list |
| `viewBy5List()` / `viewByList()` | `0xb2811c` / `0xb28130` | `str #4` | list (viewByList first checks view supports it via vtable slot +0x80) |
| `viewBySimilarCarousel()` | `0xb279f0` | — | carousel |
| `viewBy5Shelf()` | `0xb27c70` | — | shelf |
| `viewBy2x3ShelfGrid()` | `0xb27d14` | — | 2×3 shelf grid |
| `viewBy3x3ArticleGrid()` | `0xb27db8` | — | 3×3 article grid |
| `viewBy3x4NotebookGrid()` | `0xb27e34` | — | 3×4 notebook grid |
| `viewBy2x3Grid()` | `0xb280f0` | — | 2×3 grid |

`serializeViewType(ViewType)@0xb27a00` is an 11-case `tbb` switch (`cmp r1,0xa`), each case yielding a distinct QString (lengths 0xd..0x17) → **11 enum members (0..10)** confirmed.

**⚠️ Two deep-dives disagree on the enum→value mapping.** Pass A read the setter store-immediates directly (CoverGrid=1, DetailGrid=2, TextTable=3, List=4). Pass B read the `serializeViewType` switch order (case0=DetailGrid, case1=TextTable, case2=5List, ...). **The store-immediate values in the setters (Pass A) are the authoritative runtime enum values**; the serialize switch order is a separate string-table order. **Do not hardcode a numeric value without re-confirming against the specific setter's `str #N`.**

### 2b. Mode is PERSISTED per-provider as a settings string

| Symbol | Addr | Detail |
|--------|------|--------|
| `ApplicationSettings::getLibraryViewType(QString provider)` | `0xa0e4d8` | Reads key literal `'LastLibraryViewType'` (rodata `0x13a081c`, len 0x13) via `QString::fromUtf8`; default = empty shared-null. |
| `ApplicationSettings::setLibraryViewType(QString, QString)` | `0xa0e61c` | Writes same key. This is what the on-device Cover/List menu writes. |
| `AbstractBuilder<Volume>::useLastViewTypeForProvider(QString provider)` | `0xb2f168` | Read side: fetches `getLibraryViewType(provider)`, string-compares, calls the matching `viewBy*` setter (`blx r2`) → populates `builder+0x1c`. |
| `LibraryBuilder::useDefaultViewType()` | `0xe4da90` | Fallback mode when nothing saved (calls a `viewBy*` via virtual dispatch — **value unresolved**). |

**⚠️ Section-name conflict between passes:** one pass reports the section as `[ApplicationSettings]`, another traced `ApplicationSettings::sectionName@0xa0baa8` → `'ApplicationPreferences'` (rodata `0x13a0da8`). **`ApplicationPreferences` is the disassembly-backed value; verify against the on-device `Kobo eReader.conf` before writing.**

### 2c. Sort and sub-filter are ALSO settings-strings (independent of view)

| Symbol | Addr | Key |
|--------|------|-----|
| `AbstractLibraryBuilder::getLastLibrarySorter()` | `0xb26fe4` | reads `'LastLibrarySorter'` (`0x13a0dd8`), builds `Sorter<Volume>` |
| `AbstractLibraryBuilder::setSortBy(QString)` | `0xb26c14` | compares vs interned `sortBy*` literals (`0x13b23c5+`): sortByTitle, sortByAuthor, sortBySeries, sortByNewest, sortByDateAdded, sortByDateAddedOldest, sortByRecent, sortByFileType, sortBySize, sortByBookNumber, sortByLocation |
| `ApplicationSettings::getSortOrder/setSortOrder` | `0xa0ca6c` / `0xa0cc38` | generic per-provider sort persistence |
| `ApplicationSettings::getLibrarySubFilter/setLibrarySubFilter` | `0xa0cdb0` / `0xa0cf1c` | `'LastLibrarySubFilter'` (`0x13a0dc0`) |
| `LibraryDataProvider<Volume>::applySorter(Sorter<Volume>*)` | `0xb0f18c` | applies chosen Sorter in-place (qSortHelper); concretes = `KeySorter<Volume,{Title,Subtitle,Series,SeriesNumber,Recent,Recent2}Key>` @ `0x839280` region |

### 2d. Grid density = `columnCount()` (the key lever for cover size)

| Symbol | Addr | Body | Meaning |
|--------|------|------|---------|
| `GridLibraryController<Volume>::columnCount()` | `0xb03110` | `ldr r0,[r0,0x40]` (bytes `00 6c`) | Returns member `*(this+0x40)` = columns per row. **Grid density lever.** |
| `AbstractLibraryController<Volume>::columnCount()` | `0xb030c4` | `movs r0,1` (bytes `01 20`) @`0xb030c6` | List/linear base returns literal **1**. |
| `Grid2LibraryController<Volume>::columnCount()` | `0xe8d2b4` | `movs r0,1` (bytes `01 20`) | Alt grid path returns constant 1. |
| `GridLibraryController<Volume>::heightForContentCanvas()` | `0xb05fc8` | calls columnCount via vtable slot **+0x20** @`0xb05fe2` | rows = `ceil(items/columns)` → columnCount drives BOTH columns AND row count/cover height. |
| `GridLibraryController<Volume>::heightForRowInSection(int,int,Volume const&)` | `0xb04278` | (garbled in decomp) | per-row height = cover height; reads geometry off `this+0x3c`. **[SPECULATIVE constant — needs raw pass]** |
| `N3DeviceCharm::gridViewRowCount()` | `0x1112e48` | returns 3 or 2 (device branch) | Device-specific hardcoded ROW count. Siblings: `listViewRowCount@0x1112dac`, `smallListViewRowCount@0x1112e60`, `articlesGridRowCount@0x1112d7c`, `standardListViewRowCount@0x1112d3c`, `myWordsItemsPerPage@0x1112e18`. **No `gridViewColumnCount` exists** — columns come from `columnCount()`. |

**Key relationship:** `cover cell width = canvasWidth / columnCount`. **Fewer columns → bigger covers; more columns → smaller/denser.** All grid layout math routes through `columnCount()` via vtable slot +0x20.

`AbstractLibraryController<Volume>::layoutType()@0xb030b4` returns 0; two arrangement algorithms exist (`layoutLeftRightTopDown` / `layoutTopDownLeftRight`).

---

## 3. FEASIBILITY — ranked by effort/safety

### Surface (1) — CONFIG/SETTINGS ONLY  [SAFEST, no binary touch, reversible]

Switch the DEFAULT among native modes by editing the persisted settings string(s):
- `LastLibraryViewType` → picks the mode (list / grid / cover-grid / detail-grid / text-table / shelf).
- `LastLibrarySorter` → picks sort (sortByTitle / sortByAuthor / sortBySeries / sortByNewest / sortByDateAdded / ...).
- `LastLibrarySubFilter` → sub-filter.

This is exactly what the on-device Cover/List menu writes. **It is a `.conf` (INI) edit — NOT a `KoboReader.sqlite` change.** (The DB only governs *which* books/metadata show, e.g. `content.ContentType`/`ReadStatus`.) Covers "list vs grid vs detail vs text-table + sort". **Cannot invent a new arrangement, and cannot set an arbitrary column count.** Nickel rewrites the key when the user toggles view in the UI, so a conf default is "sticky until changed", not locked.

**Blockers to verify before writing (see §4):** exact section name (`ApplicationPreferences` vs `ApplicationSettings`), exact key form (plain `LastLibraryViewType` vs per-provider composite), and the exact serialized string values.

### Surface (2) — LIBNICKEL BYTE-PATCH  [MEDIUM, single-site, like home `maxSlots`]

- **Grid density / cover size (RECOMMENDED):** patch `GridLibraryController<Volume>::columnCount@0xb03110`. At vaddr **`0xb03114`** replace `ldr r0,[r0,0x40]` (bytes `00 6c`) with `movs r0,#N`: N=4 → `04 20`, N=2 → `02 20`, N=3 → `03 20`. Because all grid layout (columns + `ceil(items/columns)` height → cover size) routes through this one getter via vtable slot +0x20, this 2-byte patch fully controls density.
- **Force a specific mode regardless of saved pref:** patch `LibraryBuilder::useDefaultViewType@0xe4da90` or `useLastViewTypeForProvider@0xb2f168` to always land on one `viewBy*` value.
- **Grid ROW count:** patch `N3DeviceCharm::gridViewRowCount@0x1112e48` (returns 3/2).
- **Alt grid path density:** `Grid2LibraryController<Volume>::columnCount@0xe8d2b4` `movs r0,1` (`01 20`) → `02 20`/`03 20`.
- **List multi-column:** patch `AbstractLibraryController<Volume>::columnCount@0xb030c6` `01 20` → `movs r0,#N`. **[SPECULATIVE]** — list rows are a wide single-item layout, so N>1 likely won't tile covers correctly; use the grid path instead.

### Surface (3) — NATIVE INJECTION  [HIGHEST effort, safe-v3]

Only needed for a genuinely NEW arrangement with no existing ViewType/template instantiation (the grid layouts are fixed template instantiations; a byte-patch cannot add a novel scrollable widget). Hook `GenericLibraryView<Volume>::GenericLibraryView@0xcf034c` (same imageformats-plugin `dlopen` vector as NickelHome/HomePageView), then either inject a custom QWidget subtree over `*LibraryView`, or intercept `LibraryBuilder::newController@0xe4dd48` to return your own `AbstractLibraryController<Volume>` subclass (override `columnCount`/`heightForRowInSection`/`addContentForRowInSection`). Requires the `std::function` master-key + `LibraryDataProvider<Volume>` chain to feed Volumes. **Not required for grid/list/cover switching or density tweaks.**

---

## 4. RECIPE — recommended path

**For a density/cover-size change → the single cleanest lever is the 2-byte columnCount patch:**

1. Confirm the live Books screen uses the `GridLibraryController<Volume>` path (not `Grid2LibraryController` / a QML path). Inspect the running controller class or trace `LibraryBuilder::newController@0xe4dd48` branches on-device.
2. Open `libnickel.so.1.0.0` (4.44.23552) at **vaddr `0xb03114`** inside `GridLibraryController<Volume>::columnCount()`.
3. Verify the 2 bytes there are `00 6c` (`ldr r0,[r0,0x40]`, Thumb).
4. Overwrite with `movs r0,#N`: **N=2 → `02 20`** (big covers), **N=3 → `03 20`**, **N=4 → `04 20`** (dense). (Little-endian Thumb: imm in low byte, `0x20` = `movs r0`.)
5. Optionally adjust row count at `N3DeviceCharm::gridViewRowCount@0x1112e48` (currently 3/2).
6. Keep the original bytes; the patch is fully reversible by restoring `00 6c`.

**For a mode switch (list ↔ grid ↔ detail) → do the config edit FIRST (no binary change):**

1. On-device, once, toggle the Books library to the target mode via the Cover/List menu so Nickel writes a known-good value.
2. Read `.kobo/Kobo/Kobo eReader.conf` and locate the `LastLibraryViewType` line (and note its section + whether the key is plain or per-provider composite). This resolves all three §4-blockers empirically.
3. Copy that exact string value; to force it as default, write the same key/value into the conf while Nickel is not running.
4. To pin sort: set `LastLibrarySorter` / `LastLibrarySubFilter` the same way.

**The runtime user toggle** (for reference): `LibraryViewMenuController::onCoverView@0xee8550` / `onListView@0xee8580` tail-call the `coverView()@0xee8530` / `listView()@0xee8560` Qt signals (signal indices 0/1); a builder slot catches these → `setLibraryViewType` + rebuild controller.

---

## Open questions / unverified

- **Exact serialized ViewType strings** written to `LastLibraryViewType` — the switch cases in `serializeViewType@0xb27a00` load `QStringLiteral` headers via pc-relative pools; names (`viewByMxNCoverGrid` etc.) are inferred from method names, **not dumped**. Resolve empirically (recipe step 2 above) before hardcoding.
- **Enum→value mapping conflict** — setters' `str #N` (Pass A: Cover=1/Detail=2/Text=3/List=4) vs serialize-switch order (Pass B). Setters are authoritative for runtime; re-confirm per site.
- **Section name** — `ApplicationPreferences` (traced `sectionName@0xa0baa8`) vs `ApplicationSettings` (other pass). Verify in conf.
- **Key form** — `getLibraryViewType(provider)` takes a provider QString; unclear if effective key is plain `LastLibraryViewType` or per-provider composite. Same for sorter/subfilter.
- **Default fallback mode** — `LibraryBuilder::useDefaultViewType@0xe4da90` calls a `viewBy*` via unresolved virtual dispatch.
- **Which grid path is live** — `GridLibraryController<Volume>` (field +0x40) vs `Grid2LibraryController<Volume>` (const 1) vs QML. Determines which patch to ship.
- **Cover aspect** — constant inside `heightForRowInSection@0xb04278` garbled; unknown if aspect is fixed or purely derived from cell width.
- **Where `GridLibraryController+0x40` is written** — the ctor that sets the default column count from ViewType is not located (the 2-byte getter patch sidesteps this, but locating it would reveal the shipped default).
- **buildController source-store offsets** — `+0x30/+0x34` (in `buildController@0xb2f448`) vs `+0x14/+0x18` (in `setSource`/`NickelGridLibraryControllerBase` ctor): confirm same member/class or two layouts.
- **List multi-column** — whether patched `AbstractLibraryController::columnCount` >1 actually tiles.