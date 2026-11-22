<!-- Generated 2026-07-16 by the firmware-analysis workbench (multi-agent pass 1).
     SPOT-VERIFIED on-binary: Volume::d()=ldr[r0,4]; ctor pimpl=0x198(408B);
     Title@0x10 Attribution@0x18 ContentType@0x74 ReadStatus@0x7c confirmed;
     DatabaseAdapter::createQuery@0x6d2598/executeDbQuery@0x6d0788 real;
     ReadingView::keyPressEvent@0xf8f1e8 real. A few getter addresses in this doc
     are a few bytes off (agent artifact) — offsets/semantics hold; re-confirm an
     exact address with tools/decompile.sh before hardcoding it into a patch. -->

# Volume / Content in-memory model — libnickel 4.44.23552

Reconstructed by decompiling the `Content::getX` accessors. **There is no C++ class named
`ContentModel`** — "ContentModel" in the task title refers to this reconstructed Volume/Content
pimpl layout (the closest thing Nickel has; the library grid uses a Provider/DataSource triad,
not a QAbstractItemModel — see db_load_recipe_md §Library path).

## Handle vs pimpl (QSharedData pattern)
`Volume` and `Content` are thin **8-byte handles**, not the data itself:

```
Volume  (8 bytes, on stack)          VolumePrivate  (pimpl, 408 = 0x198 bytes, heap, shared)
+0x00  vptr  ────► _ZTV6Volume       +0x00  int  QSharedData/CacheObject<true> atomic refcount
+0x04  VolumePrivate* pimpl ─────────►+0x04  ...fields below...
```

- **Evidence (handle):** `Volume::Volume()` @0xa7baa0 — `str r3,[r5]` writes vtable at +0;
  `str r4,[r5,4]` writes the freshly-`operator new`'d pimpl at +4. `Volume::d()` @0xa78b18 is
  exactly `ldr r0,[r0,4]; bx lr`.
- **Evidence (pimpl size):** ctor `mov.w r0,0x198` → `operator new` (`_Znwj` @0x67ee7c); pimpl
  ctor = reloc `_ZN13VolumePrivateC1Ev` via GOT 0x16f98a0.
- **Evidence (refcount@0):** right after storing pimpl, ctor does
  `ldrex r3,[r4]; add r3,#1; strex r2,r3,[r4]` on r4=pimpl. RTTI base = `_ZTI11CacheObjectILb1EE`.
- **Inheritance:** `Content : CacheObject<true> : ScObject`; `Volume : Content` (single-inheritance
  data chain). Distinct vtables `_ZTV6Volume` (56 B) / `_ZTV7Content` (48 B).
- **Data accessor is virtual slot 8** (`vptr+0x20`) = `Volume::d()` returning `*(this+4)` = pimpl.
  Every `Content::getX` starts `ldr r3,[r0]; ldr r3,[r3,0x20]; blx r3` then reads the field off
  the returned pimpl. In base `_ZTV7Content` that slot is `__cxa_pure_virtual` (abstract).

## VolumePrivate field layout ↔ KoboReader.sqlite `content` column
All offsets are **into the pimpl** (i.e. after `Volume::d()`), confirmed by disassembly of each getter.

| pimpl off | Field | Type | DB `content` column | Getter (evidence) |
|---|---|---|---|---|
| `0x00` | refcount | int (QAtomicInt) | — | ctor ldrex/strex |
| `0x04` | ContentID | QByteArray/QString d-ptr | **ContentID** | `Content::getId` @0x96999a `ldr r3,[r0,4]` (also idBytes) |
| `0x08` | BookID / entitlement key | QString d-ptr | **BookID** | `getBookId` @0x96acb2 `ldr r3,[r0,8]` |
| `0x10` | Title | QString d-ptr | **Title** | `getTitle` @0x96ab7c `ldr r3,[r0,0x10]` |
| `0x18` | Attribution/Author | QString d-ptr | **Attribution** | `getAttribution` @0x96ac32 `ldr r3,[r0,0x18]` |
| `0x20` | PublicationDate | QString/QDateTime d-ptr | **DateCreated** | `getPublicationDate` @0x96ad3e `[r0,0x20]` |
| `0x24` | ImageId / cover key | QString d-ptr | **ImageId** | `getImageId` @0x96d23e `[r0,0x24]`; `getImageIdRaw` @0x96d220 same offset |
| `0x28` | DateLastRead | QString/QDateTime d-ptr | **DateLastRead** | `getDateLastRead` @0x86f968 `[r0,0x28]` |
| `0x2c` | Publisher | QString d-ptr | **Publisher** | `getPublisher` @0x96d2c2 `[r0,0x2c]` |
| `0x34` | MimeType | QString d-ptr (getter returns &field) | **MimeType** | `getMimeType` @0x96d38e `adds r0,0x34` |
| `0x38` | UserID | QString d-ptr | **___UserID** | `getUserID` @0x96d3a2 `[r0,0x38]` |
| `0x60` | Properties / attribute QMap | QMap<QString,QVariant> d-ptr | *(holds Series, Subtitle, ___PercentRead)* | `getProperties` @0x96da72 `[r0,0x60]` |
| `0x68` | DateModified | QString/QDateTime d-ptr | **DateModified / ___SyncTime** | `dateModified` @0x96d968 `[r0,0x68]` |
| `0x6c` | Accessibility | int/enum | **Accessibility** | `getAccessibility` @0x96d452 `[r0,0x6c]` |
| `0x74` | ContentType | int | **ContentType** (6=book, 9=chapter, 899=shelf) | `getContentType` @0x96d36e `[r0,0x74]` |
| `0x78` | NumPages | int | **___NumPages** | `getNumPages` @0x96d296 `[r0,0x78]` |
| `0x7c` | ReadStatus | int | **ReadStatus** (0/1/2/3) | `getReadStatus` @0x96d472 `[r0,0x7c]` |
| `0x80` | ParagraphBookmarked | int | **ParagraphBookmarked** | `getParagraphBookmarked` @0x96d482 `ldr.w [r0,0x80]` |
| `0x84` | BookmarkWordOffset | int | **BookmarkWordOffset** | `getBookmarkWordOffset` @0x96d492 `ldr.w [r0,0x84]` |
| `0x88` | VolumeIndex | int | **VolumeIndex** | `getVolumeIndex` @0x96ad22 `ldr.w [r0,0x88]` |
| `0x90` | Depth | int | **Depth** | `getDepth` @0x96da0a `ldr.w [r0,0x90]` |

### Struct-style diagram
```c
// libnickel 4.44.23552 — reconstructed, offsets = binary-confirmed
struct VolumePrivate {          // 0x198 (408) bytes, refcounted, shared
    int      refcount;          // 0x00  QSharedData/CacheObject<true>
    QString  contentId;         // 0x04  -> content.ContentID   (PK; file:// or GUID)
    QString  bookId;            // 0x08  -> content.BookID
    /* 0x0c ??? */              //       (open: TitleRaw/Kana? — see below)
    QString  title;             // 0x10  -> content.Title
    /* 0x14 ??? */
    QString  attribution;       // 0x18  -> content.Attribution
    /* 0x1c ??? */
    QString  publicationDate;   // 0x20  -> content.DateCreated
    QString  imageId;           // 0x24  -> content.ImageId
    QString  dateLastRead;      // 0x28  -> content.DateLastRead
    QString  publisher;         // 0x2c  -> content.Publisher
    /* 0x30 ??? */
    QString  mimeType;          // 0x34  -> content.MimeType
    QString  userId;            // 0x38  -> content.___UserID
    /* 0x3c..0x5c ??? run of QString/QByteArray d-ptrs
       (likely Description/ShortCoverKey/adobe_location/ContentURL/Language) */
    QMap<QString,QVariant> properties; // 0x60 -> Series, SeriesNumber, Subtitle, ___PercentRead
    /* 0x64 ??? */
    QString  dateModified;      // 0x68  -> content.DateModified / ___SyncTime
    int      accessibility;     // 0x6c  -> content.Accessibility
    /* 0x70 ??? */
    int      contentType;       // 0x74  -> content.ContentType   (6/9/899)
    int      numPages;          // 0x78  -> content.___NumPages
    int      readStatus;        // 0x7c  -> content.ReadStatus
    int      paragraphBookmarked;// 0x80 -> content.ParagraphBookmarked
    int      bookmarkWordOffset;// 0x84  -> content.BookmarkWordOffset
    int      volumeIndex;       // 0x88  -> content.VolumeIndex
    /* 0x8c ??? */
    int      depth;             // 0x90  -> content.Depth
    /* 0x94 .. 0x198 : ~0x108 bytes unenumerated
       (FileSize, entitlement, sync tokens, flags — routed via helper getters) */
};

struct Volume { void** vptr; VolumePrivate* d; };  // 8 bytes
```

## Critical modeling note — Series / Subtitle / ___PercentRead are NOT fixed slots
There is **no** `Content::getSeries/getSubtitle/getPercentRead`. They live in the **attribute/property
bag at pimpl+0x60**:
- `Content::setPercentRead` @0x970368 builds a **QVariant** and stores it in the map (calls
  `operator new`), rather than a plain `str [r0,#off]`.
- Series/Subtitle are read via `SeriesKey<Volume>::key` @0x839550 / `SubtitleKey<Volume>::key`
  @0x83933c (attribute lookups on the +0x60 map).
- ⚠️ **Open:** whether `___PercentRead` also has a dedicated int field past 0x94 (within the 408-byte
  object) — the QVariant path suggests map-only, but no `getReadingProgress`-style reader was found
  to confirm. Treat +0x60 as the source of truth for now.

## Open questions (flagged speculative)
- pimpl `0x0c, 0x14, 0x1c, 0x30, 0x3c–0x5c` — run of QString/QByteArray d-ptrs, exact identity not
  isolated (likely TitleRaw/TitleKana/AttributionRaw/AttributionKana/Description/ShortCoverKey/
  adobe_location/ContentURL/Language). `getTitleRaw`/`getAttributionRaw` confirmed to route through
  `d()` but offsets not pinned.
- `0x64, 0x70, 0x8c` — unmapped.
- Date fields (0x20/0x28/0x68): single 4-byte d-ptr consistent with both Qt5 QString and QDateTime;
  DB column is TEXT so QString likely, not disassembled to confirm.
- `0x94..0x198` (~264 bytes) beyond Depth: FileSize/entitlement/sync tokens — Volume-specific getters
  (`fileSize`@0xa790b4, `entitlementId`) route through helpers, not a single `ldr`.

## DB-column context (KoboReader.sqlite, from kobo.koplugin docs + MobileRead)
- `ContentType`: 6 = book row (overall `___PercentRead`), 9 = chapter row
  (`___FileOffset` = cumulative % where chapter starts, `___FileSize` = % span). Overall progress
  reconstructed as `___FileOffset + ___FileSize*chapterPct/100`.
- `ContentID`: type-6 = book id (file:// path sideloaded, GUID for store); type-9 = `bookId!chapterPath[#kobo.x.y]`.
- Highlights live in the **`Bookmark`** table — `Text`, `Annotation`, `Color` (INTEGER, per-highlight
  color for Libra Colour — **directly relevant to the export-with-colors goal**), `Hidden`,
  `ChapterProgress` (REAL 0..1), `StartContainerPath`/`EndContainerPath`. DRM keys in `content_keys`
  (PK volumeId,elementId). **No firmware patch needed to export highlights+colors — query the DB.**
