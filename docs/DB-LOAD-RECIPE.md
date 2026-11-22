<!-- Generated 2026-07-16 by the firmware-analysis workbench (multi-agent pass 1).
     SPOT-VERIFIED on-binary: Volume::d()=ldr[r0,4]; ctor pimpl=0x198(408B);
     Title@0x10 Attribution@0x18 ContentType@0x74 ReadStatus@0x7c confirmed;
     DatabaseAdapter::createQuery@0x6d2598/executeDbQuery@0x6d0788 real;
     ReadingView::keyPressEvent@0xf8f1e8 real. A few getter addresses in this doc
     are a few bytes off (agent artifact) — offsets/semantics hold; re-confirm an
     exact address with tools/decompile.sh before hardcoding it into a patch. -->

# Recipe: load a Volume by ContentID at the nickel object level — 4.44.23552

`VolumeManager::getById` @0xa842b0 is **cache-only** (bare `QHash::findNode`) — it returns nothing
for a book not already in the in-RAM cache. To force a DB load you need one of the two recipes below.
All addresses are file vaddrs in reference libnickel; add the runtime load base
(`awk` over `/proc/<nickel-pid>/maps`) for the live process.

---

## Recipe B (RECOMMENDED for a single known ContentID) — DatabaseAdapter, no functor
No `std::function` to forge. Pure query path.

1. Build SQL. Table + columns are self-describing in the binary:
   `Volume::getTableName()` @0xa78d30 (→ the **`content`** table; ⚠️ literal inferred, dump the
   QStringData to confirm before hardcoding) and `Volume::getDbRetrieveFields()` @0xa7a6bc (the SELECT
   column list). Compose e.g. `SELECT <fields> FROM content WHERE ContentID = '<id>' AND ContentType = 6`.
2. `DatabaseAdapter::createQuery(QString sql)` @0x6d2598 — ABI: `r0` = sret `QSqlQuery`, `r1` = QString sql.
   (Optionally `dbConnection(QString,bool)` @0x6d13cc to pick the connection; `exists(QSqlQuery&)` @0x6d3394.)
3. `DatabaseAdapter::executeDbQuery(QSqlQuery&)` @0x6d0788 — `r0` = `QSqlQuery&`.
4. `QVector<Volume> DatabaseAdapter::retrieve<Volume>(QSqlQuery&)` @0xa7f278 — builds Volumes row-by-row
   into a QVector (decompile shows Volume-building helpers + `ldr [r2,0xc]` title-field access).
5. Take element 0 → your loaded `Volume`. Read fields per volume_content_model_md
   (`Content::getId`@0x96998c, `getTitle`@0x96ab6c, or direct pimpl int reads).

⚠️ Open: which named DB connection createQuery/executeDbQuery resolve to, and whether they're safe
off the GUI thread — verify before use in injected code.

---

## Recipe A (general master-key iterator) — VolumeManager::forEach / find + hand-built std::function
Use when you need to scan/filter arbitrary books (these are DB-backed: they build an
`AbstractBuilder<Volume>` internally, unlike getById).

### Entry points
- `VolumeManager::forEach(QString const& filter, std::function<void(Volume const&)> const&)` @0xa839fc
  — **STATIC**. ABI: `r0` = QString const& (filter), `r1` = std::function const&; void return, no sret,
  no `this` (singleton loaded from a GOT global).
- `VolumeManager::find(QString const& filter, std::function<bool(Volume const&)> const&)` @0xa83d1c
  — returns first Volume where predicate ≠ 0. ABI: `r0` = sret Volume (8 B), `r1` = QString filter,
  `r2` = std::function const&.
- `VolumeManager::retrieveFiltered(QString, std::function<bool(Volume const&)>)` @0xa830d4 — DB-backed filtered.
- `AllVolumesLibraryDataProvider::forEach(std::function<void(Volume const&)> const&)` @0xb335e8 — one arg
  `r1` = std::function, `r0` = this(provider).

⚠️ **Filter-string semantics unconfirmed.** The QString filter is passed to import stub 0x6b011c
(the same slot getById uses for the *user id*), so it is most likely a **user id / provider-view key,
NOT an arbitrary SQL WHERE**. **Mitigation: pass an empty/permissive filter and match ContentID
inside the callback** rather than trusting the filter.

### The std::function bridge (libstdc++ 6.0.12, 32-bit ARM) — CONFIRMED 16-byte layout
```c
// _Any_data is 8 bytes (two words); a single captured pointer uses word 0 only.
struct std_function_16 {          // base pointer = &this
    void*  functor_word0;  // +0x00  _M_functor._M_unused._M_object  = your ctx (e.g. &target_id_set)
    void*  functor_word1;  // +0x04  (unused for single-pointer capture)
    void*  manager;        // +0x08  _Manager_type  (*)(_Any_data&, const _Any_data&, _Manager_operation)
    void*  invoker;        // +0x0c  _Invoker_type  R (*)(const _Any_data&, Volume const&)
};
```
Evidence (from forEach @0xa839fc, working copy base r6 = r7+0x10):
functor read @r7+0x10, **manager @r7+0x18 (+8)**, **invoker @r7+0x1c (+12)**. Invoke is
`r0=&_M_functor, r1=&Volume, blx invoker`; destroy is `manager(&f,&f, /*op*/3)`.

### CRITICAL: manager MUST implement clone (op 2), not just destroy
forEach **copies the passed std::function TWICE** before iterating (0xa83a06 copy#1 into r7+0,
0xa83a54 copy#2 into r7+0x10; both via std::function copy-ctor 0x68ca58 which calls
`_M_manager(dest, src, __clone_functor=2)`). If your manager no-ops clone, the copies get a null/garbage
functor and the invoke dereferences bad ctx.

`_Manager_operation` enum: `__get_type_info=0, __get_functor_ptr=1, __clone_functor=2, __destroy_functor=3`.

### Minimal hand-built bridge (borrowed-ctx, shallow clone — safe for forEach/find scope only)
```c
// Our invoker: ctx is a plain pointer we captured in functor_word0.
// R = void for forEach; R = bool (nonzero=stop/keep) for find.
static void  my_invoke_void(void* fn /*&_M_functor*/, void* vol /*Volume const&*/) {
    void* ctx = *(void**)fn;            // functor_word0
    on_volume(ctx, vol);                // your logic: read ContentID, compare, collect
}
static bool  my_invoke_bool(void* fn, void* vol) {
    void* ctx = *(void**)fn;
    return volume_matches(ctx, vol);    // true => find() stops and returns this Volume
}
// Manager: handle clone(2) as a shallow copy of the 8-byte _Any_data; destroy(3) = no-op
// (we borrow ctx, own nothing). get_type_info(0)/get_functor_ptr(1) can return 0.
static void* my_manager(void* dest, const void* src, int op) {
    if (op == 2 /*clone*/) { ((void**)dest)[0] = ((void* const*)src)[0];
                             ((void**)dest)[1] = ((void* const*)src)[1]; }
    return 0;                            // destroy/others: nothing to free
}
// Assemble:
struct std_function_16 f;
f.functor_word0 = &target_id_set;       // ctx
f.functor_word1 = 0;
f.manager       = (void*)my_manager;
f.invoker       = (void*)my_invoke_void; // or my_invoke_bool for find
// forEach: r0=&emptyQString, r1=&f.  find: r0=&sretVolume, r1=&filterQString, r2=&f.
```
The borrowed-ctx shallow clone + no-op destroy is **safe** because forEach/find copy then destroy
their local copies *within the call* (verified). A provider that stored the std::function longer-lived
would need a real clone/destroy — not the case here.

### Reusable stock invoker/manager candidates — ⚠️ ABI template ONLY, do NOT reuse verbatim
The only `std::function<void(Volume const&)>` handler pair in the binary is the
`FilteredLibraryDataProvider<Volume>::contents` lambda: `_M_invoke` @0xb2feb4, `_Base_manager::_M_manager`
@0xb2b1d4. `_M_invoke` assumes that lambda's specific capture layout (reads captured provider ptr,
`+0x1c`, rebuilds a QVector) — it is **not** a generic `cb(ctx,&v)` passthrough. `_M_manager` @0xb2b1d4
is a standard 4-op `_Base_manager` (clone allocates+copies the captured pointer) and is a good structural
reference, but still assumes that capture. **Build your own (above); use these two only to cross-check
your struct offsets and the 4-op switch shape.**

---

## Direct-SQLite fallback (no libnickel at all)
For read-only needs (stats, highlight export, "load these 10 books") this is the simplest and safest.
```sql
-- book row
SELECT ContentID, Title, Attribution, ImageId, ___PercentRead, ReadStatus, DateLastRead
FROM content WHERE ContentID = ? AND ContentType = 6;
-- highlights WITH colors (Libra Colour) for the liberation goal
SELECT VolumeID, Text, Annotation, Color, ChapterProgress, StartContainerPath
FROM Bookmark WHERE VolumeID = ? AND (Hidden IS NULL OR Hidden NOT IN ('true',1))
ORDER BY ChapterProgress;
```
Notes: KoboReader.sqlite `content` table is open/unencrypted; `Bookmark.Color` is INTEGER; `Hidden` is
stored inconsistently (`'true'`/`'false'` text or 0/1) — filter defensively. Chapter rows (ContentType=9)
carry `___FileOffset`/`___FileSize` for reconstructing overall progress. **For highlight-with-color
export, no firmware patch is required** — the DB has everything. (FeatureSettings::exportHighlightsEnabled
@0x00a19778 only adds the *native menu item*; the data path is the DB.)

---

## Decision guide
- One known ContentID, read-only → **direct SQLite** (safest) or **Recipe B**.
- Need live nickel Volume objects (to feed them into the running UI / providers) → **Recipe B**, then hand
  them to the library grid via LinearLibraryDataSource + AbstractLibraryController::setSource@0xb0815c.
- Scan/filter arbitrary books inside injected code → **Recipe A** (forEach/find) with the hand-built bridge.

## Remaining blockers (flagged)
1. `Volume::getTableName` "content" literal — inferred, dump QStringData to confirm.
2. forEach/find filter-string semantics — treat as user-id/view-key, match ContentID in-callback.
3. `Content::getId` returns a QString id; not 100% verified it equals the getById cache key vs an
   internal rowid — cross-check against a known ContentID at runtime.
4. DatabaseAdapter connection name + thread-safety — verify before use in injected code.
5. No exported `Volume::getContentID()` — read pimpl+0x04 directly, or use `entitlementId()`@0xa797e4
   (==ContentID for sideloaded books; unverified for store GUIDs).
