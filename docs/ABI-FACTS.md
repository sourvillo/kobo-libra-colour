# Kobo Libra Colour 4.44.23552 — verified ABI / RE facts

Hard-won, on-device-confirmed facts about `libnickel.so.1.0.0` (and the Qt5.2.1 libs)
for firmware **4.44.23552**. Every entry here is something we
paid for with disassembly and/or a reboot cycle — **do not re-derive; reuse.**

Reference binaries (exact hashes pinned): `kobo-liberation/recovery/reference-4.44.23552/`
- `usr/local/Kobo/libnickel.so.1.0.0` (23 MB, ARM 32-bit Thumb-2, 72k symbols)
- Qt5.2.1 shipped under legacy names: `usr/local/Trolltech/QtEmbedded-4.6.2-arm/lib/libQt{Core,Gui,Widgets}.so.4.6.2`
  (the `Qt-5.2.1-arm/lib` dir is broken symlinks — always `rabin2` the `4.6.2` files).

## Toolchain / environment
- Cross-compile a loadable module: `zig c++ -target arm-linux-gnueabihf.2.19 -mcpu=cortex_a9 -shared -fPIC -O2 -fno-exceptions -fno-rtti -nostdlib++ -Wl,--allow-shlib-undefined -Wl,-z,lazy -Wl,-z,nodelete`. Device glibc = **2.19** (the `libc-2.11.1.so` name is misleading), interpreter `/lib/ld-linux-armhf.so.3`, libstdc++ 6.0.12.
- Call Qt/nickel headerless via `extern … asm("_ZN…mangled…")`. Leave Qt symbols undefined → resolved at load inside nickel. Only `operator new` (`_Znwj`) delegated to nickel's libstdc++.
- **LOADER LANDMINE:** glibc 2.11-era eager relocation + `-z now` (which zig adds by default) reads one entry past `.rel.dyn` into `.ARM.exidx` → SIGSEGV in `ld` before the ctor. Fix: `-z lazy` + a linker script forcing `.rel.dyn` → `.rel.plt` → `.ARM.exidx` **contiguous**, no `DT_FLAGS`/`DT_BIND_NOW`. (See `homerow/safe-v3/legacy-loader-layout.ld`.)

## Injecting code into the live nickel process
- nickel auto-loads Qt **imageformats plugins** from `/usr/local/Kobo/imageformats/` (via `QFactoryLoader`, early at startup). Drop a `.so` there and it is `dlopen`ed → its `__attribute__((constructor))` runs **inside nickel**. Same vector NickelMenu (`libnm.so`) uses.
- The `.so` MUST be a valid Qt5.2 imageformats plugin or QFactoryLoader **reads the `.qtmetadata` section statically and never dlopens it** (`-z nodelete` alone is not enough). Copy stock `libkimg.so`'s 304-byte `.qtmetadata` blob verbatim and change only the length-preserving key/MIME (e.g. `kimg`→`ryv3`, `image/x-kobo`→`image/x-ryv3`) so Qt never actually calls `create()`. Export `qt_plugin_query_metadata` + `qt_plugin_instance`.
- Plugins may first be dlopened on a **cover-loader worker thread**, not the GUI thread. Do UI work only on qApp's thread: `QObject::moveToThread(qApp thread)` + `postEvent` a bootstrap, and only start timers when `QThread::currentThread()==qApp thread`.
- Reading a live value out of nickel: `base=$(awk '/libnickel.so/{print $1;exit}' /proc/$(pidof nickel)/maps|cut -d- -f1)`; runtime addr = `0x$base + symbol_vaddr` (first PT_LOAD vaddr is 0); `dd if=/proc/$PID/mem bs=1 skip=$ADDR count=4 | od -An -tu4`.

## The native Home
- Stock Home only exists when `[ApplicationPreferences] SideloadedMode=false` in `.kobo/Kobo/Kobo eReader.conf`. With `true`, `MainWindowController::createHomePage(bool)` routes to Library and **never creates HomePageView**. Requires a valid account-mode `user` row (already present after recovery).
- `HomePageView` — the home QWidget. Data symbols exported: `_ZN12HomePageView16staticMetaObjectE`, `_ZTV12HomePageView`. Compare `widget->metaObject()==&HomePageView::staticMetaObject` to identify it. `QObject` is `QWidget`'s first base (offset 0) so the `QObject*` from an event filter == the `QWidget*`.
- Home builder chain: `HomePageController::configureWidgets()` @0xd80e30 → `HomePageView::configureCurrentReads(QVector<Volume>)` @0xd87d88 (the recent-books row) + `configureMyBooks`, `configureCollection`, `configureTopRight/MiddleRight`.
- **Recent-books row is a fixed 1/3/4-cover layout, NOT scrollable.** `FourBookHomeWidget::arrange()` @0xd8e4d4 divides row width by `maxSlots` (this+0x20, default 4) and `setFixedSize`s each cover → more covers just shrink. No QScrollArea/QHBoxLayout/gesture anywhere in Single/Three/FourBookHomeWidget. A byte-patch cannot add a scrollable widget; only injected native code can.
- v20 home byte-patches that are safe & verified: hide the 2nd stats tile = redirect `Ui_HomePageView::setupUi` +0x714 `setObjectName`→`QWidget::hide`; blank the stats SmartLink = `SmartLinkWidget::showRotationMessage` head → `b.w showEmptyMessage`; tap-to-click in the built-in browser = NOP the network-guard `beq` at `BrowserInputMixin::handleTapAtPoint`+28.

## Volumes (books)
- `Volume` is **8 bytes** (vptr + shared pimpl). ctor `_ZN6VolumeC1Ev` size 88, dtor `_ZN6VolumeD1Ev` size 76 → non-trivial, but see getById below. `Volume::isValid()` = `_ZNK6Volume7isValidEv`.
- **`VolumeManager::getById(QString contentId, QString user)` @0xa842b0 is a STATIC method** (no `this`!) and **cache-only**. ABI: `r0=sret(Volume), r1=contentId, r2=user`. It computes `Repository::cacheKey(contentId,user)` and does `QHash<QByteArray,Volume>::findNode` on a GOT global; **on a cache miss it returns an invalid Volume — it does NOT load from the DB.** Passing a `VolumeManager*` as a first arg (assuming a normal member) shifts the args and makes it treat the manager pointer as the contentId QString → `QVariant(QString)` refcounts the manager's read-only vtable → **SIGSEGV**. (This cost us one crash cycle.)
- `getByIdBytes(QByteArray,QString)` @0xa85120 is ALSO cache-only (same `findNode` pattern). Not a loader.
- **OPEN PROBLEM:** need a method that LOADS a Volume from the DB by ContentID (getById only returns cached ones, so arbitrary selected books come back invalid). The library/Home populates the cache via a query, then getById hits. Confirmed DB-path methods (they build an `AbstractBuilder<Volume>` = the query builder, unlike getById's bare `QHash::findNode`):
  - `VolumeManager::find(QString, std::function<bool(Volume const&)>)` **@0xa83d1c** — decompile shows it constructs+destructs an `AbstractBuilder<Volume>` (0x6b011c dtor) and iterates; QString arg = filter, callback returns bool (keep going / stop).
  - `VolumeManager::forEach(QString, std::function<void(Volume const&)>)` **@0xa839fc** — same builder, void callback.
  - `VolumeManager::retrieveBooks(User, bool, bool, QString)` **@0xa83c80**; `forEachBook(User,bool,bool,QString,std::function<void(Volume)>)` @0xa83c00.
  - **Blocker to wire up:** all take a `std::function` → must hand-build one matching device libstdc++ 6.0.12 layout (2-word storage + invoker ptr + manager ptr), or find a stock lambda/functor to reuse. This is the next step for the native pager. See `firmware-analysis/README.md`.
  - Alternative not yet explored: let Home/Library populate the cache first (they run the query on startup), THEN getById hits — i.e. only pager books that are ALSO recent/on-a-shelf would be cached. Too fragile for arbitrary 10 books.
- **Stock DB-load path (how real Home gets its books):** `HomePageController` uses `LibraryDataProvider<Volume>` / `AllVolumesLibraryDataProvider(Device const&)` @0xb33588 → these are DB-backed. `AllVolumesLibraryDataProvider::forEach(std::function<void(Volume const&)>)` @0xb335e8 and `::contents()` @0xb330b0 yield REAL loaded Volumes. `HomePageController::getCurrentReads()` @0xd7ffe4 returns a `QVector<Volume>` already loaded. Data providers also compose: `FilteredLibraryDataProvider<Volume>(LibraryDataProvider<Volume>*, FilterBase<Volume>*)` @0xd87250 → wrap-and-filter to narrow to specific books.

## ⭐ THE MASTER KEY — a hand-built `std::function`
Almost every nickel data API (`VolumeManager::find/forEach`, every `LibraryDataProvider<Volume>::forEach`, `AllVolumesLibraryDataProvider::forEach`) takes a **`std::function<void/bool(Volume const&)>`** by const-ref. Build ONE such object in our injected code and *all* DB loaders + iterators become callable — this is the single highest-leverage primitive for building native features.
- libstdc++ 6.0.12 (gcc ~4.9), 32-bit ARM layout of `std::function` = **16 bytes**: `_Any_data _M_functor` (8 bytes = 2 words of inline storage/union), `_Manager_type _M_manager` (4-byte fn ptr), `_Invoker_type _M_invoker` (4-byte fn ptr).
- To synthesize one that captures a context pointer `ctx` and calls our C function `cb(ctx, const Volume&)`:
  - `_M_functor.first_word = ctx` (store our pointer inline),
  - `_M_invoker = &our_invoker` where `our_invoker(const _Any_data& d, const Volume& v)` reads `ctx = *(void**)&d` and calls `cb(ctx, v)`,
  - `_M_manager = &our_manager` handling `__clone_functor`/`__destroy_functor`/`__get_type_info` — for our injected-lifetime use, clone = shallow-copy the 8 bytes, destroy = no-op, so nickel's internal copy of the std::function stays valid.
  - A non-null `_M_manager` is required or `std::function::operator()` throws bad_function_call. Verify by reusing nickel's own `std::_Function_handler<void(Volume const&), …>::_M_invoke` / `_M_manager` pairs already in the binary (e.g. @0xb2feb4 / @0xb2b1d4) as templates for the exact ABI, or point our std::function's manager+invoker AT those stock ones if a matching lambda-capture layout exists.
- **Once this works:** load any book by ContentID via `AllVolumesLibraryDataProvider::forEach` + a filter on `v.getContentID()==id`, or via `VolumeManager::find(filter, cb)`. Resolves `build_fail: Volume invalid`. This is the #1 analysis/build priority.

## Gestures
- `SwipeGestureRecognizer::_gestureType` (`_ZN22SwipeGestureRecognizer12_gestureTypeE`, a 4-byte static int) is assigned at runtime by `registerRecognizer` starting from `Qt::CustomGesture` (256). Observed value = **262** (0x106) and it can drift by a few between boots depending on registration order → **check a range [256,511], never `==256`.** Use the runtime value for `QWidget::grabGesture`.
- Kobo's custom `SwipeGesture` stores raw direction ints at gesture **+0x2c (horizontal)** and **+0x30 (vertical)**; stock `GenericTourView` reads them only for `QGesture::state()==1` (GestureStarted). `HorizontalBookStrip` max items = **5** (hard constant; `maxItems` @0x110183c returns 5, `addContentForRowInSection` early-returns at count==maxItems).
- HBS secondary `GestureDelegate` vtable is at `&_ZTV19HorizontalBookStrip + 0x17c`; slot 3 = swipe, slot 4 = tap. Clone the already-relocated 11-word vtable, replace only slot 3, keep tap.

## Qt5.2 ABI helpers (verified)
- `QObject` vtable (`_ZTV7QObject`, 56 bytes = 2 header + 12 slots): [0]metaObject [1]qt_metacast [2]qt_metacall [3]~(D2) [4]~(D0) [5]event [6]**eventFilter** [7]timerEvent [8]childEvent [9]customEvent [10]connectNotify [11]disconnectNotify. Build a synthetic QObject by constructing a real one then copying+patching this vtable (slot 6 for a filter).
- `QImageIOPlugin` vtable (`_ZTV14QImageIOPlugin`, 64 bytes = 16 words): slot 12 = capabilities, slot 13 = create (both pure-virtual in base).
- `QString::fromUtf8_helper(const char*,int)` @libQtCore 0xd7228 IS sret: `r0=QString buffer, r1=str, r2=size`; for `str==NULL` it stores the shared-null. A QString is `{ QStringData* d; }` = 4 bytes.
- `QEvent` type is a `ushort` at event **+8**; `QTimerEvent` timer id is an int at **+12**.
