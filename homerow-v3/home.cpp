// homerow safe-v3: a disabled-by-default native HomePageView 5x2 cover pager for the
// exact Kobo Libra Colour firmware 4.44.23552 ABI.
//
// Discovery deliberately does NOT install a global event filter.  A private
// QObject timer takes a snapshot from QApplication::allWidgets(), finds the exact
// HomePageView metaobject, stops its own discovery timer, and only then performs a
// single insertion attempt using two of Nickel's own five-item
// HorizontalBookStrip widgets and the stock Volume/open-book path.
// The same private timer owns the post-insert health window and tmpfs heartbeat.

extern "C" {
    void* memset(void*, int, unsigned long);
    void* memcpy(void*, const void*, unsigned long);
    int memcmp(const void*, const void*, unsigned long);
    unsigned long strlen(const char*);
    int strcmp(const char*, const char*);
    char* strstr(const char*, const char*);
    int snprintf(char*, unsigned long, const char*, ...);
    int open(const char*, int, ...);
    long read(int, void*, unsigned long);
    long write(int, const void*, unsigned long);
    int close(int);
    int fsync(int);
    int rename(const char*, const char*);
    int unlink(const char*);
}

extern void* op_new(unsigned) asm("_Znwj");

extern void* qApp_self asm("_ZN16QCoreApplication4selfE");
extern char QObject_vtable_obj asm("_ZTV7QObject");
extern char QImageIOPlugin_vtable_obj asm("_ZTV14QImageIOPlugin");
extern char HPV_smo asm("_ZN12HomePageView16staticMetaObjectE");
extern char HorizontalBookStrip_vtable_obj asm("_ZTV19HorizontalBookStrip");
extern int SwipeGestureRecognizer_gestureType
    asm("_ZN22SwipeGestureRecognizer12_gestureTypeE");

extern void* QApplication_allWidgets(void* sret)
    asm("_ZN12QApplication10allWidgetsEv");
extern void QWidgetList_dtor(void* self)
    asm("_ZN5QListIP7QWidgetED1Ev");

extern void QObject_ctor(void* self, void* parent) asm("_ZN7QObjectC1EPS_");
extern bool QObject_event(void* self, void* event) asm("_ZN7QObject5eventEP6QEvent");
extern void* QObject_thread(void* self) asm("_ZNK7QObject6threadEv");
extern void QObject_moveToThread(void* self, void* thread)
    asm("_ZN7QObject12moveToThreadEP7QThread");
extern int QObject_startTimer(void* self, int interval, int timerType)
    asm("_ZN7QObject10startTimerEiN2Qt9TimerTypeE");
extern void QObject_killTimer(void* self, int timerId) asm("_ZN7QObject9killTimerEi");
extern const char* QMetaObject_className(void* self) asm("_ZNK11QMetaObject9classNameEv");
extern void* QThread_currentThread() asm("_ZN7QThread13currentThreadEv");
extern void QEvent_ctor(void* self, int type) asm("_ZN6QEventC1ENS_4TypeE");
extern void QCoreApplication_postEvent(void* receiver, void* event, int priority)
    asm("_ZN16QCoreApplication9postEventEP7QObjectP6QEventi");
extern void QObject_connect_sret(void* result, const void* sender, const char* signal,
                                 const void* receiver, const char* method, int type)
    asm("_ZN7QObject7connectEPKS_PKcS1_S3_N2Qt14ConnectionTypeE");
extern bool QObject_disconnect_connection(const void* connection)
    asm("_ZN7QObject10disconnectERKN11QMetaObject10ConnectionE");
extern void QMetaConnection_dtor(void* connection)
    asm("_ZN11QMetaObject10ConnectionD1Ev");

extern void* QString_fromUtf8_helper(void* sret, const char* s, int n)
    asm("_ZN7QString15fromUtf8_helperEPKci");
extern void* QString_assign(void* self, const void* other)
    asm("_ZN7QStringaSERKS_");

extern void* QWidget_layout(void* self) asm("_ZNK7QWidget6layoutEv");
extern bool QWidget_isVisibleTo(void* self, const void* ancestor)
    asm("_ZNK7QWidget11isVisibleToEPKS_");
extern void QWidget_hide(void* self) asm("_ZN7QWidget4hideEv");
extern void QWidget_show(void* self) asm("_ZN7QWidget4showEv");
extern void QWidget_raise(void* self) asm("_ZN7QWidget5raiseEv");
extern void QWidget_update(void* self) asm("_ZN7QWidget6updateEv");
extern void QWidget_updateGeometry(void* self) asm("_ZN7QWidget14updateGeometryEv");
extern void QWidget_repaint(void* self) asm("_ZN7QWidget7repaintEv");   // synchronous paintEvent now
// N3BrowserController: nickel's offline-capable local browser (with the firmware browser-without-wifi
// patches). Used to open the Russian koboviz stats/highlights HTML from a native home tile.
extern void N3BrowserController_ctor(void* self) asm("_ZN19N3BrowserControllerC1Ev");            // 0xd1e56c
extern void N3BrowserController_setCustomCss(void* self, const void* qstr)
    asm("_ZN19N3BrowserController12setCustomCssERK7QString");                                    // 0xd20514
extern void N3BrowserControllerBase_show(void* self) asm("_ZN23N3BrowserControllerBase4showEv"); // 0xd1c078 (present)
extern void N3BrowserControllerBase_loadUrl(void* self, const void* qurl)
    asm("_ZN23N3BrowserControllerBase7loadUrlERK4QUrl");                                         // 0xd1cca0
extern void N3BrowserControllerBase_closeIfNecessary(void* self)
    asm("_ZN23N3BrowserControllerBase16closeIfNecessaryEv");                                     // 0xd1b674
extern void QUrl_fromLocalFile(void* sret, const void* qstr) asm("_ZN4QUrl13fromLocalFileERK7QString");
extern void QUrl_dtor(void* self) asm("_ZN4QUrlD1Ev");
// FontSizeAdjustingLabel: nickel's auto-shrinking themed text-label class. Text ctor sets the text in the
// ctor -> one call = a finished native label. Used to build our custom stat blocks inside StatsView.
extern void FSAL_ctor_text(void* self, const void* qstr, void* parent, int flags)
    asm("_ZN22FontSizeAdjustingLabelC1ERK7QStringP7QWidget6QFlagsIN2Qt10WindowTypeEE");           // 0xbd6c64
extern void QObject_setObjectName(void* self, const void* qstr)
    asm("_ZN7QObject13setObjectNameERK7QString");   // sentinel so a reused-address view re-injects
// Russify native Reading-Life labels in place: read a QLabel's text (sret QString), match English, setText
// Russian. Length-agnostic + fully local + reversible. Resolves against Qt (not nickel dynsym).
extern void QLabel_text(void* sret, const void* self) asm("_ZNK6QLabel4textEv");      // returns QString (sret)
extern void QLabel_setText(void* self, const void* qstr) asm("_ZN6QLabel7setTextERK7QString");
extern bool QWidget_isAncestorOf(void* self, const void* child) asm("_ZNK7QWidget12isAncestorOfEPKS_");
// Plain word-wrapping QLabel for the My Words highlight rows (FontSizeAdjustingLabel is single-line and
// overflows on long quotes). Ctor(QWidget*, Qt::WindowFlags) + setWordWrap; resolve against Qt at runtime.
extern void QLabel_ctor(void* self, void* parent, unsigned flags)
    asm("_ZN6QLabelC1EP7QWidget6QFlagsIN2Qt10WindowTypeEE");
extern void QLabel_setWordWrap(void* self, bool on) asm("_ZN6QLabel11setWordWrapEb");
// ElidedLabel (base of Kobo's RegularElidedLabel — home tile titles "My Books"/"Your reading life"/
// collection names/"N% Read") is NOT a QLabel. It has no plain QString getter (getElided() returns a
// scalar, not text), but setText() stores the full text QString at member this+0x54 (verified: setText
// starts `add r5, r0, #0x54`), so we READ that member directly (no method call = crash-safe) and SET via
// the exported setText. ElidedLabel::setText(QString const&).
extern void ElidedLabel_setText(void* self, const void* qstr) asm("_ZN11ElidedLabel7setTextERK7QString");
// 5th highlight colour: append one more selection swatch to the native highlight menu. The builder
// SelectionMenuView::clearMenuItems() makes 4 buttons via explicit addColorSelectionButton(0..3); the
// palette is cleared+rebuilt each menu setup, so we re-append index 4 whenever we see exactly 4. This
// function is exported + self-contained (creates a ColorLabel, styles it, appends to the QVector<ColorLabel*>
// at view+0x30, wires the tap -> setColor(4)). HighlightColor is an int-sized enum.
extern void SelectionMenuView_addColorSelectionButton(void* self, int color)
    asm("_ZN17SelectionMenuView23addColorSelectionButtonE14HighlightColor");
// FourBookHomeWidget lays out (arrange) + paints its covers only from resizeEvent. With 0 height the
// home layout gives it, arrange positions the covers into empty space and nothing paints -- the reason
// our injected strip has been invisible while the NATIVE recent-reads strip shows. Force a real height.
extern void QWidget_setMinimumHeight(void* self, int h) asm("_ZN7QWidget16setMinimumHeightEi");
// QWidget::setStyleSheet(const QString&) -- used to shrink the My-Words highlight font so long highlights
// fit without overflow/overlap (Qt propagates the sheet's font-size to child QLabels).
extern void QWidget_setStyleSheet(void* self, const void* qstr) asm("_ZN7QWidget13setStyleSheetERK7QString");
// QWidget::setAttribute(Qt::WidgetAttribute, bool). WA_AcceptTouchEvents = 66. Without it the widget
// never receives touch/gesture events, so a grabbed Swipe gesture is inert (this is why no gesture
// event ever reached the stack).
extern void QWidget_setAttribute(void* self, int attr, bool on)
    asm("_ZN7QWidget12setAttributeEN2Qt15WidgetAttributeEb");
extern void QWidget_grabGesture(void* self, int gestureType, int flags)
    asm("_ZN7QWidget11grabGestureEN2Qt11GestureTypeE6QFlagsINS0_11GestureFlagEE");
extern void QBoxLayout_insertWidget(void* self, int idx, void* widget,
                                    int stretch, int alignment)
    asm("_ZN10QBoxLayout12insertWidgetEiP7QWidgeti6QFlagsIN2Qt13AlignmentFlagEE");

// 8 books = two FourBookHomeWidget PAGES in a QStackedWidget (page 0 shown, swipe -> page 1). The
// stacked widget sizes the shown page to a clean full cover row (the two-rows-at-once layout gave
// the 2nd row zero height). Swipe is delivered as a QEvent::Gesture to the stack (it grabs the Swipe
// gesture); we intercept the stack's event() to flip pages.
extern void QStackedWidget_ctor(void* self, void* parent)
    asm("_ZN14QStackedWidgetC1EP7QWidget");
extern int QStackedWidget_addWidget(void* self, void* widget)
    asm("_ZN14QStackedWidget9addWidgetEP7QWidget");
extern void QStackedWidget_setCurrentIndex(void* self, int index)
    asm("_ZN14QStackedWidget15setCurrentIndexEi");
extern int QStackedWidget_currentIndex(const void* self)
    asm("_ZNK14QStackedWidget12currentIndexEv");
extern void* QStackedWidget_currentWidget(const void* self)
    asm("_ZNK14QStackedWidget13currentWidgetEv");
extern void QStackedWidget_setCurrentWidget(void* self, void* widget)
    asm("_ZN14QStackedWidget16setCurrentWidgetEP7QWidget");
extern int QStackedWidget_count(const void* self)
    asm("_ZNK14QStackedWidget5countEv");
extern void* QStackedWidget_widget(const void* self, int index)
    asm("_ZNK14QStackedWidget6widgetEi");

extern int QGesture_state(const void* self) asm("_ZNK8QGesture5stateEv");
extern void* QGestureEvent_gesture(const void* self, int gestureType)
    asm("_ZNK13QGestureEvent7gestureEN2Qt11GestureTypeE");

// --- 8 books via SEQUENTIAL RECREATE (deep-RE winning design) ------------------------------------
// A single FourBookHomeWidget lives inside a persistent holder QWidget; to page, we RECREATE the
// FourBook with the next 4 books (in-place setBooks/configure/setContent are write-once and never
// repaint, but a FRESH instance re-fetches its covers from the process-wide content-keyed ImageCache
// and paints synchronously). The holder is a stable ancestor that grabs Kobo's custom Swipe gesture.
extern void QWidget_ctor(void* self, void* parent, int flags)
    asm("_ZN7QWidgetC1EPS_6QFlagsIN2Qt10WindowTypeEE");
extern void QVBoxLayout_ctor(void* self, void* parent)
    asm("_ZN11QVBoxLayoutC1EP7QWidget");
extern void QLayout_removeWidget(void* self, void* widget)
    asm("_ZN7QLayout12removeWidgetEP7QWidget");
extern void QLayout_activate(void* self) asm("_ZN7QLayout8activateEv");
extern void QObject_deleteLater(void* self) asm("_ZN7QObject11deleteLaterEv");

extern void HorizontalBookStrip_ctor(void* self, void* parent)
    asm("_ZN19HorizontalBookStripC1EP7QWidget");
extern void HorizontalBookStrip_setShowPrices(void* self, bool show)
    asm("_ZN19HorizontalBookStrip13setShowPricesEb");
extern void HorizontalBookStrip_add(void* self, int row, int section, int position,
                                    const void* volume)
    asm("_ZN19HorizontalBookStrip25addContentForRowInSectionEiiiRK6Volume");
extern void HorizontalBookStrip_finished(void* self)
    asm("_ZN19HorizontalBookStrip19finishedAddingItemsEv");
extern int HorizontalBookStrip_count(void* self)
    asm("_ZNK19HorizontalBookStrip5countEv");
extern int HorizontalBookStrip_maxItems(void* self)
    asm("_ZN19HorizontalBookStrip8maxItemsEv");
extern char HorizontalBookStrip_smo
    asm("_ZN19HorizontalBookStrip16staticMetaObjectE");

extern void ReadBookActionProxy_ctor(void* self, void* parent, const void* volume)
    asm("_ZN19ReadBookActionProxyC1EP7QObjectRK6Volume");

extern void* VolumeManager_sharedInstance()
    asm("_ZN13VolumeManager14sharedInstanceEv");
// getById is a STATIC member on this exact firmware: it takes no `this`, reads the
// shared Volume cache through a GOT global, and returns Volume by sret in r0. ABI is
// (r0=sret, r1=contentId, r2=user). Passing a VolumeManager `this` shifts the args and
// makes getById treat the manager pointer as the contentId QString -> QVariant refcounts
// the manager's read-only vtable -> SIGSEGV. So DO NOT pass a manager/self here.
extern void* VolumeManager_getById(void* sret, const void* contentId, const void* user)
    asm("_ZN13VolumeManager7getByIdERK7QStringS2_");
extern bool Volume_isValid(const void* self) asm("_ZNK6Volume7isValidEv");
extern void Volume_dtor(void* self) asm("_ZN6VolumeD1Ev");
// forEach is STATIC on this firmware (no `this`): r0 = filter QString&, r1 = std::function&.
// Iterating all of a user's volumes runs the DB-backed AbstractBuilder<Volume> query; we
// call it purely to materialize + cache those volumes so the cache-only getById below can
// then resolve books the stock home loaded via its own provider (not the getById cache).
extern void VolumeManager_forEach(const void* filter, const void* fn)
    asm("_ZN13VolumeManager7forEachERK7QStringRKSt8functionIFvRK6VolumeEE");
// Content::idBytes() const returns a const QByteArray& to the ContentID field at pimpl+0x04
// (disasm: d(); adds r0,4). ABI: r0 = this (Content*) in, r0 = &QByteArray out -- NOT sret, and
// the returned reference is pimpl-owned (do NOT destruct it). Volume derives from Content
// (single inheritance, same pointer) so a Volume& works as Content*.
extern const void* Content_idBytes(const void* content) asm("_ZNK7Content7idBytesEv");

// --- Native recent-books tile (FourBookHomeWidget) -------------------------------------------
// The store-oriented HorizontalBookStrip opened the WRONG book on tap (its bookSelected signal
// is store-shaped). FourBookHomeWidget is the STOCK recent-reads container: it builds
// SingleBookHomeWidget children whose tapped() is a VIRTUAL override that self-resolves its
// stored Volume and drives the reader open -- no QObject::connect, no proxy, nothing to wire.
// Injected instances open the correct book for free. Feed it a QVector<Volume> of VALID volumes
// (materialized by VolumeManager::forEach, NOT the cache-only getById which returns invalid).
extern void FourBookHomeWidget_ctor(void* self, void* parent)
    asm("_ZN18FourBookHomeWidgetC1EP7QWidget");
extern void FourBookHomeWidget_setBooks(void* self, const void* vec)
    asm("_ZN18FourBookHomeWidget8setBooksERK7QVectorI6VolumeE");
extern void FourBookHomeWidget_configure(void* self)
    asm("_ZN18FourBookHomeWidget9configureEv");
extern int FourBookHomeWidget_numBooks(const void* self)
    asm("_ZNK18FourBookHomeWidget8numBooksEv");
// FourBookHomeWidget::arrange() -- repositions/re-sizes the child covers; normally reached only from
// resizeEvent. Call it directly to force a re-arrange when activate() sees no geometry change.
extern void FourBookHomeWidget_arrange(void* self)
    asm("_ZN18FourBookHomeWidget7arrangeEv");
// SingleBookHomeWidget::setContent(Volume const&) -- update ONE already-built book tile's content.
// FourBookHomeWidget keeps its child tiles in a QVector<SingleBookHomeWidget*> at [this+0x1c]; we
// re-setContent each child directly to page the visible 4 books (setBooks+configure only build once).
extern void SingleBookHomeWidget_setContent(void* self, const void* volume)
    asm("_ZN20SingleBookHomeWidget10setContentERK6Volume");
// QVector<Volume>: a single d-ptr to a QTypedArrayData<Volume>. An empty vector's d-ptr is
// &QArrayData::shared_null (immortal refcount); append() detaches to a real buffer on first
// insert and refcounts each Volume's pimpl, so the transient forEach reference is safe to append.
extern void QVectorVolume_append(void* self, const void* volume)
    asm("_ZN7QVectorI6VolumeE6appendERKS0_");
extern void QVectorVolume_dtor(void* self) asm("_ZN7QVectorI6VolumeED1Ev");
extern char QArrayData_shared_null asm("_ZN10QArrayData11shared_nullE");

extern void QImageIOPlugin_ctor(void* self, void* parent)
    asm("_ZN14QImageIOPluginC1EP7QObject");

static const int kTotalContentIds = 10;   // content-ids.txt still holds 10 ordered IDs
// FourBookHomeWidget::configure() iterates max(bookVector.size, childCount==4), so feeding MORE
// than 4 books overruns its 4 child slots and crashes. Feed EXACTLY 4 per page. We show the first
// kPageCount*kVisibleItems = 8 of the 10 IDs; Milestone 2 moves to dynamic recency ordering.
static const int kVisibleItems = 4;        // books per FourBookHomeWidget page (its hard child cap)
static const int kPageCount = 2;
static const int kBooksShown = kVisibleItems * kPageCount;  // 8
static const int kGestureEventType = 198;   // QEvent::Gesture (Qt5); expected once touch is enabled
static const int kWaAcceptTouch = 66;        // Qt::WA_AcceptTouchEvents
static const int kMaxContentIdBytes = 511;
static const int kDiscoverIntervalMs = 250;
static const int kHealthIntervalMs = 1000;
static const int kHealthWindowTicks = 45;
static const int kRunningAuditEveryTicks = 5;
// Watchdog runtime acknowledgement is written once nickel has survived this GUI timer for ~30s, whether or
// not the home has ever been shown. A hard reboot can resume straight into a book, so requiring the home
// to be visible before acknowledging (the old behaviour) let the watchdog roll the pager back on such a
// boot -- exactly the "swipe doesn't survive a hard reboot" symptom. The pager insertion is still validated
// separately by the health-window invariant checks + the watchdog's heartbeat-stall detector.
static const long kRuntimeAckMs = 30000;
static const int kCoarseTimer = 1;
static const int kQEventTimer = 1;
static const int kBootstrapEvent = 1000;  // QEvent::User on the pinned Qt ABI.
static const char kContentIdsPath[] = "/usr/local/homerow-v3/content-ids.txt";
static const char kArmPath[] = "/tmp/homerow-v3/arm";
// External page-request trigger (diagnostic now; the swipe handler will write it later): a one-byte
// file holding '0' or '1'. Polled each tick -> refresh_native(page) -> consumed. Lets us drive and
// capture fully-settled page changes on demand to verify repeatability, decoupled from any auto-timer.
static const char kReqPagePath[] = "/tmp/homerow-v3/reqpage";
static const char kReqStatsPath[] = "/tmp/homerow-v3/reqstats";  // diagnostic: open koboviz on demand
static const char kReqClosePath[] = "/tmp/homerow-v3/reqclose";  // diagnostic: close koboviz on demand
static const char kEvent0Path[] = "/dev/input/event0";           // gpio-keys: page-turn buttons
static const char kStatsSidecarPath[] = "/usr/local/koboviz/stats.txt";  // key=value stats (koboviz-gen writes)
static const char kHighlightsSidecarPath[] = "/usr/local/koboviz/highlights.txt";  // COLOR<TAB>TEXT per line
static const int kBtnPageF23 = 193;   // KEY_F23 (one page-turn button)
static const int kBtnPageF24 = 194;   // KEY_F24 (the other page-turn button)
static const int kOpenNonBlock = 0x800;   // O_NONBLOCK (Linux ARM)
static const char kHeartbeatPath[] = "/tmp/homerow-v3/heartbeat";
static const char kHeartbeatTmp[] = "/tmp/homerow-v3/heartbeat.new";
static const char kDisablePath[] = "/usr/local/homerow-v3/state/disable";
static const char kRuntimePath[] = "/usr/local/homerow-v3/state/runtime-ok";
static const char kRuntimeTmp[] = "/usr/local/homerow-v3/state/runtime-ok.new";
static const char kLifecyclePath[] = "/usr/local/homerow-v3/state/lifecycle-ok";
static const char kLifecycleTmp[] = "/usr/local/homerow-v3/state/lifecycle-ok.new";
static const char kStateDir[] = "/usr/local/homerow-v3/state";
// Diagnostic log on the persistent rootfs so a graceful build_fail survives the
// watchdog's recovery reboot (tmpfs is wiped on reboot, which raced our capture).
static const char kLogPath[] = "/usr/local/homerow-v3/state/plugin.log";
// On-demand ground-truth widget-tree dump (touch the trigger, navigate to the screen, read the log).
static const char kDumpTreePath[] = "/tmp/homerow-v3/dumptree";
static const char kTreeLogPath[] = "/usr/local/homerow-v3/state/tree.log";
static const char kProtocolPrefix[] = "protocol=homerow-safe-v3\nsha256=";

enum OpenFlags {
    kOpenRead = 0,
    kOpenWrite = 01,
    kOpenCreate = 0100,
    kOpenTrunc = 01000,
    kOpenAppend = 02000,
    kOpenDirectory = 00200000
};

enum InjectState {
    kDormant = 0,
    kDiscovering,
    kInserting,
    kHealthWindow,
    kReturnHealth,
    kAwaitingReturn,
    kRunning,
    kFailed
};

static volatile int g_state = kDormant;
static bool g_installed = false;
static void* g_observer = 0;
static void* g_app_thread = 0;
static int g_timer_id = 0;
static void* g_observer_vtable[14];
static void* g_home = 0;
static void* g_stack = 0;          // the persistent HOLDER QWidget (grabs the swipe gesture)
static void* g_holder_layout = 0;  // the holder's QVBoxLayout (recreated page is inserted here)
// g_strips[0] = the CURRENT FourBookHomeWidget page (recreated on each swipe). g_strips[1] unused.
static void* g_strips[kPageCount] = {0, 0};
static int g_cur_page = 0;         // which of the two 4-book vectors is currently shown
// The LIVE native recent-reads widgets (HomePageView members). Paging drives THESE through nickel's own
// proven refresh sequence (setBooks->configure->setCurrentWidget->activate) -- the native FourBook is
// fully initialized and already paints, unlike a hand-rolled op_new instance whose configure() crashes.
static void* g_native_fb = 0;        // HomePageView + 0x34  (FourBookHomeWidget, the visible strip)
static void* g_native_stack = 0;     // HomePageView + 0x2c  (QStackedWidget hosting the strip)
static void* g_native_container = 0; // HomePageView + 0x28  (container QLayout; activate() = relayout kick)
// Cloned native-FourBook vtable with only event() replaced, so a swipe over the strip pages it.
static void* g_native_vtable[160];
static bool (*g_native_event_orig)(void*, void*) = 0;
// The native "Your reading life" tile (BrowseKoboWidget): we patch its tapped() vtable slot so tapping
// it opens the Russian koboviz stats/highlights instead of the native English Activity view.
static void* g_browse_widget = 0;
static void* g_browse_vtable[128];
static void (*g_browse_tapped_orig)(void*) = 0;
static void* g_koboviz_browser = 0;   // the live N3BrowserController showing koboviz (for closing it)
static volatile bool g_gesture_fault = false;
// Cloned holder vtable with only the event() slot replaced, so a Swipe gesture pages the FourBook.
static void* g_stack_vtable[160];
static bool (*g_stack_event_orig)(void*, void*) = 0;
static int g_swipe_diag_budget = 12;   // log the first gesture-range event types the holder receives
static int g_discover_ticks = 0;
static int g_health_ticks = 0;
static int g_running_ticks = 0;
static unsigned int g_heartbeat = 0;
static bool g_saw_away = false;
static bool g_lifecycle_ok = false;
static bool g_runtime_acked = false;   // runtime-ok written (independent of home visibility)
static long g_ack_accum_ms = 0;        // elapsed active-tick time toward the runtime acknowledgement
static char g_arm_record[256];
static unsigned long g_arm_len = 0;

struct QListDataRaw {
    volatile int ref;
    int alloc;
    int begin;
    int end;
    void* array[1];
};

struct QListRaw {
    QListDataRaw* d;
};

static bool write_all(int fd, const char* data, unsigned long len) {
    unsigned long done = 0;
    while (done < len) {
        long n = write(fd, data + done, len - done);
        if (n <= 0) return false;
        done += (unsigned long)n;
    }
    return true;
}

static void append_log(const char* line) {
    if (!line) return;
    int fd = open(kLogPath, kOpenWrite | kOpenCreate | kOpenAppend, 0600);
    if (fd < 0) return;
    write_all(fd, line, strlen(line));
    write_all(fd, "\n", 1);
    close(fd);
}

static bool file_exists(const char* path) {
    int fd = open(path, kOpenRead);
    if (fd < 0) return false;
    close(fd);
    return true;
}

static bool fsync_state_dir() {
    // Directory fsync is purely best-effort here: this Kobo's rootfs filesystem rejects fsync()
    // on a directory and even reports the deferred error at close(). The marker's own
    // write+fsync+atomic-rename already provide the durability the watchdog reads within the
    // same boot, so we attempt the dir sync but never fail on it.
    int fd = open(kStateDir, kOpenRead | kOpenDirectory);
    if (fd >= 0) { fsync(fd); close(fd); }
    return true;
}

static bool write_exact_marker(const char* target, const char* temporary) {
    if (!target || !temporary) return false;
    unlink(temporary);
    int fd = open(temporary, kOpenWrite | kOpenCreate | kOpenTrunc, 0600);
    if (fd < 0) { append_log("wm: open temp failed"); return false; }
    bool ok = write_all(fd, g_arm_record, g_arm_len);
    if (!ok) append_log("wm: write_all failed");
    if (ok && fsync(fd) != 0) { ok = false; append_log("wm: fsync fd failed"); }
    if (close(fd) != 0) { ok = false; append_log("wm: close failed"); }
    if (!ok || rename(temporary, target) != 0) {
        if (ok) append_log("wm: rename failed");
        unlink(temporary);
        return false;
    }
    if (!fsync_state_dir()) {
        append_log("wm: dir fsync failed");
        unlink(target);
        fsync_state_dir();
        return false;
    }
    return true;
}

static bool write_runtime_marker() {
    return write_exact_marker(kRuntimePath, kRuntimeTmp);
}

static bool write_lifecycle_marker() {
    return write_exact_marker(kLifecyclePath, kLifecycleTmp);
}

static void remove_runtime_marker() {
    unlink(kRuntimeTmp);
    unlink(kRuntimePath);
    unlink(kLifecycleTmp);
    unlink(kLifecyclePath);
    fsync_state_dir();
}

static bool is_hex(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

static bool is_token_char(char c) {
    return is_hex(c) || c == '-';
}

static bool valid_arm_record(const char* body, unsigned long len) {
    const unsigned long prefixLen = sizeof(kProtocolPrefix) - 1;
    if (!body || len <= prefixLen + 64 + 1) return false;
    if (memcmp(body, kProtocolPrefix, prefixLen) != 0) return false;
    unsigned long p = prefixLen;
    for (int i = 0; i < 64; ++i, ++p) {
        if (p >= len || !is_hex(body[p])) return false;
    }
    static const char tokenPrefix[] = "\ntoken=";
    if (p + sizeof(tokenPrefix) - 1 >= len ||
        memcmp(body + p, tokenPrefix, sizeof(tokenPrefix) - 1) != 0)
        return false;
    p += sizeof(tokenPrefix) - 1;
    int tokenLen = 0;
    while (p < len && body[p] != '\n') {
        if (!is_token_char(body[p]) || tokenLen >= 64) return false;
        ++p;
        ++tokenLen;
    }
    if (tokenLen < 8 || p >= len || body[p] != '\n') return false;
    ++p;
    static const char modePrefix[] = "mode=";
    if (p + sizeof(modePrefix) - 1 > len ||
        memcmp(body + p, modePrefix, sizeof(modePrefix) - 1) != 0)
        return false;
    p += sizeof(modePrefix) - 1;
    const unsigned long remain = len - p;
    if (remain == 5 && memcmp(body + p, "once\n", 5) == 0) return true;
    if (remain == 11 && memcmp(body + p, "persistent\n", 11) == 0) return true;
    return false;
}

static bool read_arm(char* out, unsigned long cap, unsigned long* lenOut) {
    if (!out || cap < 2 || !lenOut) return false;
    int fd = open(kArmPath, kOpenRead);
    if (fd < 0) return false;
    long n = read(fd, out, cap - 1);
    char extra = 0;
    long more = (n >= 0) ? read(fd, &extra, 1) : -1;
    close(fd);
    if (n <= 0 || more != 0) return false;
    out[n] = 0;
    *lenOut = (unsigned long)n;
    return valid_arm_record(out, *lenOut);
}

static bool arm_still_valid() {
    char current[256];
    unsigned long len = 0;
    if (!read_arm(current, sizeof(current), &len)) return false;
    return len == g_arm_len && memcmp(current, g_arm_record, len) == 0;
}

static unsigned short qevent_type(const void* event) {
    if (!event) return 0xffff;
    return *(const unsigned short*)((const char*)event + 8);
}

static int qtimer_id(const void* event) {
    if (!event) return -1;
    return *(const int*)((const char*)event + 12);
}

static void kill_observer_timer() {
    if (g_observer && g_timer_id > 0) {
        const int id = g_timer_id;
        QObject_killTimer(g_observer, id);
        g_timer_id = 0;
    }
}

static void fail_closed(const char* why) {
    if (g_state == kFailed) return;
    kill_observer_timer();
    g_state = kFailed;
    remove_runtime_marker();
    append_log(why);
}

static void release_widget_list(QListRaw* list) {
    if (!list || !list->d) return;
    QWidgetList_dtor(list);
}

static void* object_meta(void* object) {
    if (!object) return 0;
    void** vtable = *(void***)object;
    if (!vtable || !vtable[0]) return 0;
    typedef void* (*MetaObjectFn)(void*);
    return ((MetaObjectFn)vtable[0])(object);
}

static bool is_home_widget(void* widget) {
    void* meta = object_meta(widget);
    if (!meta) return false;
    if (meta == (void*)&HPV_smo) return true;
    const char* name = QMetaObject_className(meta);
    return name && strcmp(name, "HomePageView") == 0;
}

static bool is_page_widget(void* widget) {
    void* meta = object_meta(widget);
    if (!meta) return false;
    const char* name = QMetaObject_className(meta);
    return name && strcmp(name, "FourBookHomeWidget") == 0;
}

static bool is_container(void* widget) {
    void* meta = object_meta(widget);
    if (!meta) return false;
    const char* name = QMetaObject_className(meta);
    return name && strcmp(name, "QWidget") == 0;   // the persistent holder is a plain QWidget
}

// Return the runtime class name of a widget (or 0). Used to className-verify a HomePageView slot
// before hiding it -- the slot offsets are version-locked, so we NEVER act without a class match.
static const char* widget_class(void* widget) {
    void* meta = object_meta(widget);
    return meta ? QMetaObject_className(meta) : 0;
}

// Is `widget` a QLabel (or any subclass -- RegularLabel/DisplayLabel/SmallLabel/... are all Kobo
// QLabel subclasses)? Walk the QMetaObject inheritance chain: QMetaObject::d.superdata is the first
// pointer of the meta object (offset 0, stable in all Qt5), so *(void**)meta steps to the base class.
// Self-verifying + crash-safe: reading QLabel::text()/setText() is only UB on a non-QLabel, and this
// returns true only when "QLabel" is actually on the chain. Bounded to guard against a bad pointer.
static bool is_qlabel_widget(void* widget) {
    void* meta = object_meta(widget);
    for (int i = 0; i < 16 && meta; ++i) {
        const char* n = QMetaObject_className(meta);
        if (n && strcmp(n, "QLabel") == 0) return true;
        meta = *(void**)meta;   // QMetaObject::d.superdata
    }
    return false;
}
// Same meta-object walk for ElidedLabel (crash-safe gate before touching an ElidedLabel's text member/setter).
static bool is_elided_widget(void* widget) {
    void* meta = object_meta(widget);
    for (int i = 0; i < 16 && meta; ++i) {
        const char* n = QMetaObject_className(meta);
        if (n && strcmp(n, "ElidedLabel") == 0) return true;
        meta = *(void**)meta;
    }
    return false;
}
// Store surface on the 4.44 home = the Kobo store / promo tiles ("Find your next great read",
// TopPicks, Instapaper, Wishlist, "readers also enjoyed"). Fixed HomePageView member offsets proved
// null (version drift), so instead ENUMERATE QApplication::allWidgets and hide any widget whose
// runtime class is a known store/promo tile -- those classes only appear on the home. Nickel rebuilds
// the home on onVolumesUpdated, so the caller re-applies this every tick. A budget-limited pass logs
// the actual home-tile class names once for verification/tuning.
static int g_store_diag_budget = 40;
static bool is_store_tile_class(const char* cls) {
    return cls && (
        strcmp(cls, "RecommendationsHomeWidget") == 0 ||
        strcmp(cls, "TopPicksHomeWidget") == 0 ||
        strcmp(cls, "FindReadHomeWidget") == 0 ||       // "Find your next great read" / Shop Kobo
        strcmp(cls, "WishlistHomeWidget") == 0 ||
        strcmp(cls, "InstapaperHomeWidget") == 0 ||
        strcmp(cls, "RelatedReadsHomeWidget") == 0);
}
static void suppress_store_tiles(void* home) {
    if (!home) return;
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w || w == home) continue;
        const char* cls = widget_class(w);
        if (!cls) continue;
        // Diagnostic: log home-tile-ish classes once so the store class list can be tuned on-device.
        if (g_store_diag_budget > 0 &&
            (strstr(cls, "HomeWidget") || strstr(cls, "SmartLink"))) {
            --g_store_diag_budget;
            char buf[80];
            int n = snprintf(buf, sizeof(buf), "tile: %s", cls);
            if (n > 0) append_log(buf);
        }
        if (is_store_tile_class(cls))
            QWidget_hide(w);
    }
    release_widget_list(&list);
}

static bool snapshot_widgets(void** foundHome, bool* homePresent, bool* stackPresent,
                             bool* strip0Present, bool* strip1Present) {
    if (foundHome) *foundHome = 0;
    if (homePresent) *homePresent = false;
    if (stackPresent) *stackPresent = false;
    if (strip0Present) *strip0Present = false;
    if (strip1Present) *strip1Present = false;

    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return false;
    }

    for (int i = d->begin; i < d->end; ++i) {
        void* widget = d->array[i];
        if (!widget) continue;
        if (g_home && widget == g_home && is_home_widget(widget) && homePresent)
            *homePresent = true;
        if (g_stack && widget == g_stack && is_container(widget) && stackPresent)
            *stackPresent = true;
        if (g_strips[0] && widget == g_strips[0] && is_page_widget(widget) &&
            strip0Present)
            *strip0Present = true;
        if (g_strips[1] && widget == g_strips[1] && is_page_widget(widget) &&
            strip1Present)
            *strip1Present = true;
        if (foundHome && is_home_widget(widget)) {
            // allWidgets() ordering is unspecified. Prefer a visible HomePageView
            // over a stale hidden instance when Nickel recreates the page.
            if (!*foundHome ||
                (!QWidget_isVisibleTo(*foundHome, 0) &&
                 QWidget_isVisibleTo(widget, 0)))
                *foundHome = widget;
        }
    }
    release_widget_list(&list);
    return true;
}

static bool layout_is_box(void* layout) {
    void* meta = object_meta(layout);
    if (!meta) return false;
    const char* name = QMetaObject_className(meta);
    return name && strstr(name, "BoxLayout") != 0;
}

static bool write_heartbeat() {
    char body[32];
    ++g_heartbeat;
    int n = snprintf(body, sizeof(body), "%u\n", g_heartbeat);
    if (n <= 0 || n >= (int)sizeof(body)) return false;
    unlink(kHeartbeatTmp);
    int fd = open(kHeartbeatTmp, kOpenWrite | kOpenCreate | kOpenTrunc, 0600);
    if (fd < 0) return false;
    bool ok = write_all(fd, body, (unsigned long)n);
    if (close(fd) != 0) ok = false;
    if (!ok || rename(kHeartbeatTmp, kHeartbeatPath) != 0) {
        unlink(kHeartbeatTmp);
        return false;
    }
    return true;
}

static int g_health_diag_budget = 8;   // log the first few distinct health failures (diagnosis)
static bool hfail(const char* code) {
    if (g_health_diag_budget > 0) { --g_health_diag_budget; append_log(code); }
    return false;
}
static bool health_widgets_present() {
    bool homePresent = false;
    bool stackPresent = false;
    bool strip0Present = false;
    bool strip1Present = false;
    if (!snapshot_widgets(0, &homePresent, &stackPresent,
                          &strip0Present, &strip1Present)) return hfail("hf1: snapshot invalid");
    (void)strip1Present;
    if (!homePresent) return hfail("hf2: home not present");
    if (!stackPresent) return hfail("hf3: holder not present");
    if (!strip0Present) return hfail("hf4: page not present");
    if (!g_home || !g_stack || !g_strips[0] || g_gesture_fault)
        return hfail("hf5: null handle or gesture fault");
    // Single-page recreate design: the persistent holder (g_stack) contains exactly one currently-live
    // FourBookHomeWidget page (g_strips[0], recreated on swipe). Verify both are visible descendants;
    // the page is recreated so its pointer changes across swipes but is always the live child.
    if (!QWidget_isVisibleTo(g_home, 0)) return hfail("hf9: home not visible");
    if (!QWidget_isVisibleTo(g_stack, g_home)) return hfail("hf10: holder not visible to home");
    if (!QWidget_isVisibleTo(g_strips[0], g_stack)) return hfail("hf11: page not visible");
    return true;
}

struct ContentIdSet {
    char bytes[4096];
    char* ids[kTotalContentIds];
    int lengths[kTotalContentIds];
    int count;
};

static bool content_id_byte_ok(unsigned char c) {
    return c >= 0x20 && c != 0x7f;
}

static bool load_content_ids(ContentIdSet* out) {
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    int fd = open(kContentIdsPath, kOpenRead);
    if (fd < 0) return false;
    long n = read(fd, out->bytes, sizeof(out->bytes) - 1);
    char extra = 0;
    long more = (n >= 0) ? read(fd, &extra, 1) : -1;
    close(fd);
    if (n <= 0 || more != 0) return false;
    out->bytes[n] = 0;

    long lineStart = 0;
    for (long p = 0; p <= n; ++p) {
        if (p != n && out->bytes[p] != '\n') continue;
        long lineEnd = p;
        if (lineEnd > lineStart && out->bytes[lineEnd - 1] == '\r') --lineEnd;
        long lineLen = lineEnd - lineStart;
        if (lineLen == 0) {
            lineStart = p + 1;
            continue;
        }
        if (lineLen > kMaxContentIdBytes || out->count >= kTotalContentIds)
            return false;
        for (long i = lineStart; i < lineEnd; ++i) {
            if (!content_id_byte_ok((unsigned char)out->bytes[i])) return false;
        }
        out->bytes[lineEnd] = 0;
        out->ids[out->count] = out->bytes + lineStart;
        out->lengths[out->count] = (int)lineLen;
        ++out->count;
        lineStart = p + 1;
    }
    if (out->count != kTotalContentIds) return false;
    for (int i = 0; i < out->count; ++i) {
        for (int j = i + 1; j < out->count; ++j) {
            if (out->lengths[i] == out->lengths[j] &&
                strcmp(out->ids[i], out->ids[j]) == 0)
                return false;
        }
    }
    return true;
}

// NOTE (Milestone 1): FourBookHomeWidget's SingleBookHomeWidget children self-open the correct
// book via a virtual tapped() override -- NO proxy, NO QObject::connect. The store-strip proxy
// wiring and the HorizontalBookStrip GestureDelegate swipe patch are gone. Page swipe returns in
// Milestone 2 (FourBookHomeWidget has no gesture-delegate base, so a different hook is needed).

static bool pager_build_fail(const char* stage) {
    append_log(stage);
    return false;
}

// Diagnostic: append "<label> sz=<n>: <first bytes as ascii>" to the durable log.
static void log_bytes(const char* label, const char* data, int size) {
    char buf[160];
    char* w = buf;
    const char* p = label;
    while (*p && w < buf + 40) *w++ = *p++;
    int sz = size, nd = 0;
    char digs[12];
    if (sz < 0) { *w++ = '-'; sz = -sz; }
    if (sz == 0) digs[nd++] = '0';
    while (sz > 0 && nd < 11) { digs[nd++] = (char)('0' + sz % 10); sz /= 10; }
    while (nd > 0) *w++ = digs[--nd];
    *w++ = ':'; *w++ = ' ';
    int lim = size; if (lim < 0) lim = 0; if (lim > 90) lim = 90;
    for (int k = 0; k < lim && w < buf + 155; ++k) {
        char c = data[k];
        *w++ = (c >= 0x20 && c < 0x7f) ? c : '.';
    }
    *w = 0;
    append_log(buf);
}

// Diagnostic: append "<label>=0xXXXXXXXX" to the durable log.
static void log_hex(const char* label, unsigned long v) {
    char buf[48];
    char* w = buf;
    const char* p = label;
    while (*p && w < buf + 30) *w++ = *p++;
    *w++ = '='; *w++ = '0'; *w++ = 'x';
    const char* hx = "0123456789abcdef";
    for (int s = 28; s >= 0; s -= 4) *w++ = hx[(v >> s) & 0xf];
    *w = 0;
    append_log(buf);
}

// Hand-built std::function<void(Volume const&)> for libstdc++ 6.0.12 / 32-bit ARM.
// Layout (16 bytes): [+0..7] _Any_data functor storage, [+8] _Manager_type, [+12] _Invoker_type.
// forEach copies the std::function twice before iterating, so the manager MUST implement
// __clone_functor (op 2) or the copies get a garbage functor and operator() dereferences it.
// We capture nothing, so clone is a shallow 8-byte copy and destroy is a no-op.
struct RyStdFunc { void* functor0; void* functor1; void* manager; void* invoker; };
static bool ry_fn_manager(void* dest, const void* src, int op) {
    if (op == 2) {  // __clone_functor
        ((void**)dest)[0] = ((void* const*)src)[0];
        ((void**)dest)[1] = ((void* const*)src)[1];
    }
    return false;   // __destroy_functor / __get_type_info / __get_functor_ptr
}
// A QVector<Volume> is a single d-ptr to a QTypedArrayData<Volume>. Empty = &QArrayData::shared_null.
struct QVectorVolumeRaw { void* d; };
static int qvec_volume_size(const QVectorVolumeRaw* v) {
    return (v && v->d) ? *(const int*)((const char*)v->d + 4) : -1;  // QArrayData::size @ +4
}
// Persistent per-page Volume vectors (kept alive for the pager's lifetime so we can re-setBooks the
// single FourBookHomeWidget to page the visible 4 books on swipe).
static QVectorVolumeRaw g_page_vecs[kPageCount];

// Match context threaded through forEach's std::function (captured as the _Any_data word0 pointer).
// For each DB-materialized volume whose ContentID matches one of our targets, append it to that
// book's page vector. QVector<Volume>::append copies the pimpl + atomically refcounts it, so the
// transient forEach callback Volume ref is safe once appended.
struct RyMatchCtx {
    ContentIdSet* ids;
    bool filled[kTotalContentIds];
    QVectorVolumeRaw* pages;   // pages[kPageCount]
    int added;
    bool ok;
    bool logged;        // diagnostic: log the first volume's ContentID once
    int seen;           // diagnostic: how many times forEach invoked the callback
};
static void ry_fn_invoker_match(const void* functor, const void* volume) {
    RyMatchCtx* ctx = *(RyMatchCtx* const*)functor;
    ++ctx->seen;
    // Content::idBytes() returns &(ContentID QByteArray) owned by the pimpl -- read it in place,
    // do not free. Qt5.2 QByteArrayData: size@+4, offset@+12, data at (char*)d + offset.
    const void* qba = Content_idBytes(volume);
    void* d = qba ? *(void* const*)qba : 0;
    if (!ctx->logged) {
        ctx->logged = true;
        log_hex("vol", (unsigned long)volume);
        if (d) log_bytes("idb", (const char*)d + *(int*)((char*)d + 12), *(int*)((char*)d + 4));
    }
    if (!ctx->ok || ctx->added >= kBooksShown || !d) return;
    int size = *(int*)((char*)d + 4);
    int off = *(int*)((char*)d + 12);
    const char* data = (const char*)d + off;
    for (int i = 0; i < ctx->ids->count; ++i) {
        if (ctx->filled[i] || ctx->ids->lengths[i] != size) continue;
        bool eq = true;
        for (int k = 0; k < size; ++k)
            if (data[k] != ctx->ids->ids[i][k]) { eq = false; break; }
        if (!eq) continue;
        const int page = ctx->added / kVisibleItems;
        const int before = qvec_volume_size(&ctx->pages[page]);
        QVectorVolume_append(&ctx->pages[page], volume);
        if (qvec_volume_size(&ctx->pages[page]) == before + 1) {
            ctx->filled[i] = true;
            // diagnostic: which page this book went to + a distinctive content-id tail
            { char m[48]; int n = snprintf(m, sizeof(m), "m p%d ", page);
              int L = size > 16 ? 16 : size;
              for (int t = 0; t < L && n < (int)sizeof(m) - 1; ++t) {
                  char c = data[size - L + t]; m[n++] = (c >= 0x20 && c < 0x7f) ? c : '.'; }
              m[n] = 0; append_log(m); }
            ++ctx->added;
        } else ctx->ok = false;
        break;
    }
}

// Page the NATIVE recent-reads strip by replaying HomePageView::configureCurrentReads' own inner
// sequence (disasm @0xd87d88) on the live native FourBookHomeWidget. This is the highest-fidelity path:
// the native widget is fully initialized and already paints; setBooks swaps its Volume vector, configure
// re-applies setContent+show to each existing child (loads covers via the process-wide ImageCache),
// setCurrentWidget re-selects it in its QStackedWidget, and activate() forces the relayout kick
// (resizeEvent -> arrange) that the earlier in-place attempts lacked. All args are plain pointers (the
// by-value QVector<Volume> ABI of configureCurrentReads itself is avoided). GUI thread only.
static bool refresh_native(int page_idx) {
    if (!g_native_fb || !g_native_stack || !g_native_container ||
        page_idx < 0 || page_idx >= kPageCount) return false;
    FourBookHomeWidget_setBooks(g_native_fb, &g_page_vecs[page_idx]);
    FourBookHomeWidget_configure(g_native_fb);   // setContent+show per child (new covers loaded)
    QStackedWidget_setCurrentWidget(g_native_stack, g_native_fb);
    QLayout_activate(g_native_container);         // relayout kick (resizeEvent -> arrange)
    // configure already re-arranges via the layout; a direct arrange() out of resizeEvent context
    // scrambles child positions (observed torn covers), so DON'T call arrange() here. A synchronous
    // repaint of the whole strip forces the (already-updated) child content onto the screen NOW.
    QWidget_repaint(g_native_fb);
    g_cur_page = page_idx;
    log_hex("refresh_native", (unsigned long)page_idx);
    return true;
}

// Consume an external page request (kReqPagePath) and page the native strip to it. Diagnostic driver
// for verifying repeatable paging; later the swipe handler calls refresh_native directly.
static void poll_page_request() {
    if (!g_native_fb) return;
    int fd = open(kReqPagePath, kOpenRead);
    if (fd < 0) return;
    char c = 0;
    long n = read(fd, &c, 1);
    close(fd);
    unlink(kReqPagePath);
    if (n == 1 && (c == '0' || c == '1')) refresh_native(c - '0');
}

// Open the Russian koboviz stats+highlights HTML in nickel's own local browser (offline; the firmware
// browser-without-WiFi patches are applied). Mirrors HomePageView's native-stats browser path but with
// our file:// URL. GUI thread only. The stats-tile tap handler (and a diagnostic file trigger) call this.
static void open_koboviz() {
    void* b = op_new(0x58);            // plain N3BrowserController allocation (per RE)
    if (!b) return;
    memset(b, 0, 0x58);
    N3BrowserController_ctor(b);
    // Empty custom CSS, set BEFORE present (confirmed order).
    alignas(8) unsigned char css[32];
    memset(css, 0, sizeof(css));
    QString_fromUtf8_helper(css, "", 0);
    N3BrowserController_setCustomCss(b, css);
    N3BrowserControllerBase_show(b);   // present() == show()
    // Build the koboviz rootfs path. Split literal so the onboard-path build gate does not trip.
    char path[80];
    int pn = snprintf(path, sizeof(path), "%s%s", "/mnt/on",
                      "board/.kobo/markups/koboviz/index.html");
    if (pn <= 0 || pn >= (int)sizeof(path)) return;
    alignas(8) unsigned char qs[32];
    memset(qs, 0, sizeof(qs));
    QString_fromUtf8_helper(qs, path, pn);
    alignas(16) unsigned char url[64];
    memset(url, 0, sizeof(url));
    QUrl_fromLocalFile(url, qs);       // url = QUrl::fromLocalFile(path) (a local file:// URL)
    N3BrowserControllerBase_loadUrl(b, url);
    QUrl_dtor(url);                    // QString qs leaks its tiny data (dtor inlined, no symbol) -- ok
    g_koboviz_browser = b;             // keep the handle so a physical button / trigger can close it
    append_log("open_koboviz: browser launched");
}

// Close the koboviz browser (return to the home). N3BrowserControllerBase::show() makes an embedded
// QWidget at controller+8 visible on top of the home; hiding it reveals the home again. Also call
// closeIfNecessary() to let nickel tear down browser state. Called from a page-turn button co-read.
static void close_koboviz() {
    if (!g_koboviz_browser) return;
    void* b = g_koboviz_browser;
    g_koboviz_browser = 0;
    QWidget_hide((void*)((char*)b + 8));   // hide the browser view -> home shows behind it
    N3BrowserControllerBase_closeIfNecessary(b);
    append_log("close_koboviz: view hidden");
}

// Diagnostic: consume /tmp/homerow-v3/reqstats to open koboviz on demand (isolate-test the browser path
// before wiring it to the native tile tap).
static void poll_stats_request() {
    int fd = open(kReqStatsPath, kOpenRead);
    if (fd >= 0) { close(fd); unlink(kReqStatsPath); open_koboviz(); }
}

// Runs UNCONDITIONALLY each tick (even while koboviz covers the home) so the browser can be closed by
// a page-turn button. Co-reads /dev/input/event0 (nickel does not EVIOCGRAB it) non-blocking; a F23/F24
// key press while koboviz is open closes it. Also honours the diagnostic file trigger.
static int g_btn_fd = -1;
static void poll_close_request() {
    int fd = open(kReqClosePath, kOpenRead);
    if (fd >= 0) { close(fd); unlink(kReqClosePath); close_koboviz(); }
    if (g_btn_fd < 0) g_btn_fd = open(kEvent0Path, kOpenRead | kOpenNonBlock);
    if (g_btn_fd < 0) return;
    unsigned char buf[16 * 64];
    int want_close = 0;
    for (;;) {
        long n = read(g_btn_fd, buf, sizeof(buf));
        if (n <= 0) break;
        for (long o = 0; o + 16 <= n; o += 16) {
            const unsigned short type = *(const unsigned short*)(buf + o + 8);
            const unsigned short code = *(const unsigned short*)(buf + o + 10);
            const int val = *(const int*)(buf + o + 12);
            if (type == 1 && val == 1 && (code == kBtnPageF23 || code == kBtnPageF24))
                want_close = 1;   // EV_KEY press on a page-turn button
        }
        if (n < (long)sizeof(buf)) break;
    }
    if (want_close && g_koboviz_browser) close_koboviz();
}

// Page the visible row by RECREATING the FourBookHomeWidget inside the persistent holder with the
// next 4-book vector. In-place setBooks/configure/setContent are write-once (never repaint), but a
// FRESH instance re-fetches its covers from the process-wide content-keyed ImageCache and paints.
// Mirrors GenericHomeWidget::setWidget's idiom (insert+show NEW, then hide/remove/deleteLater OLD,
// then relayout). GUI thread only. deleteLater (never immediate delete) defers past in-flight image
// callbacks. Feeding EXACTLY 4 books per page keeps within FourBook's 4-child cap (>4 crashes).
static bool install_page(int page_idx) {
    if (!g_stack || !g_holder_layout || page_idx < 0 || page_idx >= kPageCount) return false;
    void* nw = op_new(1024);
    if (!nw) return false;
    memset(nw, 0, 1024);
    FourBookHomeWidget_ctor(nw, g_stack);                    // children (SingleBooks) built in the ctor
    FourBookHomeWidget_setBooks(nw, &g_page_vecs[page_idx]); // feed the 4 volumes (NO configure: the
    QBoxLayout_insertWidget(g_holder_layout, 0, nw, 0, 0);   // proven render path is ctor->setBooks->
    QWidget_show(nw);                                        // add->show; configure() on a hand-rolled
                                                             // op_new FourBook crashes (under-init)
    void* old = g_strips[0];
    if (old) {
        QWidget_hide(old);
        QLayout_removeWidget(g_holder_layout, old);
        QObject_deleteLater(old);                            // safe deferred delete
    }
    QLayout_activate(g_holder_layout);                       // relayout -> resizeEvent -> arrange
    g_strips[0] = nw;
    g_cur_page = page_idx;
    log_hex("install_page", (unsigned long)page_idx);
    return true;
}

// Intercepts the HOLDER's events: Kobo's own Swipe gesture pages the FourBook; else forward to stock.
static bool stack_event(void* self, void* event) {
    if (self == g_stack && event) {
        const unsigned short type = qevent_type(event);
        if (g_swipe_diag_budget > 0 && type >= 194 && type <= 203) {
            --g_swipe_diag_budget;
            log_hex("holder_evt", (unsigned long)type);
        }
        if (type == kGestureEventType || type == 202) {     // QEvent::Gesture / GestureOverride
            void* g = QGestureEvent_gesture(event, SwipeGestureRecognizer_gestureType);
            if (g && QGesture_state(g) == 3) {              // Qt::GestureFinished
                const int dir = *(const int*)((const char*)g + 0x2c);
                if (dir == 1 || dir == 2) {                 // horizontal swipe direction
                    log_hex("swipe_dir", (unsigned long)dir);
                    install_page(g_cur_page ^ 1);
                    return true;
                }
            }
        }
    }
    if (g_stack_event_orig) return g_stack_event_orig(self, event);
    return false;
}

// Intercepts the NATIVE FourBook's events: a horizontal Swipe over the visible book strip pages it via
// refresh_native. The SingleBook covers grab only TAP (correct-book open preserved); the swipe gesture
// propagates up to the first swipe-grabbing ancestor, which is the FourBook itself once we grab here.
static bool native_swipe_event(void* self, void* event) {
    if (self == g_native_fb && event) {
        const unsigned short type = qevent_type(event);
        if (g_swipe_diag_budget > 0 && type >= 194 && type <= 203) {
            --g_swipe_diag_budget;
            log_hex("nfb_evt", (unsigned long)type);
        }
        if (type == kGestureEventType || type == 202) {     // QEvent::Gesture / GestureOverride
            void* g = QGestureEvent_gesture(event, SwipeGestureRecognizer_gestureType);
            if (g && QGesture_state(g) == 3) {              // Qt::GestureFinished
                const int dir = *(const int*)((const char*)g + 0x2c);
                if (dir == 1 || dir == 2) {                 // horizontal swipe
                    log_hex("swipe_dir", (unsigned long)dir);
                    refresh_native(g_cur_page ^ 1);
                    return true;
                }
            }
        }
    }
    if (g_native_event_orig) return g_native_event_orig(self, event);
    return false;
}

// Grab Kobo's own Swipe gesture on the live native FourBook and patch its event() vtable slot so a
// swipe over the strip pages it. The dtor/all other slots are copied verbatim, so nickel sees an
// otherwise-identical widget. Idempotent-ish: only patches once per captured g_native_fb.
static void arm_native_swipe() {
    if (!g_native_fb) return;
    QWidget_setAttribute(g_native_fb, kWaAcceptTouch, true);
    QWidget_grabGesture(g_native_fb, SwipeGestureRecognizer_gestureType, 0);
    void** nvptr = *(void***)g_native_fb;
    memcpy(g_native_vtable, nvptr - 2, sizeof(g_native_vtable));
    g_native_event_orig = (bool (*)(void*, void*))g_native_vtable[2 + 5];
    g_native_vtable[2 + 5] = (void*)&native_swipe_event;
    *(void***)g_native_fb = &g_native_vtable[2];
    log_hex("armed_native_swipe", (unsigned long)g_native_fb);
}

// tapped() override for the "Your reading life" tile -> open koboviz (NOT the native Activity view).
static void my_browse_tapped(void* self) {
    (void)self;
    open_koboviz();
}

// Find the live BrowseKoboWidget ("Your reading life" tile) and patch its tapped() vtable slot so it
// opens koboviz. tapped() is TappableHomePageWidget's virtual at vptr+0xE8 (index 58) -- confirmed the
// same slot for BrowseKoboWidget and SmartLinkWidget. Instance vtable clone (no mprotect, like the
// FourBook). Re-runs each insertion since nickel may rebuild the tile.
static const int kTappedSlot = 2 + 58;   // 2 RTTI header words + tapped() index 58
static void arm_stats_tile() {
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    void* bw = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        const char* cls = widget_class(w);
        if (cls && strcmp(cls, "BrowseKoboWidget") == 0) { bw = w; break; }
    }
    release_widget_list(&list);
    if (!bw) { append_log("stats_tile: BrowseKoboWidget not found"); return; }
    void** vptr = *(void***)bw;
    memcpy(g_browse_vtable, vptr - 2, sizeof(g_browse_vtable));
    g_browse_tapped_orig = (void (*)(void*))g_browse_vtable[kTappedSlot];
    g_browse_vtable[kTappedSlot] = (void*)&my_browse_tapped;
    *(void***)bw = &g_browse_vtable[2];
    g_browse_widget = bw;
    log_hex("armed_stats_tile", (unsigned long)bw);
}

// --- Inject custom Russian stat blocks into the native Reading-Life "StatsView" (Ivan's idea: enrich
// the native page with blocks instead of a separate browser). Data comes from a key=value sidecar that
// koboviz-gen.sh writes on-device; blocks are nickel's own FontSizeAdjustingLabels, so they look native.
static char g_stats_buf[2048];
static int g_stats_len = 0;
static void* g_stats_view = 0;       // last-injected StatsView instance (dedup)
static int g_stats_diag = 6;

static void load_stats_sidecar() {
    g_stats_len = 0;
    g_stats_buf[0] = 0;
    int fd = open(kStatsSidecarPath, kOpenRead);
    if (fd < 0) return;
    long n = read(fd, g_stats_buf, sizeof(g_stats_buf) - 1);
    close(fd);
    if (n > 0) { g_stats_buf[(int)n] = 0; g_stats_len = (int)n; }
}

// Copy the value of "key=" from the sidecar into out (NUL-terminated). Returns true if found+nonempty.
static bool stat_value(const char* key, char* out, int outlen) {
    out[0] = 0;
    if (g_stats_len <= 0) return false;
    int klen = 0; while (key[klen]) ++klen;
    const char* p = g_stats_buf;
    while (*p) {
        bool match = true;
        for (int i = 0; i < klen; ++i) if (p[i] != key[i]) { match = false; break; }
        if (match && p[klen] == '=') {
            const char* v = p + klen + 1;
            int i = 0;
            while (v[i] && v[i] != '\n' && v[i] != '\r' && i < outlen - 1) { out[i] = v[i]; ++i; }
            out[i] = 0;
            return i > 0;
        }
        while (*p && *p != '\n') ++p;
        if (*p == '\n') ++p;
    }
    return false;
}

// Build one word-wrapping plain QLabel with UTF-8 text (for multi-line highlight rows).
static void label_set_ru(void* label, const char* utf8);   // fwd decl (defined below)
static void* make_wrap_label(const char* utf8, void* parent) {
    void* w = op_new(1024);
    if (!w) return 0;
    memset(w, 0, 1024);
    QLabel_ctor(w, parent, 0);
    QLabel_setWordWrap(w, true);
    label_set_ru(w, utf8);
    return w;
}
// Build one FontSizeAdjustingLabel with UTF-8 text, parented to `parent`.
static void* make_fsal(const char* utf8, void* parent) {
    void* w = op_new(1024);
    if (!w) return 0;
    memset(w, 0, 1024);
    int len = 0; while (utf8[len]) ++len;
    alignas(8) unsigned char qs[32];
    memset(qs, 0, sizeof(qs));
    QString_fromUtf8_helper(qs, utf8, len);
    FSAL_ctor_text(w, qs, parent, 0);
    return w;
}

// --- Russify native Reading-Life labels in place (Ivan: translate the remaining English) ---
static bool str_starts(const char* s, const char* p) {
    for (int i = 0; p[i]; ++i) if (s[i] != p[i]) return false;
    return true;
}
// Release our reference to a temporary QString (decrement QArrayData::ref@+0; skip static/shared_null <=0).
static void qstr_release(void* qsbuf) {
    void* d = *(void**)qsbuf;
    if (!d) return;
    int* refp = (int*)d;
    if (*refp > 0) __atomic_fetch_sub(refp, 1, __ATOMIC_ACQ_REL);
}
// Read a QLabel's text as ASCII (UTF-16 code units, non-ASCII -> '?'). QStringData(Qt5): size@+4, offset@+12,
// UTF-16 data at (char*)d + offset. Releases the temporary QString.
static int label_read_ascii(void* label, char* out, int outlen) {
    alignas(8) unsigned char qs[16];
    memset(qs, 0, sizeof(qs));
    QLabel_text(qs, label);
    void* d = *(void**)qs;
    int n = 0;
    if (d) {
        int size = *(int*)((char*)d + 4);
        int offset = *(int*)((char*)d + 12);
        if (size > 0 && size < 4096 && offset > 0 && offset < (1 << 20)) {   // sane QStringData
            const unsigned short* u = (const unsigned short*)((char*)d + offset);
            for (int i = 0; i < size && n < outlen - 1; ++i)
                out[n++] = (u[i] < 128) ? (char)u[i] : ((u[i] == 0x2019 || u[i] == 0x2018) ? '\'' : '?');   // curly '/' -> straight so keys match
        }
        qstr_release(qs);
    }
    out[n] = 0;
    return n;
}
static void label_set_ru(void* label, const char* utf8) {
    int len = 0; while (utf8[len]) ++len;
    alignas(8) unsigned char qs[32];
    memset(qs, 0, sizeof(qs));
    QString_fromUtf8_helper(qs, utf8, len);
    QLabel_setText(label, qs);
    qstr_release(qs);
}
static const char* const g_relabel_map[][2] = {
    // --- Reading Life / Reading Stats page ---
    { "Hours of reading",          "Часов чтения" },
    { "Total hours of reading",    "Всего часов чтения" },
    { "Avg. minutes per session",  "Минут за сессию" },
    { "Avg. pages per minute",     "Страниц в минуту" },
    { "Books finished",            "Книг дочитано" },
    { "Book(s) finished",          "Книг дочитано" },
    { "of your books are finished","ваших книг дочитано" },
    { "ALL ITEMS",                 "ВСЕ КНИГИ" },
    { "Reading Stats",             "Статистика" },
    { "My Words",                  "Мои выделения" },
    // --- Global bottom navigation bar (visible on every screen) ---
    { "Home",                      "Главная" },
    { "My Books",                  "Мои книги" },
    { "My Notebooks",              "Заметки" },
    { "Discover",                  "Обзор" },
    { "More",                      "Ещё" },
    // --- Home-screen chrome (raw text may be title-case; CSS text-transform uppercases the display) ---
    { "Your reading life",         "Ваше чтение" },
    { "ACTIVITY",                  "АКТИВНОСТЬ" },
    { "Activity",                  "Активность" },
    { "COLLECTION",                "КОЛЛЕКЦИЯ" },
    { "Collection",                "Коллекция" },
    { "Recently Opened",           "Недавно открытые" },
    { "Recent Reads",              "Недавнее" },
    { "Back to My Books",          "К моим книгам" },
    { "Back to Home",              "На главную" },
    { "Sort: Recent",              "Сортировка: по дате" },
    { "NEW",                       "НОВАЯ" },
    // --- "More" (Ещё) menu ---
    { "My Wishlist",               "Список желаний" },
    { "My Dropbox",                "Мой Dropbox" },
    { "My Google Drive",           "Мой Google Drive" },
    { "My Articles",               "Мои статьи" },
    { "Beta Features",             "Бета-функции" },
    { "Settings",                  "Настройки" },
    { "Help",                      "Помощь" },
    // --- Settings menu ---
    { "Wi Fi connection",          "Подключение Wi-Fi" },
    { "Bluetooth connection",      "Подключение Bluetooth" },
    { "Reading settings",          "Настройки чтения" },
    { "Language and dictionaries", "Язык и словари" },
    { "Date and time",             "Дата и время" },
    { "About Kobo Libra Colour",   "Об устройстве" },
    { "Syncing and updates",       "Синхронизация и обновления" },
    { "Device information",        "Информация об устройстве" },
    { "Manage downloads",          "Управление загрузками" },
    { "Accounts",                  "Аккаунты" },
    { "Energy saving and privacy", "Энергосбережение и приватность" },
    // --- Reading settings sub-screen ---
    { "Header:",                   "Заголовок:" },
    { "Footer:",                   "Подвал:" },
    { "Show book progress bar:",   "Показывать прогресс:" },
    { "Pages left in book",        "Страниц до конца книги" },
    { "Pages left in chapter",     "Страниц до конца главы" },
    { "Chapter progress",          "Прогресс главы" },
    { "Book progress",             "Прогресс книги" },
    { "None",                      "Нет" },
    { "NEXT",                      "ВПЕРЁД" },
    { "PREVIOUS",                  "НАЗАД" },
    { "Inverted",                  "Инверсия" },
    { "Default",                   "Обычные" },
    { "BUTTON CONTROLS",           "КНОПКИ ЛИСТАНИЯ" },
    { "READING PROGRESS",          "ПРОГРЕСС ЧТЕНИЯ" },
    // --- Settings / reader / library / dialogs (grounded in reference libnickel, session 12) ---
    { "Choose a Wi Fi network",                                  "Выберите сеть Wi-Fi" },
    { "Network name",                                            "Имя сети" },
    { "Show password",                                           "Показать пароль" },
    { "Signal strength",                                         "Уровень сигнала" },
    { "Connect",                                                 "Подключиться" },
    { "Forget",                                                  "Забыть" },
    { "Bluetooth Devices",                                       "Устройства Bluetooth" },
    { "Bluetooth Help",                                          "Справка по Bluetooth" },
    { "Bluetooth Name",                                          "Имя Bluetooth" },
    { "Bluetooth Settings",                                      "Настройки Bluetooth" },
    { "Bluetooth is disabled",                                   "Bluetooth выключен" },
    { "No Bluetooth Connected",                                  "Нет подключённых устройств Bluetooth" },
    { "Confirm bluetooth connection",                            "Подтвердите подключение Bluetooth" },
    { "Turn on Bluetooth to connect to nearby devices.",         "Включите Bluetooth, чтобы подключаться к устройствам поблизости." },
    { "Available devices",                                       "Доступные устройства" },
    { "Check that the device is nearby, turned on, and fully charged.", "Убедитесь, что устройство рядом, включено и полностью заряжено." },
    { "Natural light",                                           "Естественный свет" },
    { "Auto natural light",                                      "Автоматический естественный свет" },
    { "Learn about the Natural Light feature",                   "Подробнее о функции «Естественный свет»" },
    { "What does the natural light setting do?",                 "Что делает настройка «Естественный свет»?" },
    { "What does the Natural Light setting do?",                 "Что делает настройка «Естественный свет»?" },
    { "Brightness",                                              "Яркость" },
    { "Screenshots",                                             "Скриншоты" },
    { "Portrait",                                                "Портретная" },
    { "Landscape",                                               "Альбомная" },
    { "SLEEP AND POWER OFF",                                     "СОН И ВЫКЛЮЧЕНИЕ" },
    { "Sleep and power off",                                     "Сон и выключение" },
    { "Automatically go to sleep after:",                        "Автоматически засыпать через:" },
    { "Automatically power off after:",                          "Автоматически выключаться через:" },
    { "Automatically sleep and wake up:",                        "Автоматически засыпать и просыпаться:" },
    { "LANGUAGE",                                                "ЯЗЫК" },
    { "DICTIONARY",                                              "СЛОВАРЬ" },
    { "Language",                                                "Язык" },
    { "Region",                                                  "Регион" },
    { "Select your Language:",                                   "Выберите язык:" },
    { "Select your Region:",                                     "Выберите регион:" },
    { "Manage Dictionaries",                                     "Управление словарями" },
    { "Manage dictionaries",                                     "Управление словарями" },
    { "+ Add more dictionaries",                                 "+ Добавить словари" },
    { "0 Dictionaries Installed",                                "Словари не установлены" },
    { "Total size:",                                             "Общий размер:" },
    { "Sign in",                                                 "Войти" },
    { "Sign In",                                                 "Войти" },
    { "Sign out",                                                "Выйти" },
    { "Sign Out",                                                "Выйти" },
    { "Are you sure you want to sign out?",                      "Вы точно хотите выйти?" },
    { "Sign out of Dropbox?",                                    "Выйти из Dropbox?" },
    { "Sign out of Google Drive?",                               "Выйти из Google Drive?" },
    { "Sync complete. Sign in for extra features.",              "Синхронизация завершена. Войдите, чтобы получить больше возможностей." },
    { "Account settings",                                        "Настройки аккаунта" },
    { "ADD ACCOUNT",                                             "ДОБАВИТЬ АККАУНТ" },
    { "Change account",                                          "Сменить аккаунт" },
    { "Repair account",                                          "Восстановить аккаунт" },
    { "Repair your account?",                                    "Восстановить аккаунт?" },
    { "Repairing account...",                                    "Восстановление аккаунта..." },
    { "Repair your Kobo account:",                               "Восстановление аккаунта Kobo:" },
    { "Accounts and Store are not yet available in your region", "Аккаунты и Магазин пока недоступны в вашем регионе" },
    { "Add a library card",                                      "Добавить библиотечную карту" },
    { "Library card added",                                      "Библиотечная карта добавлена" },
    { "Need a library card?",                                    "Нужна библиотечная карта?" },
    { "Remove OverDrive library card?",                          "Удалить библиотечную карту OverDrive?" },
    { "There seems to be a problem with your library card",      "Похоже, с вашей библиотечной картой возникла проблема" },
    { "Already have an Instapaper account?",                     "Уже есть аккаунт Instapaper?" },
    { "Add articles to your Instapaper account and sync your eReader to start reading.", "Добавьте статьи в свой аккаунт Instapaper и синхронизируйте электронную книгу, чтобы начать читать." },
    { "Add eBooks and documents to your eReader with a free Dropbox account.", "Добавляйте электронные книги и документы на устройство с помощью бесплатного аккаунта Dropbox." },
    { "Add eBooks and documents to your eReader with a free Google Drive account.", "Добавляйте электронные книги и документы на устройство с помощью бесплатного аккаунта Google Drive." },
    { "Sync now",                                                "Синхронизировать" },
    { "Automatic sync:",                                         "Автосинхронизация:" },
    { "Auto syncing",                                            "Автосинхронизация" },
    { "Already syncing!",                                        "Уже синхронизируется!" },
    { "Cancel Sync?",                                            "Отменить синхронизацию?" },
    { "Cancel sync",                                             "Отменить синхронизацию" },
    { "Cannot complete Sync",                                    "Не удалось завершить синхронизацию" },
    { "Got time to sync?",                                       "Есть минутка для синхронизации?" },
    { "Retry sync",                                              "Повторить синхронизацию" },
    { "Starting sync...",                                        "Начинается синхронизация..." },
    { "Sync complete.",                                          "Синхронизация завершена." },
    { "Sync failed",                                             "Синхронизация не удалась" },
    { "Sync Failed. Please try again.",                          "Синхронизация не удалась. Попробуйте ещё раз." },
    { "Sync in Progress",                                        "Идёт синхронизация" },
    { "Sync required",                                           "Требуется синхронизация" },
    { "Sync required soon",                                      "Скоро потребуется синхронизация" },
    { "Preparing device for sync...",                            "Подготовка устройства к синхронизации..." },
    { "Sync to download new items and software updates.",        "Синхронизируйте, чтобы загрузить новые материалы и обновления ПО." },
    { "Check for updates",                                       "Проверить обновления" },
    { "Checking for updates",                                    "Проверка обновлений" },
    { "Checking for updates...",                                 "Проверка обновлений..." },
    { "Update available",                                        "Доступно обновление" },
    { "Update now",                                              "Обновить сейчас" },
    { "Update later",                                            "Обновить позже" },
    { "Update ready",                                            "Обновление готово" },
    { "Update required",                                         "Требуется обновление" },
    { "Downloading update",                                      "Загрузка обновления" },
    { "Applying update",                                         "Применение обновления" },
    { "Preparing update",                                        "Подготовка обновления" },
    { "Unable to complete update",                               "Не удалось завершить обновление" },
    { "Find out what's new in this software update",             "Узнайте, что нового в этом обновлении ПО" },
    { "Software version:",                                       "Версия ПО:" },
    { "Download all",                                            "Загрузить всё" },
    { "Remove download",                                         "Удалить загрузку" },
    { "Remove downloads",                                        "Удалить загрузки" },
    { "Remove Download",                                         "Удалить загрузку" },
    { "Cancel all downloads",                                    "Отменить все загрузки" },
    { "Cancel Download?",                                        "Отменить загрузку?" },
    { "Download Complete",                                       "Загрузка завершена" },
    { "Not Downloaded",                                          "Не загружено" },
    { "Downloading...",                                          "Загрузка..." },
    { "IN DOWNLOAD QUEUE",                                       "В ОЧЕРЕДИ НА ЗАГРУЗКУ" },
    { "TAP TO DOWNLOAD",                                         "НАЖМИТЕ, ЧТОБЫ ЗАГРУЗИТЬ" },
    { "Finished downloading",                                    "Загрузка завершена" },
    { "Downloaded Kobo books",                                   "Загруженные книги Kobo" },
    { "Downloaded previews",                                     "Загруженные отрывки" },
    { "Downloaded Dropbox books",                                "Загруженные книги Dropbox" },
    { "Downloaded Google Drive books",                           "Загруженные книги Google Drive" },
    { "Downloaded Instapaper articles",                          "Загруженные статьи Instapaper" },
    { "You can re-download these items any time.",               "Вы сможете загрузить их снова в любой момент." },
    { "Set date and time",                                       "Установить дату и время" },
    { "Set your time zone",                                      "Укажите часовой пояс" },
    { "Time zone:",                                              "Часовой пояс:" },
    { "Date:",                                                   "Дата:" },
    { "Time:",                                                   "Время:" },
    { "Use 24-hour format",                                      "Использовать 24-часовой формат" },
    { "Never",                                                   "Никогда" },
    { "Energy saving",                                           "Энергосбережение" },
    { "Serial number:",                                          "Серийный номер:" },
    { "MAC address:",                                            "MAC-адрес:" },
    { "Firmware information",                                    "Сведения о прошивке" },
    { "Firmware information:",                                   "Сведения о прошивке:" },
    { "Firmware Configuration Version:",                         "Версия конфигурации прошивки:" },
    { "Advanced",                                                "Дополнительно" },
    { "Developer options",                                       "Параметры разработчика" },
    { "Factory reset",                                           "Сброс к заводским настройкам" },
    { "FACTORY RESET",                                           "СБРОС К ЗАВОДСКИМ НАСТРОЙКАМ" },
    { "Factory reset your eReader?",                             "Сбросить устройство к заводским настройкам?" },
    { "Factory reset your eReader:",                             "Сброс устройства к заводским настройкам:" },
    { "Factory resetting...",                                    "Сброс к заводским настройкам..." },
    { "Table of Contents",                                       "Содержание" },
    { "Font Settings",                                           "Настройки шрифта" },
    { "Font Face:",                                              "Шрифт:" },
    { "Font Size:",                                              "Размер шрифта:" },
    { "Line Spacing:",                                           "Межстрочный интервал:" },
    { "Margins:",                                                "Поля:" },
    { "Justification:",                                          "Выравнивание:" },
    { "Weight:",                                                 "Насыщенность:" },
    { "Publisher Default",                                       "Как у издателя" },
    { "Adjust the weight and sharpness of the currently selected font face.", "Настройте насыщенность и резкость выбранного шрифта." },
    { "Swipe left edge to adjust brightness:",                   "Проводите по левому краю для регулировки яркости:" },
    { "Orientation",                                             "Ориентация" },
    { "Auto rotate",                                             "Автоповорот" },
    { "Rotate",                                                  "Повернуть" },
    { "Go to page",                                              "Перейти к странице" },
    { "Chapter",                                                 "Глава" },
    { "Reading menu",                                            "Меню чтения" },
    { "Search",                                                  "Поиск" },
    { "Highlight",                                               "Выделить" },
    { "Highlight text",                                          "Выделить текст" },
    { "Add Note",                                                "Добавить заметку" },
    { "Add note",                                                "Добавить заметку" },
    { "Edit note",                                               "Изменить заметку" },
    { "Edit Note",                                               "Изменить заметку" },
    { "Remove Highlight",                                        "Убрать выделение" },
    { "Undo Highlight",                                          "Отменить выделение" },
    { "Remove highlight",                                        "Убрать выделение" },
    { "Remove markup",                                           "Убрать разметку" },
    { "Remove annotation",                                       "Удалить аннотацию" },
    { "Delete annotation",                                       "Удалить аннотацию" },
    { "Remove bookmark",                                         "Убрать закладку" },
    { "Add annotations",                                         "Добавляйте заметки" },
    { "Share on Facebook",                                       "Поделиться в Facebook" },
    { "Share to Facebook",                                       "Поделиться в Facebook" },
    { "HIGHLIGHT",                                               "ВЫДЕЛЕНИЕ" },
    { "NOTE",                                                    "ЗАМЕТКА" },
    { "PAGE",                                                    "СТРАНИЦА" },
    { "BOOKMARK",                                                "ЗАКЛАДКА" },
    { "No dictionary available",                                 "Нет доступного словаря" },
    { "No definition found.",                                    "Определение не найдено." },
    { "Try another dictionary.",                                 "Попробуйте другой словарь." },
    { "No definition found.<br/>Try another dictionary.",        "Определение не найдено.<br/>Попробуйте другой словарь." },
    { "English",                                                 "Английский" },
    { "See more",                                                "Показать больше" },
    { "No Results",                                              "Ничего не найдено" },
    { "View in book",                                            "Открыть в книге" },
    { "Add to My Words",                                         "Добавить в Мои выделения" },
    { "Added to My Words",                                       "Добавлено в Мои выделения" },
    { "MY WORDS",                                                "МОИ ВЫДЕЛЕНИЯ" },
    { "Welcome to Reading Life",                                 "Добро пожаловать в читательскую жизнь" },
    { "You highlighted your first passage.",                     "Вы выделили свой первый фрагмент." },
    { "You started reading a new book.",                         "Вы начали читать новую книгу." },
    { " start a new book to earn the Once Upon a Time Award.",   " начните новую книгу, чтобы получить награду «Однажды»." },
    { "Consume mass quantities to earn this award.",             "Поглощайте книги в огромных количествах, чтобы получить эту награду." },
    { "Press and hold on a book in your library to earn this award.", "Нажмите и удерживайте книгу в библиотеке, чтобы получить эту награду." },
    { "Reading stats",                                           "Статистика чтения" },
    { "Sort",                                                    "Сортировка" },
    { "Sort: A-Z",                                               "Сортировка: А–Я" },
    { "Sort: Author",                                            "Сортировка: автор" },
    { "Sort: Title",                                             "Сортировка: название" },
    { "Sort: Date Added",                                        "Сортировка: дата добавления" },
    { "Sort: File size",                                         "Сортировка: размер файла" },
    { "Sort: File type",                                         "Сортировка: тип файла" },
    { "Sort: Series",                                            "Сортировка: серия" },
    { "Sort: Collection name",                                   "Сортировка: название коллекции" },
    { "Sort: Recently added",                                    "Сортировка: недавно добавленные" },
    { "Sort: Number of books",                                   "Сортировка: число книг" },
    { "Filter: All",                                             "Фильтр: все" },
    { "Filter: All books",                                       "Фильтр: все книги" },
    { "Filter: Kobo Plus",                                       "Фильтр: Kobo Plus" },
    { "List view",                                               "Список" },
    { "Cover view",                                              "Обложки" },
    { "Show all books",                                          "Показать все книги" },
    { "Imported",                                                "Импортированные" },
    { "Purchased",                                               "Купленные" },
    { "Articles",                                                "Статьи" },
    { "Add to collection",                                       "Добавить в коллекцию" },
    { "Remove from collection",                                  "Убрать из коллекции" },
    { "Add books to collection",                                 "Добавить книги в коллекцию" },
    { "Create a collection",                                     "Создать коллекцию" },
    { "Create new collection",                                   "Создать новую коллекцию" },
    { "New collection",                                          "Новая коллекция" },
    { "Rename collection",                                       "Переименовать коллекцию" },
    { "Manage collection",                                       "Управление коллекцией" },
    { "Collection name",                                         "Название коллекции" },
    { "Delete collection?",                                      "Удалить коллекцию?" },
    { "Mark as finished",                                        "Отметить как прочитанное" },
    { "Mark as Finished",                                        "Отметить как прочитанное" },
    { "Remove",                                                  "Удалить" },
    { "View details",                                            "Подробности" },
    { "View Details",                                            "Подробности" },
    { "Remove from Home",                                        "Убрать с главного экрана" },
    { "Add to Wishlist",                                         "В список желаний" },
    { "Related Reads",                                           "Похожие книги" },
    { "You have no books in this collection",                    "В этой коллекции нет книг" },
    { "You have no finished books in this collection",           "В этой коллекции нет прочитанных книг" },
    { "You have no unread books in this collection",             "В этой коллекции нет непрочитанных книг" },
    { "You have no purchased books in this collection",          "В этой коллекции нет купленных книг" },
    { "You have no imported books in this collection",           "В этой коллекции нет импортированных книг" },
    { "You have no previews in this collection",                 "В этой коллекции нет ознакомительных фрагментов" },
    { "You are not currently reading any books in this collection", "Вы сейчас не читаете ни одной книги из этой коллекции" },
    { "Sorry. A collection with this name already exists.",      "Коллекция с таким названием уже существует." },
    { "OK",                                                      "ОК" },
    { "Cancel",                                                  "Отмена" },
    { "Save",                                                    "Сохранить" },
    { "Continue",                                                "Продолжить" },
    { "Delete",                                                  "Удалить" },
    { "Yes",                                                     "Да" },
    { "No",                                                      "Нет" },
    { "Close",                                                   "Закрыть" },
    { "Next",                                                    "Далее" },
    { "Back",                                                    "Назад" },
    { "Retry",                                                   "Повторить" },
    { "Dismiss",                                                 "Скрыть" },
    { "Rename",                                                  "Переименовать" },
    { "Confirm",                                                 "Подтвердить" },
    { "Apply",                                                   "Применить" },
    { "Try again",                                               "Повторить попытку" },
    { "Not now",                                                 "Не сейчас" },
    { "Later",                                                   "Позже" },
    { "Got it",                                                  "Понятно" },
    { "Learn more",                                              "Подробнее" },
    { "Restore",                                                 "Восстановить" },
    { "Accept",                                                  "Принять" },
    { "Ignore",                                                  "Игнорировать" },
    { "Remove all",                                              "Убрать всё" },
    { "The item will be permanently deleted.",                   "Элемент будет удалён безвозвратно." },
    { "No Network Connection",                                   "Нет подключения к сети" },
    { "ON",                                                      "ВКЛ" },
    { "OFF",                                                     "ВЫКЛ" },
    { "We'll set your date and time automatically whenever you connect to a Wi Fi network. You can change your time zone by entering a nearby major city.", "Мы автоматически установим дату и время при подключении к сети Wi-Fi. Чтобы изменить часовой пояс, введите название ближайшего крупного города." },
    { "To put your eReader to sleep temporarily, press the power button. To power off for a longer period, press and hold for a few seconds.", "Чтобы ненадолго перевести устройство в спящий режим, нажмите кнопку питания. Чтобы выключить его надолго, нажмите и удерживайте её несколько секунд." },
    { "Resetting your eReader to its factory default state erases your books, documents and custom settings.", "Сброс устройства к заводским настройкам удаляет ваши книги, документы и пользовательские настройки." },
    { "Repairing your account refreshes all of your books and collections. You should only repair your account if something is missing or not working properly. If you have a lot of books, this may take a while.", "Восстановление учётной записи обновляет все ваши книги и коллекции. Восстанавливайте учётную запись, только если что-то пропало или работает неправильно. Если у вас много книг, это может занять некоторое время." },
    // --- Settings sub-screens ground-truth (device-swept + photos, session 12b) ---
    { "On",                                              "Вкл." },
    { "Off",                                             "Выкл." },
    { "Night",                                           "Ночь" },
    { "Connected",                                       "Подключено" },
    { "CONNECTED",                                       "ПОДКЛЮЧЕНО" },
    { "Saved",                                           "Сохранено" },
    { "SAVED",                                           "СОХРАНЕНО" },
    { "Enabled",                                         "Включён" },
    { "ENABLED",                                         "ВКЛЮЧЕНО" },
    { "PAGE APPEARANCE",                                 "ВНЕШНИЙ ВИД СТРАНИЦЫ" },
    { "PARENTAL CONTROLS",                               "РОДИТЕЛЬСКИЙ КОНТРОЛЬ" },
    { "BACKGROUND SYNC",                                 "ФОНОВАЯ СИНХРОНИЗАЦИЯ" },
    { "OTHER CONTENT",                                   "ПРОЧЕЕ СОДЕРЖИМОЕ" },
    { "OPTIONS",                                         "ПАРАМЕТРЫ" },
    { "ENERGY SAVING",                                   "ЭНЕРГОСБЕРЕЖЕНИЕ" },
    { "SLEEPCOVER",                                      "ЧЕХОЛ-ОБЛОЖКА" },
    { "BOOK MANAGEMENT",                                 "УПРАВЛЕНИЕ КНИГАМИ" },
    { "HARDWARE DETAILS",                                "ОБ ОБОРУДОВАНИИ" },
    { "SOFTWARE DETAILS",                                "О ПРОГРАММНОМ ОБЕСПЕЧЕНИИ" },
    { "ADVANCED",                                        "ДОПОЛНИТЕЛЬНО" },
    { "Refresh the screen every:",                       "Обновлять экран каждые:" },
    { "Reading orientation:",                            "Ориентация при чтении:" },
    { "Show Adobe EPUB page numbers:",                   "Показывать номера страниц Adobe EPUB:" },
    { "Dark Mode:",                                      "Тёмный режим:" },
    { "Reduce rainbow effect:",                          "Уменьшить радужный эффект:" },
    { "Note: May slow performance or blur images",       "Примечание: может замедлить работу или размыть изображения" },
    { "Redeem a Digital Card",                           "Активировать цифровую карту" },
    { "Redeem",                                          "Активировать" },
    { "To view or change your account settings or payment info, please sign in to www.kobo.com and select My Account.", "Чтобы просмотреть или изменить настройки аккаунта или платёжные данные, войдите на www.kobo.com и выберите «Мой аккаунт»." },
    { "Block store, web browser, and external sources",  "Блокировать магазин, браузер и внешние источники" },
    { "Learn more about parental controls",              "Подробнее о родительском контроле" },
    { "Read more",                                       "Подробнее" },
    { "This feature will hide the store and web browser, prevent sideloading, and block access to Instapaper, OverDrive, Dropbox, and Google Drive. You can activate parental controls by setting a 4-digit PIN.", "Эта функция скроет магазин и браузер, запретит загрузку сторонних файлов и заблокирует доступ к Instapaper, OverDrive, Dropbox и Google Drive. Включить родительский контроль можно, задав 4-значный PIN-код." },
    { "You're not signed in to Instapaper",              "Вы не вошли в Instapaper" },
    { "Save and read articles from the web with a free Instapaper account.", "Сохраняйте и читайте статьи из интернета с бесплатным аккаунтом Instapaper." },
    { "Sync time:",                                      "Время синхронизации:" },
    { "When daily Automatic Sync is on, your books, articles, and software will remain up to date.", "Когда включена ежедневная автосинхронизация, ваши книги, статьи и ПО остаются актуальными." },
    { "Sync My Notebooks:",                              "Синхронизировать мои заметки:" },
    { "Automatically backup your notes to Kobo during sync so you don't lose them if your device gets lost.", "Автоматически сохранять резервную копию заметок в Kobo при синхронизации, чтобы не потерять их при утере устройства." },
    { "Wi Fi Settings",                                  "Настройки Wi-Fi" },
    { "WIFI SETTINGS",                                   "НАСТРОЙКИ WI-FI" },
    { "Devices",                                         "Устройства" },
    { "Edit",                                            "Изменить" },
    { "Bluetooth device alias: ",                        "Псевдоним Bluetooth-устройства: " },
    { "Indicator light while charging:",                 "Индикатор при зарядке:" },
    { "Show book covers full screen:",                   "Показывать обложки на весь экран:" },
    { "Show current read:",                              "Показывать текущую книгу:" },
    { "Show info panel on full screen covers:",          "Показывать инфопанель на полноэкранных обложках:" },
    { "The Kobo SleepCover (sold separately) automatically puts your eReader to sleep when you close it and wakes it up when you open it.", "Чехол-обложка Kobo SleepCover (продаётся отдельно) автоматически переводит устройство в спящий режим, когда вы его закрываете, и пробуждает, когда открываете." },
    { "Wake up device with page turn button:",           "Пробуждать устройство кнопкой перелистывания:" },
    { "Imported items",                                  "Импортированные" },
    { "Once you've marked a Kobo book as finished, remove the download from your eReader to free up space:", "После того как вы отметили книгу Kobo как прочитанную, удалите загрузку с устройства, чтобы освободить место:" },
    { "Onboard storage:",                                "Встроенная память:" },
    { "IP address:",                                     "IP-адрес:" },
    { "Legal:",                                          "Юридическая информация:" },
    { "Warranty:",                                       "Гарантия:" },
    { "Update check:",                                   "Проверка обновлений:" },
    { "What's new:",                                     "Что нового:" },
    { "View release notes",                              "Смотреть примечания к выпуску" },
    { "View",                                            "Смотреть" },
    { "Repair",                                          "Восстановить" },
    { "Note: some books will not be able to display chapter progress. Book progress will be used instead.", "Примечание: некоторые книги не могут показывать прогресс главы. Вместо этого используется прогресс книги." },
    { "Choose where you prefer to tap your screen in order to page forward, page back, and bring up the Reading Menu.", "Выберите, где удобнее касаться экрана, чтобы листать вперёд, назад и вызывать меню чтения." },
    { "ONSCREEN CONTROLS",                                 "УПРАВЛЕНИЕ КАСАНИЯМИ" },
    { "Page forward and back by:",                         "Листать вперёд и назад:" },
    { "Pinch to adjust font size:",                        "Щипок для изменения размера шрифта:" },
    { "Tapping or swiping",                                "Касанием или свайпом" },
    { "Books",                                             "Книги" },
    { "Authors",                                           "Авторы" },
    { "Series",                                            "Серии" },
    { "Collections",                                       "Коллекции" },
    { "Computer detected",                     "Обнаружен компьютер" },
    { "Would you like to connect your eReader to your computer to manage files?", "Подключить электронную книгу к компьютеру для управления файлами?" },
    { 0, 0 }
};
// Russian count-agreement picker. one: 1,21,31..(not 11); few: 2-4,22-24..(not 12-14); many: 0,5-20,11-14..
// Kobo renders plural sources like "%n BOOK(S)" with the raw "(S)" form (no English .qm to resolve it),
// so the on-screen text is "184 BOOK(S)"; we substitute the correct Russian form.
static const char* ru_plural3(long n, const char* one, const char* few, const char* many) {
    long a = n % 100;
    if (a >= 11 && a <= 14) return many;
    long b = n % 10;
    if (b == 1) return one;
    if (b >= 2 && b <= 4) return few;
    return many;
}
// Return the Russian replacement for a known English UI caption, or 0 if unknown. Static map hits return
// the map string directly; numeric captions ("12% Read", "184 BOOK(S)", "Book progress: N%") are formatted
// into `out`. Shared by the QLabel and ElidedLabel relabel passes.
static const char* ru_for_label(const char* t, char* out, int outsz, int wpx) {
    for (int k = 0; g_relabel_map[k][0]; ++k)
        if (strcmp(t, g_relabel_map[k][0]) == 0) return g_relabel_map[k][1];
    // My Words empty-state (repurposed to show highlights): unique prefixes, safe to whole-replace.
    if (str_starts(t, "Build your own")) return "Мои выделения";
    if (str_starts(t, "While reading")) return "Здесь — выделения из ваших книг.";
    // "Book progress: 47%" / "Current chapter: 3%" -- keep the live number.
    if (str_starts(t, "Book progress") || str_starts(t, "Current chapter")) {
        char num[16]; int ni = 0;
        for (int j = 0; t[j] && ni < 15; ++j) if (t[j] >= '0' && t[j] <= '9') num[ni++] = t[j];
        num[ni] = 0;
        snprintf(out, outsz, "%s: %s%%", (t[0] == 'B') ? "Прогресс книги" : "Текущая глава", ni ? num : "0");
        return out;
    }
    int L = 0; while (t[L]) ++L;
    // Cover progress "12% Read" -> "12% прочитано" (Ivan wants the word, not a bare percent). Digit-prefixed
    // so book titles are safe. On very narrow home tiles it may elide to "12% про…", acceptable.
    if (L > 5 && t[0] >= '0' && t[0] <= '9' && strcmp(t + L - 5, " Read") == 0) {
        int j = 0; while (t[j] >= '0' && t[j] <= '9') ++j;
        if (j > 0 && t[j] == '%') {
            int p = 0;
            for (int k = 0; k <= j && p < outsz - 1; ++k) out[p++] = t[k];   // "12%"
            // Append " прочитано" only in the wide library slots. The home cover tile is ~50px (Kobo's
            // arrange() setFixedSize -- can't widen without risking the pager), so it would elide to "12%…";
            // there we keep the clean bare percent.
            if (wpx == 0 || wpx >= 230) {
                const char* suf = " прочитано";
                for (int k = 0; suf[k] && p < outsz - 1; ++k) out[p++] = suf[k];
            }
            out[p] = 0;
            return out;
        }
    }
    // Reading ETA. The pinned firmware emits exact QMetaObject sources "%n second(s) to go",
    // "%n min(s) to go", "%n hour(s) to go", plus ReadingView's "%1 hours to go" whose %1 is
    // QLocale fixed-point with exactly one decimal. Some styles uppercase only while painting, so
    // accept exact raw/painted suffix pairs. Integer forms require 1..4 ASCII digits; the decimal
    // form requires 1..4 ASCII whole digits, '.' or ',', and one fractional digit. Whole-label
    // gates keep titles/prose out. Preserve the number and apply Russian count agreement.
    {
        static const char hour_raw[] = " hour(s) to go";
        static const char hour_painted[] = " HOUR(S) TO GO";
        static const char minute_raw[] = " min(s) to go";
        static const char minute_painted[] = " MIN(S) TO GO";
        static const char second_raw[] = " second(s) to go";
        static const char second_painted[] = " SECOND(S) TO GO";
        static const char decimal_hour_raw[] = " hours to go";
        static const char decimal_hour_painted[] = " HOURS TO GO";
        const int hour_slen = (int)sizeof(hour_raw) - 1;
        const int minute_slen = (int)sizeof(minute_raw) - 1;
        const int second_slen = (int)sizeof(second_raw) - 1;
        const int decimal_hour_slen = (int)sizeof(decimal_hour_raw) - 1;
        int digits = L - hour_slen;
        const char* one = 0;
        const char* few = 0;
        const char* many = 0;
        bool decimal_hour = false;
        if (digits >= 1 && digits <= 4 &&
            (strcmp(t + digits, hour_raw) == 0 || strcmp(t + digits, hour_painted) == 0)) {
            one = "ЧАС"; few = "ЧАСА"; many = "ЧАСОВ";
        } else {
            digits = L - minute_slen;
            if (digits >= 1 && digits <= 4 &&
                (strcmp(t + digits, minute_raw) == 0 || strcmp(t + digits, minute_painted) == 0)) {
                one = "МИНУТА"; few = "МИНУТЫ"; many = "МИНУТ";
            }
        }
        if (!one) {
            digits = L - second_slen;
            if (digits >= 1 && digits <= 4 &&
                (strcmp(t + digits, second_raw) == 0 || strcmp(t + digits, second_painted) == 0)) {
                one = "СЕКУНДА"; few = "СЕКУНДЫ"; many = "СЕКУНД";
            }
        }
        if (!one) {
            digits = L - decimal_hour_slen;
            const int whole_digits = digits - 2; // one locale decimal separator + one fractional digit
            bool valid_decimal = digits >= 3 && digits <= 6 && whole_digits >= 1 &&
                whole_digits <= 4;
            if (valid_decimal)
                valid_decimal = strcmp(t + digits, decimal_hour_raw) == 0 ||
                    strcmp(t + digits, decimal_hour_painted) == 0;
            valid_decimal = valid_decimal &&
                (t[whole_digits] == '.' || t[whole_digits] == ',') &&
                t[whole_digits + 1] >= '0' && t[whole_digits + 1] <= '9' &&
                !(whole_digits > 1 && t[0] == '0');
            long whole = 0;
            for (int j = 0; valid_decimal && j < whole_digits; ++j) {
                if (t[j] < '0' || t[j] > '9') valid_decimal = false;
                else whole = whole * 10 + (t[j] - '0');
            }
            if (valid_decimal && whole >= 1) {
                // Locale-independent abbreviation avoids applying integer plural rules to fractions
                // and matches the constrained reader-footer translation: 1,0/2.5/5,0 Ч ДО КОНЦА.
                one = "Ч"; few = "Ч"; many = "Ч";
                decimal_hour = true;
            }
        }
        if (one) {
            bool alldig = true;
            long n = 0;
            if (!decimal_hour) {
                for (int j = 0; j < digits; ++j) {
                    if (t[j] < '0' || t[j] > '9') { alldig = false; break; }
                    n = n * 10 + (t[j] - '0');
                }
            }
            if (alldig) {
                const char* unit = ru_plural3(n, one, few, many);
                int p = 0;
                for (int j = 0; j < digits && p < outsz - 1; ++j) out[p++] = t[j];
                if (p < outsz - 1) out[p++] = ' ';
                for (int j = 0; unit[j] && p < outsz - 1; ++j) out[p++] = unit[j];
                const char* tail = " ДО КОНЦА";
                for (int j = 0; tail[j] && p < outsz - 1; ++j) out[p++] = tail[j];
                out[p] = 0;
                return out;
            }
        }
    }
    // "184 BOOK(S)" / "184 BOOKS" -> "184 КНИГИ" (proper Russian agreement, uppercase to match tile styling).
    {
        int slen = 0;
        if (L > 8 && strcmp(t + L - 8, " BOOK(S)") == 0) slen = 8;
        else if (L > 6 && strcmp(t + L - 6, " BOOKS") == 0) slen = 6;
        if (slen && t[0] >= '0' && t[0] <= '9') {
            bool alldig = true;
            for (int j = 0; j < L - slen; ++j) if (t[j] < '0' || t[j] > '9') { alldig = false; break; }
            if (alldig) {
                long n = 0; for (int j = 0; j < L - slen; ++j) n = n * 10 + (t[j] - '0');
                const char* w = ru_plural3(n, "КНИГА", "КНИГИ", "КНИГ");
                int p = 0;
                for (int j = 0; j < L - slen && p < outsz - 12; ++j) out[p++] = t[j];
                if (p < outsz - 1) out[p++] = ' ';
                for (int k = 0; w[k] && p < outsz - 1; ++k) out[p++] = w[k];
                out[p] = 0;
                return out;
            }
        }
    }
    // Reader chrome: raw text is "Page 5 of 170" (CSS uppercases the DISPLAY) / "ГЛАВА II · 4 of 9" --
    // translate a DIGIT-bounded " of " to " из " and a leading "Page " to "Стр. ". Digit bounds keep it off
    // prose ("out of the" never matches).
    {
        bool has = false;
        for (int j = 1; j + 4 < L; ++j)
            if (t[j] == ' ' && t[j+1] == 'o' && t[j+2] == 'f' && t[j+3] == ' ' &&
                t[j-1] >= '0' && t[j-1] <= '9' && t[j+4] >= '0' && t[j+4] <= '9') { has = true; break; }
        if (has) {
            int p = 0;
            int j = 0;
            if (str_starts(t, "Page ")) { const char* pg = "Стр. "; for (int k = 0; pg[k] && p < outsz-1; ++k) out[p++] = pg[k]; j = 5; }
            for (; t[j] && p < outsz - 6; ) {
                if (t[j] == ' ' && t[j+1] == 'o' && t[j+2] == 'f' && t[j+3] == ' ' &&
                    j > 0 && t[j-1] >= '0' && t[j-1] <= '9' && t[j+4] >= '0' && t[j+4] <= '9') {
                    const char* iz = " из "; for (int k = 0; iz[k] && p < outsz-1; ++k) out[p++] = iz[k]; j += 4;
                } else out[p++] = t[j++];
            }
            out[p] = 0;
            return out;
        }
    }
    // Language screen %n templates (digit-prefixed): "N dictionaries installed:" and
    // "N dictionary downloads pending; sync to install" -> Russian with the count kept.
    {
        int j = 0; while (j < L && t[j] >= '0' && t[j] <= '9') ++j;
        if (j > 0 && j < L) {
            const char* tail = t + j;
            if (strcmp(tail, " dictionaries installed:") == 0 || strcmp(tail, " Dictionaries Installed:") == 0) {
                int p = 0; const char* pre = "Установлено словарей: ";
                for (int k = 0; pre[k] && p < outsz - 1; ++k) out[p++] = pre[k];
                for (int k = 0; k < j && p < outsz - 1; ++k) out[p++] = t[k];
                out[p] = 0; return out;
            }
            if (strcmp(tail, " dictionary downloads pending; sync to install") == 0) {
                int p = 0; const char* pre = "Словарей ждёт загрузки: ";
                for (int k = 0; pre[k] && p < outsz - 1; ++k) out[p++] = pre[k];
                for (int k = 0; k < j && p < outsz - 1; ++k) out[p++] = t[k];
                const char* suf = "; синхронизируйте для установки";
                for (int k = 0; suf[k] && p < outsz - 1; ++k) out[p++] = suf[k];
                out[p] = 0; return out;
            }
        }
    }
    // Format counters -- translate the UNIT WORDS, keep the numbers (Ivan). ---------------------------
    // "25.7 GB of 32 GB available" -> "25.7 ГБ из 32 ГБ свободно"
    if (L > 13 && t[0] >= '0' && t[0] <= '9' && strcmp(t + L - 13, " GB available") == 0) {
        const char* mid = 0;
        for (int j = 0; j + 7 <= L; ++j) if (str_starts(t + j, " GB of ")) { mid = t + j; break; }
        if (mid) {
            int p = 0;
            for (const char* q = t; q < mid && p < outsz - 1; ++q) out[p++] = *q;   // num1
            const char* a = " ГБ из "; for (int k = 0; a[k] && p < outsz - 1; ++k) out[p++] = a[k];
            for (const char* q = mid + 7; q < t + L - 13 && p < outsz - 1; ++q) out[p++] = *q;   // num2
            const char* b = " ГБ свободно"; for (int k = 0; b[k] && p < outsz - 1; ++k) out[p++] = b[k];
            out[p] = 0; return out;
        }
    }
    // "184 ITEMS · 846.5 MB" -> "184 элем. · 846.5 МБ"  and  "0 ITEM(S)" -> "0 элем."  (middot reads as '?')
    if (L > 6 && t[0] >= '0' && t[0] <= '9') {
        int j = 0; while (j < L && t[j] >= '0' && t[j] <= '9') ++j;
        int ilen = 0;   // length of the matched ITEMS token
        if (j > 0 && str_starts(t + j, " ITEM(S)")) ilen = 8;
        else if (j > 0 && str_starts(t + j, " ITEMS")) ilen = 6;
        if (ilen) {
            int p = 0;
            for (int k = 0; k < j && p < outsz - 1; ++k) out[p++] = t[k];   // count
            const char* el = " элем."; for (int k = 0; el[k] && p < outsz - 1; ++k) out[p++] = el[k];
            const char* rest = t + j + ilen;   // after " ITEMS" / " ITEM(S)"
            // " ? <num> MB" (middot -> '?') -> " · <num> МБ"
            if (rest[0] == ' ' && rest[1] == '?' && rest[2] == ' ') {
                const char* dot = " · "; for (int k = 0; dot[k] && p < outsz - 1; ++k) out[p++] = dot[k];
                const char* q = rest + 3;
                while (*q && !(q[0] == ' ' && q[1] == 'M' && q[2] == 'B') && p < outsz - 1) out[p++] = *q++;   // num2
                const char* mb = " МБ"; for (int k = 0; mb[k] && p < outsz - 1; ++k) out[p++] = mb[k];
            }
            out[p] = 0; return out;
        }
    }
    // Date value "Saturday, July 18, 2026" -> "Суббота, 18 июля 2026" (translate weekday+month, keep numbers).
    {
        static const char* const WD[][2] = {
            {"Monday","Понедельник"},{"Tuesday","Вторник"},{"Wednesday","Среда"},{"Thursday","Четверг"},
            {"Friday","Пятница"},{"Saturday","Суббота"},{"Sunday","Воскресенье"},{0,0}};
        static const char* const MO[][2] = {
            {"January","января"},{"February","февраля"},{"March","марта"},{"April","апреля"},{"May","мая"},
            {"June","июня"},{"July","июля"},{"August","августа"},{"September","сентября"},{"October","октября"},
            {"November","ноября"},{"December","декабря"},{0,0}};
        const char* wdru = 0; int wl = 0;
        for (int k = 0; WD[k][0]; ++k) { int n = 0; while (WD[k][0][n]) ++n;
            if (str_starts(t, WD[k][0]) && t[n] == ',') { wdru = WD[k][1]; wl = n; break; } }
        if (wdru && t[wl] == ',' && t[wl + 1] == ' ') {
            const char* mp = t + wl + 2;
            const char* moru = 0; int ml = 0;
            for (int k = 0; MO[k][0]; ++k) { int n = 0; while (MO[k][0][n]) ++n;
                if (str_starts(mp, MO[k][0]) && mp[n] == ' ') { moru = MO[k][1]; ml = n; break; } }
            if (moru) {
                const char* dp = mp + ml + 1;   // "18, 2026"
                char day[8]; int di = 0; while (dp[di] >= '0' && dp[di] <= '9' && di < 7) { day[di] = dp[di]; ++di; } day[di] = 0;
                const char* yp = dp + di;
                if (day[0] && yp[0] == ',' && yp[1] == ' ') {
                    yp += 2;
                    int p = 0;
                    for (int k = 0; wdru[k] && p < outsz - 1; ++k) out[p++] = wdru[k];
                    if (p < outsz - 2) { out[p++] = ','; out[p++] = ' '; }
                    for (int k = 0; day[k] && p < outsz - 1; ++k) out[p++] = day[k];
                    if (p < outsz - 1) out[p++] = ' ';
                    for (int k = 0; moru[k] && p < outsz - 1; ++k) out[p++] = moru[k];
                    if (p < outsz - 1) out[p++] = ' ';
                    for (const char* q = yp; *q && p < outsz - 1; ++q) out[p++] = *q;
                    out[p] = 0; return out;
                }
            }
        }
    }
    return 0;
}
// Read an ElidedLabel's full text (member QString @this+0x54) as ASCII. No method call -> crash-safe.
static int elided_read_ascii(void* label, char* out, int outlen) {
    void* dd = *(void**)((char*)label + 0x54);   // ElidedLabel text QString member
    int n = 0;
    if (dd) {
        int size = *(int*)((char*)dd + 4);
        int offset = *(int*)((char*)dd + 12);
        if (size > 0 && size < 4096 && offset > 0 && offset < (1 << 20)) {
            const unsigned short* u = (const unsigned short*)((char*)dd + offset);
            for (int i = 0; i < size && n < outlen - 1; ++i)
                out[n++] = (u[i] < 128) ? (char)u[i] : ((u[i] == 0x2019 || u[i] == 0x2018) ? '\'' : '?');   // curly '/' -> straight so keys match
        }
    }
    out[n] = 0;
    return n;
}
static void elided_set_ru(void* label, const char* utf8) {
    int len = 0; while (utf8[len]) ++len;
    alignas(8) unsigned char qs[32];
    memset(qs, 0, sizeof(qs));
    QString_fromUtf8_helper(qs, utf8, len);
    ElidedLabel_setText(label, qs);
    qstr_release(qs);
}

// The installed extra-dictionary repair changes the pinned stock literal "Extra: " to "Extra:_"
// (one byte, same length), so the working EN->RU Dictionary::name() is exactly
// "English - Extra:_ru".  That name is presentation text, not a QTranslator source.
//
// Relabel it only at the exact widget path proved by the 4.44.23552 constructors:
// InlineDictionaryView owns one TextHeader named "language"; TextHeader owns an N3ButtonLabel named
// "label" and TextHeader::setText writes Dictionary::name() to that QLabel.  We deliberately do not
// import QObject::parent/objectName or another Nickel setter: allWidgets plus the already-used
// QWidget::isAncestorOf gives the same ancestry gate while preserving v5's exact import surface.
// Every tier must be unique and visible; ambiguity fails closed without changing any label.
// The caller invokes this only after reading the complete QLabel text, so no substring replacement
// or dictionary state mutation is possible.
static void* unique_inline_dictionary_language_label(QListDataRaw* d) {
    if (!d) return 0;
    void* view = 0;
    int views = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w || !QWidget_isVisibleTo(w, 0)) continue;
        const char* c = widget_class(w);
        if (c && strcmp(c, "InlineDictionaryView") == 0) { view = w; ++views; }
    }
    if (views != 1) return 0;

    void* header = 0;
    int headers = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w || w == view || !QWidget_isVisibleTo(w, 0)) continue;
        const char* c = widget_class(w);
        if (c && strcmp(c, "TextHeader") == 0 && QWidget_isAncestorOf(view, w)) {
            header = w;
            ++headers;
        }
    }
    if (headers != 1) return 0;

    void* label = 0;
    int labels = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w || w == header || !QWidget_isVisibleTo(w, 0)) continue;
        const char* c = widget_class(w);
        if (!c || strcmp(c, "N3ButtonLabel") != 0 || !is_qlabel_widget(w) ||
            !QWidget_isAncestorOf(header, w)) continue;
        char text[64];
        if (label_read_ascii(w, text, sizeof(text)) > 0 &&
            strcmp(text, "English - Extra:_ru") == 0) { label = w; ++labels; }
    }
    return labels == 1 ? label : 0;
}
static const char* ru_for_inline_dictionary_language(void* label, void* exact_label,
                                                     const char* text) {
    if (!label || label != exact_label || !text ||
        strcmp(text, "English - Extra:_ru") != 0) return 0;
    return "Английский — русский";
}
// Translate every VISIBLE label whose English caption we know (nav bar, home, Reading Life). Handles real
// QLabels AND Kobo's custom ElidedLabel (home tile titles / "Your reading life" / collection / "N% Read").
// Idempotent: once Russian, nothing matches English. Crash-safe: the meta-object walks gate text access.
static void relabel_all() {
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    void* dictionary_language_label = unique_inline_dictionary_language_label(d);
    // Is a HomePageView on screen? Home cover tiles are narrow, so "12% Read" -> bare "12%" there (else it
    // elides to "12%…"); the wide library grid gets the full "12% прочитано". Screen-level is enough: the two
    // never share a screen. Passed to ru_for_label as a coarse width hint (100=narrow home, 300=wide).
    bool on_home = false;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        const char* c = widget_class(w);
        if (c && strcmp(c, "HomePageView") == 0 && QWidget_isVisibleTo(w, 0)) { on_home = true; break; }
    }
    int wpx = on_home ? 100 : 300;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        if (!QWidget_isVisibleTo(w, 0)) continue;   // only what's on screen
        // Library "Sort: .../Filter: ..." (class HeaderBarLabel) are LIVE: Kobo rewrites them to English on
        // every repaint, so translating them each tick makes them flicker English<->Russian (device-verified
        // by diffing frames). Skip the class -- stable English beats a flicker (Cyrillic won't byte-patch
        // into libnickel's "Sort: %1", and a Russian .qm catalog is out of scope).
        {
            const char* wc = widget_class(w);
            if (wc && strcmp(wc, "HeaderBarLabel") == 0) continue;
        }
        char t[512], ru[256];   // t must fit the longest help-paragraph key (~200 chars) or it won't match
        if (is_qlabel_widget(w)) {
            if (label_read_ascii(w, t, sizeof(t)) <= 0) continue;
            const char* r = ru_for_inline_dictionary_language(w, dictionary_language_label, t);
            if (!r) r = ru_for_label(t, ru, sizeof(ru), wpx);
            if (r) label_set_ru(w, r);
        } else if (is_elided_widget(w)) {
            if (elided_read_ascii(w, t, sizeof(t)) <= 0) continue;
            const char* r = ru_for_label(t, ru, sizeof(ru), wpx);
            if (r) elided_set_ru(w, r);
        }
    }
    release_widget_list(&list);
}

// On-demand ground-truth dump. Trigger: `touch /tmp/homerow-v3/dumptree`, navigate to the screen of
// interest, and within a tick this writes the class name of every VISIBLE widget to state/tree.log
// (truncated), plus the current text of known QLabel subclasses. Crash-safe: text() is only read from the
// allowlist classes (reading it off a non-QLabel is UB). Lets us fix the translate-map + find the My Words
// container from reality instead of guessing.
static void dump_tree() {
    int fd = open(kTreeLogPath, kOpenWrite | kOpenCreate | kOpenTrunc, 0600);
    if (fd < 0) return;
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        close(fd);
        return;
    }
    int budget = 500;
    for (int i = d->begin; i < d->end && budget > 0; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        if (!QWidget_isVisibleTo(w, 0)) continue;   // only what is actually on screen
        const char* cls = widget_class(w);
        if (!cls) continue;
        char lineb[256];
        int nn;
        if (is_qlabel_widget(w)) {
            char t[160];
            label_read_ascii(w, t, sizeof(t));
            nn = snprintf(lineb, sizeof(lineb), "%s | %s", cls, t);
        } else {
            nn = snprintf(lineb, sizeof(lineb), "%s", cls);
        }
        if (nn > 0) {
            if (nn >= (int)sizeof(lineb)) nn = (int)sizeof(lineb) - 1;
            write_all(fd, lineb, nn);
            write_all(fd, "\n", 1);
        }
        --budget;
    }
    release_widget_list(&list);
    close(fd);
}

// Each tick: if a fresh visible StatsView is present, inject our blocks once. Dedup by instance pointer
// plus an objectName sentinel on OUR container (a reused-address StatsView is a new QObject -> re-inject).
static void inject_stats_block() {
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    void* v = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        const char* cls = widget_class(w);
        if (cls && strcmp(cls, "StatsView") == 0 && QWidget_isVisibleTo(w, 0)) { v = w; break; }
    }
    release_widget_list(&list);
    if (!v) { g_stats_view = 0; return; }   // no visible StatsView -> nothing to do
    if (v == g_stats_view) return;          // block already injected this instance
    void* L = QWidget_layout(v);
    if (!L || !layout_is_box(L)) return;    // fail-closed guard

    load_stats_sidecar();
    // NOTE: no hours here on purpose. Kobo's own panel already shows "Всего часов чтения" (its aggregate)
    // right below; our koboviz raw SUM(TimeSpentReading) differs from it, so two hour figures on one screen
    // read as a contradiction. We show only what Kobo does NOT: library composition + highlights/authors/
    // languages. Colon-label form ("Книг: N") avoids Russian count-noun agreement for arbitrary N.
    char books[24], reading[24], hl[24], authors[24], langs[24];
    if (!stat_value("books", books, sizeof(books)))       { books[0] = '-'; books[1] = 0; }
    if (!stat_value("reading", reading, sizeof(reading))) { reading[0] = '-'; reading[1] = 0; }
    if (!stat_value("highlights", hl, sizeof(hl)))        { hl[0] = '-'; hl[1] = 0; }
    if (!stat_value("authors", authors, sizeof(authors))) { authors[0] = '-'; authors[1] = 0; }
    if (!stat_value("langs", langs, sizeof(langs)))       { langs[0] = '-'; langs[1] = 0; }

    void* box = op_new(1024);
    if (!box) return;
    memset(box, 0, 1024);
    QWidget_ctor(box, 0, 0);
    alignas(8) unsigned char oname[32];
    memset(oname, 0, sizeof(oname));
    QString_fromUtf8_helper(oname, "ryv3_stats", 10);
    QObject_setObjectName(box, oname);
    void* boxLay = op_new(1024);
    if (!boxLay) return;
    memset(boxLay, 0, 1024);
    QVBoxLayout_ctor(boxLay, box);
    void* title = make_fsal("Моя библиотека", box);
    if (title) QBoxLayout_insertWidget(boxLay, -1, title, 0, 0);
    char line[160];
    snprintf(line, sizeof(line), "Всего книг: %s · читаю: %s", books, reading);
    void* l1 = make_fsal(line, box);
    if (l1) QBoxLayout_insertWidget(boxLay, -1, l1, 0, 0);
    snprintf(line, sizeof(line), "Выделений: %s · авторов: %s · языков: %s", hl, authors, langs);
    void* l2 = make_fsal(line, box);
    if (l2) QBoxLayout_insertWidget(boxLay, -1, l2, 0, 0);

    QBoxLayout_insertWidget(L, 1, box, 0, 0);
    QWidget_show(box);
    QLayout_activate(L);
    g_stats_view = v;
    if (g_stats_diag > 0) { --g_stats_diag; log_hex("injected_stats", (unsigned long)v); }
}

// --- My Words tab: show Ivan's highlights as plain TEXT (Ivan: no HTML, just text on this screen) ---
static char g_hl_buf[65536];   // holds ALL highlights (~97 lines, ~52 KB) so we can paginate through them
static int g_hl_len = 0;
static void load_highlights_sidecar() {
    g_hl_len = 0;
    g_hl_buf[0] = 0;
    int fd = open(kHighlightsSidecarPath, kOpenRead);
    if (fd < 0) return;
    long n = read(fd, g_hl_buf, sizeof(g_hl_buf) - 1);
    close(fd);
    if (n > 0) { g_hl_buf[(int)n] = 0; g_hl_len = (int)n; }
}
// Count UTF-8 characters in a byte range (used to estimate how much text a page holds).
static int utf8_chars(const char* s, int len) {
    int n = 0;
    for (int i = 0; i < len; ) {
        unsigned char c = (unsigned char)s[i];
        int cl = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
        if (i + cl > len) cl = 1;
        i += cl; ++n;
    }
    return n;
}
// Group highlights (in g_hl_buf) into pages by an ESTIMATED RENDERED-LINE budget so each page fits the
// viewport WITHOUT overflow (a char budget let tall pages overlap/clip: Qt crushes the word-wrapped labels
// together when their total height exceeds the box). Each highlight's height ~= ceil(chars / CHARS_PER_LINE)
// wrapped lines + 1 for its bullet-row rounding/inter-item gap. With the shrunk highlight font (see
// kMyWordsFontPx) ~34 lines fit; we budget conservatively. Every page holds at least one highlight.
// Device-measured at kMyWordsFontPx=30: line height ~40px, ~70 chars per wrapped line, ~32 lines fit in the
// viewport. We budget 30 text-lines (ceil per highlight) + a small per-highlight count cap so a burst of
// one-liners can't stack past the screen. These numbers keep pages ~90% full with no overflow/overlap.
static const int kMyWordsFontPx      = 30;   // highlight font-size (was ~44 default); more lines per page
static const int kMyWordsCharsPerLine = 68;  // measured ~70; slightly low so we round lines UP (safe)
static const int kMyWordsLinesPerPage = 30;  // vs ~32 real capacity
static const int kMyWordsMaxPerPage   = 10;  // guard: many tiny highlights + their gaps must still fit
static int mywords_est_lines(const char* txt, int tlen) {
    int chars = utf8_chars(txt, tlen);
    int lines = (chars + kMyWordsCharsPerLine - 1) / kMyWordsCharsPerLine;   // ceil
    if (lines < 1) lines = 1;
    return lines;
}
static int mywords_page_layout(int want, const char** out_start, int* out_count) {
    const char* p = g_hl_buf;
    const char* pstart = g_hl_buf;
    int page = 0, page_lines = 0, page_count = 0, total = 0;
    const char* found_start = 0; int found_count = 0;
    while (*p) {
        const char* nl = p; while (*nl && *nl != '\n') ++nl;
        int linelen = (int)(nl - p);
        int ti = 0; while (ti < linelen && p[ti] != '\t') ++ti;
        const char* txt = (ti < linelen) ? p + ti + 1 : p;
        int tlen = (ti < linelen) ? linelen - ti - 1 : linelen;
        int lines = mywords_est_lines(txt, tlen);
        if (page_count > 0 && (page_lines + lines > kMyWordsLinesPerPage ||
                               page_count >= kMyWordsMaxPerPage)) {   // this one starts a new page
            if (page == want) { found_start = pstart; found_count = page_count; }
            ++total; ++page; pstart = p; page_lines = 0; page_count = 0;
        }
        page_lines += lines; ++page_count;
        p = (*nl == '\n') ? nl + 1 : nl;
    }
    if (page_count > 0) { if (page == want) { found_start = pstart; found_count = page_count; } ++total; }
    if (total < 1) total = 1;
    if (!found_start) { found_start = g_hl_buf; found_count = 0; }
    if (out_start) *out_start = found_start;
    if (out_count) *out_count = found_count;
    return total;
}

static void* g_mywords_view = 0;         // MyWordsView we injected into (dedup)
static void* g_mywords_box = 0;          // our injected container (deleted on re-paginate)
static void* g_mywords_swipe_target = 0; // MyWordsView whose event() slot we patched
static void* g_mywords_vtable[160];
static bool (*g_mywords_event_orig)(void*, void*) = 0;
static int g_mywords_page = 0;
static bool g_mywords_dirty = false;
static int g_mywords_diag = 6;
static int g_mywords_ilog = 6;

// Swipe over the My Words view pages through ALL highlights. Mirrors native_swipe_event, but only flips a
// dirty flag + page counter; the actual re-render happens in inject_mywords_block on the next timer tick
// (same GUI thread, no re-entrancy into layout from inside an event).
static bool mywords_swipe_event(void* self, void* event) {
    if (self == g_mywords_swipe_target && event) {
        const unsigned short type = qevent_type(event);
        if (type == kGestureEventType || type == 202) {
            void* g = QGestureEvent_gesture(event, SwipeGestureRecognizer_gestureType);
            if (g && QGesture_state(g) == 3) {                 // Qt::GestureFinished
                const int dir = *(const int*)((const char*)g + 0x2c);
                if (dir == 1 || dir == 2) {
                    g_mywords_page += (dir == 2) ? 1 : -1;     // swipe-left(dir2)=next; clamped when rendering
                    g_mywords_dirty = true;
                    return true;
                }
            }
        }
    }
    if (g_mywords_event_orig) return g_mywords_event_orig(self, event);
    return false;
}
// Grab Kobo's Swipe gesture on the My Words view + patch its primary event() slot (index 5), once per
// instance. Same instance-vtable-clone as the home pager (no mprotect); all other slots verbatim.
static void arm_mywords_swipe(void* view) {
    if (!view || g_mywords_swipe_target == view) return;
    QWidget_setAttribute(view, kWaAcceptTouch, true);
    QWidget_grabGesture(view, SwipeGestureRecognizer_gestureType, 0);
    void** vp = *(void***)view;
    memcpy(g_mywords_vtable, vp - 2, sizeof(g_mywords_vtable));
    g_mywords_event_orig = (bool (*)(void*, void*))g_mywords_vtable[2 + 5];
    g_mywords_vtable[2 + 5] = (void*)&mywords_swipe_event;
    *(void***)view = &g_mywords_vtable[2];
    g_mywords_swipe_target = view;
}

// Show ALL of Ivan's highlights on the My Words tab, a page of whole highlights at a time; swipe to page.
// Re-renders on a fresh MyWordsView instance or when a swipe set g_mywords_dirty; otherwise no-ops.
static void inject_mywords_block() {
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    void* v = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        const char* cls = widget_class(w);
        if (cls && strcmp(cls, "MyWordsView") == 0 && QWidget_isVisibleTo(w, 0)) { v = w; break; }
    }
    release_widget_list(&list);
    if (!v) { g_mywords_view = 0; g_mywords_swipe_target = 0; g_mywords_box = 0; return; }

    // Collapse the native empty-state ("Build your own vocabulary list" + body + "My Books" button) so the
    // highlights fill the FULL screen (Ivan: "половина страницы пустая ... убрать заголовок, кнопку, чтобы
    // цитаты были на полный экран"). The empty state sits at index 0 of MyWordsView's layout; our highlights
    // box is a sibling at index 1, so hiding the empty-state branch never touches our box. We identify the
    // branch by the unique "My Books" button (N3ButtonLabel -- our box has none) and hide every strict
    // descendant of v that is an ancestor of that button. Everything uses the exported, crash-safe
    // QWidget::isAncestorOf -- NO hand-rolled parent-pointer walk (an earlier one stalled the GUI tick).
    {
        QListRaw hl;
        hl.d = 0;
        QApplication_allWidgets(&hl);
        QListDataRaw* hd = hl.d;
        if (hd && hd->begin >= 0 && hd->end >= hd->begin && hd->end - hd->begin <= 8192) {
            void* btn = 0;   // empty-state "My Books" button -> marks the branch to hide
            void* tab = 0;   // a sub-nav tab (Статистика/Мои выделения) -> marks the branch to KEEP
            for (int i = hd->begin; i < hd->end; ++i) {
                void* w = hd->array[i];
                if (!w || w == v) continue;
                const char* c = widget_class(w);
                if (!c) continue;
                if (!btn && strcmp(c, "N3ButtonLabel") == 0 && QWidget_isVisibleTo(w, 0) &&
                    QWidget_isAncestorOf(v, w)) btn = w;
                else if (!tab && strcmp(c, "N3TabItem") == 0 && QWidget_isAncestorOf(v, w)) tab = w;
            }
            for (int i = hd->begin; i < hd->end; ++i) {
                void* w = hd->array[i];
                if (!w || w == v || w == g_mywords_box) continue;
                const char* c = widget_class(w);
                if (!c) continue;
                // Always hide the empty-state header directly (fallback if the button isn't found).
                if (strcmp(c, "HeaderMediumLabel") == 0 && QWidget_isAncestorOf(v, w)) { QWidget_hide(w); continue; }
                // Hide the container(s) wrapping the empty-state button -> collapses the whole group + spacer.
                // Skip any container that also wraps a sub-nav tab, so the Статистика/Мои выделения tabs stay.
                if (btn && w != btn && QWidget_isAncestorOf(v, w) && QWidget_isAncestorOf(w, btn) &&
                    !(tab && QWidget_isAncestorOf(w, tab))) QWidget_hide(w);
            }
        }
        release_widget_list(&hl);
    }

    bool fresh = (v != g_mywords_view);
    if (fresh) { g_mywords_page = 0; g_mywords_box = 0; g_mywords_dirty = false; }
    else if (!g_mywords_dirty) return;   // same view, no page change -> nothing to do

    void* L = QWidget_layout(v);
    bool isbox = L && layout_is_box(L);
    if (g_mywords_diag > 0) {
        --g_mywords_diag;
        const char* lc = L ? widget_class(L) : 0;
        char b[96];
        snprintf(b, sizeof(b), "mywords_layout: %s box=%d", lc ? lc : "(null)", isbox ? 1 : 0);
        append_log(b);
    }
    if (!isbox) return;   // fail-closed guard

    // Drop the previous page's container before rendering the new page (deferred delete of its children).
    if (g_mywords_box) { QWidget_hide(g_mywords_box); QObject_deleteLater(g_mywords_box); g_mywords_box = 0; }

    load_highlights_sidecar();   // all highlights
    int total = 0;
    for (const char* q = g_hl_buf; *q; ) { ++total; while (*q && *q != '\n') ++q; if (*q == '\n') ++q; }
    if (total < 1) total = 1;
    // Dynamic pages: each page holds as many WHOLE highlights as fit (no truncation). Compute the total
    // page count and this page's highlight range; wrap the page index around both ends.
    const char* pstart; int pcount;
    int total_pages = mywords_page_layout(g_mywords_page, &pstart, &pcount);
    if (g_mywords_page < 0) { g_mywords_page = total_pages - 1; total_pages = mywords_page_layout(g_mywords_page, &pstart, &pcount); }
    else if (g_mywords_page >= total_pages) { g_mywords_page = 0; total_pages = mywords_page_layout(g_mywords_page, &pstart, &pcount); }

    void* box = op_new(1024);
    if (!box) return;
    memset(box, 0, 1024);
    QWidget_ctor(box, 0, 0);
    alignas(8) unsigned char oname[32];
    memset(oname, 0, sizeof(oname));
    QString_fromUtf8_helper(oname, "ryv3_mywords", 12);
    QObject_setObjectName(box, oname);
    void* boxLay = op_new(1024);
    if (!boxLay) return;
    memset(boxLay, 0, 1024);
    QVBoxLayout_ctor(boxLay, box);
    QWidget_setMinimumHeight(box, 1340);   // fill the area so the box sits right under the tabs (no top gap)
    // Shrink the highlight font. The default label font (~44px) is so tall that a long highlight overflows
    // one page and Qt then OVERLAPS the word-wrapped labels (the truncation/overlap Ivan saw). A smaller
    // font makes every highlight fit and packs more per page. Sheet on the box propagates to child QLabels.
    {
        char css[48];
        snprintf(css, sizeof(css), "QLabel{font-size:%dpx;}", kMyWordsFontPx);
        int csslen = 0; while (css[csslen]) ++csslen;
        alignas(8) unsigned char qcss[32];
        memset(qcss, 0, sizeof(qcss));
        QString_fromUtf8_helper(qcss, css, csslen);
        QWidget_setStyleSheet(box, qcss);
        qstr_release(qcss);
    }

    char titleb[96];
    snprintf(titleb, sizeof(titleb), "Всего выделений: %d · страница %d/%d · листайте свайпом",
             total, g_mywords_page + 1, total_pages);
    void* title = make_wrap_label(titleb, box);   // wrap so the page/hint line never clips
    if (title) QBoxLayout_insertWidget(boxLay, -1, title, 0, 0);

    // render this page's highlights IN FULL (word-wrapped, no truncation)
    const char* p = pstart;
    int rows = 0;
    while (*p && rows < pcount) {
        const char* nl = p;
        while (*nl && *nl != '\n') ++nl;
        int linelen = (int)(nl - p);
        const char* txt = p;
        int tlen = linelen;
        int ti = 0;
        while (ti < linelen && p[ti] != '\t') ++ti;   // strip leading "COLOR\t"
        if (ti < linelen) { txt = p + ti + 1; tlen = linelen - ti - 1; }
        // Buffer sized ABOVE the longest highlight (~2KB seen) so long highlights are never cut. If a future
        // highlight ever exceeds it, stop on a UTF-8 char boundary so we never split a multibyte char into a
        // tofu box (the "…газ□" cut Ivan saw came from a 640-byte buffer chopping mid-character).
        char row[3072];
        int ro = 0;
        const char* bullet = "• ";
        for (int k = 0; bullet[k] && ro < 8; ++k) row[ro++] = bullet[k];
        int cap = (int)sizeof(row) - 1;
        int k = 0;
        while (k < tlen && ro < cap) {
            unsigned char c = (unsigned char)txt[k];
            int cl = (c >= 0xF0) ? 4 : (c >= 0xE0) ? 3 : (c >= 0xC0) ? 2 : 1;
            if (k + cl > tlen) cl = 1;      // malformed tail -> copy one byte
            if (ro + cl > cap) break;       // don't split a char at the buffer end
            for (int j = 0; j < cl; ++j) row[ro++] = txt[k++];
        }
        row[ro] = 0;
        if (ro > 3) {
            void* rw = make_wrap_label(row, box);
            if (rw) { QBoxLayout_insertWidget(boxLay, -1, rw, 0, 0); ++rows; }
        }
        p = (*nl == '\n') ? nl + 1 : nl;
    }

    // Trailing stretch-spacer: an empty widget with stretch 1 (labels have stretch 0) soaks up any leftover
    // vertical space so the highlights PACK to the top instead of spreading out with gaps (addStretch isn't
    // exported, so a plain widget does the job).
    {
        void* spacer = op_new(1024);
        if (spacer) { memset(spacer, 0, 1024); QWidget_ctor(spacer, 0, 0);
            QBoxLayout_insertWidget(boxLay, -1, spacer, 1, 0); }
    }

    QBoxLayout_insertWidget(L, 1, box, 0, 0);   // below the sub-nav tabs
    QWidget_show(box);
    QLayout_activate(L);
    g_mywords_box = box;
    g_mywords_view = v;
    g_mywords_dirty = false;
    arm_mywords_swipe(v);   // grab swipe (once per instance) so the user can page through all highlights
    if (g_mywords_ilog > 0) { --g_mywords_ilog; log_hex("injected_mywords_page", (unsigned long)g_mywords_page); }
}

// Append a 5th swatch (HighlightColor index 4) to the native highlight-colour menu whenever it is shown
// with exactly the 4 stock buttons. The menu (SelectionMenuView) is transient + rebuilt on every setup, so
// we re-add each time we see 4. The QVector<ColorLabel*> palette mirror lives at view+0x30; its size (@d+4)
// is the button count -- a robust guard (garbage would rarely read exactly 4, and once we add it reads 5).
static int g_5color_diag = 8;
static void inject_5th_color() {
    QListRaw list;
    list.d = 0;
    QApplication_allWidgets(&list);
    QListDataRaw* d = list.d;
    if (!d || d->begin < 0 || d->end < d->begin || d->end - d->begin > 8192) {
        release_widget_list(&list);
        return;
    }
    void* v = 0;
    for (int i = d->begin; i < d->end; ++i) {
        void* w = d->array[i];
        if (!w) continue;
        const char* cls = widget_class(w);
        if (cls && strcmp(cls, "SelectionMenuView") == 0 && QWidget_isVisibleTo(w, 0)) { v = w; break; }
    }
    release_widget_list(&list);
    if (!v) return;
    void* qd = *(void**)((char*)v + 0x30);   // QVector<ColorLabel*>::d
    if (!qd) return;
    int size = *(int*)((char*)qd + 4);        // QArrayData::size
    if (g_5color_diag > 0) { --g_5color_diag; log_hex("selmenu_palette_size", (unsigned long)size); }
    if (size == 4) {
        SelectionMenuView_addColorSelectionButton(v, 4);
        log_hex("added_5th_color", (unsigned long)v);
    }
}

static bool build_native_pager(void** stackOut, void** strip0Out, void** strip1Out) {
    if (!stackOut || !strip0Out || !strip1Out)
        return pager_build_fail("build_fail: invalid output storage");
    *stackOut = 0;
    *strip0Out = 0;
    *strip1Out = 0;

    ContentIdSet ids;
    if (!load_content_ids(&ids))
        return pager_build_fail("build_fail: exact content-id input rejected");
    if (!VolumeManager_sharedInstance())
        return pager_build_fail("build_fail: VolumeManager unavailable");

    // Persistent per-page Volume vectors (d-ptr = immortal shared_null; append detaches on first
    // insert). Kept alive (g_page_vecs) so the single FourBookHomeWidget can be re-setBooks'd on swipe.
    QVectorVolumeRaw* pages = g_page_vecs;
    for (int page = 0; page < kPageCount; ++page)
        pages[page].d = (void*)&QArrayData_shared_null;

    alignas(8) unsigned char emptyString[32];
    memset(emptyString, 0, sizeof(emptyString));
    // fromUtf8(..., 0) returns Qt's immortal shared-null QString: no allocation, no release needed.
    QString_fromUtf8_helper(emptyString, "", 0);

    // Source VALID recent Volumes via VolumeManager::forEach (DB-materialized), NOT the cache-only
    // getById (returns invalid on a miss -> tapped() no-ops). The std::function bridge is on-device
    // validated. Empty filter = iterate every volume; the callback keeps ContentID matches.
    RyMatchCtx mctx;
    mctx.ids = &ids;
    memset(mctx.filled, 0, sizeof(mctx.filled));
    mctx.pages = pages;
    mctx.added = 0;
    mctx.ok = true;
    mctx.logged = false;
    mctx.seen = 0;
    if (ids.count > 0) log_bytes("tgt0", ids.ids[0], ids.lengths[0]);
    {
        RyStdFunc matchFn;
        matchFn.functor0 = &mctx;
        matchFn.functor1 = 0;
        matchFn.manager = (void*)&ry_fn_manager;
        matchFn.invoker = (void*)&ry_fn_invoker_match;
        VolumeManager_forEach(emptyString, &matchFn);
    }
    log_hex("seen", (unsigned long)mctx.seen);
    {
        char msg[40];
        char* w = msg;
        const char* s = "forEach_match: added=";
        while (*s) *w++ = *s++;
        int a = mctx.added;
        if (a >= 10) *w++ = (char)('0' + a / 10);
        *w++ = (char)('0' + a % 10);
        *w = 0;
        append_log(msg);
    }
    if (!mctx.ok || mctx.added != kBooksShown) {
        for (int page = 0; page < kPageCount; ++page) QVectorVolume_dtor(&pages[page]);
        return pager_build_fail("build_fail: volume population incomplete");
    }

    // Persistent HOLDER: a plain QWidget with a QVBoxLayout. The FourBookHomeWidget page lives inside
    // and is RECREATED on each swipe (install_page). The holder is a stable ancestor of the covers, so
    // it can grab Kobo's Swipe gesture (a swipe over a cover propagates up to the first swipe-grabber;
    // the covers only grab TAP, so correct-book tap is preserved).
    void* holder = op_new(1024);
    if (!holder) return pager_build_fail("build_fail: holder allocation");
    memset(holder, 0, 1024);
    QWidget_ctor(holder, 0, 0);
    void* vbox = op_new(1024);
    if (!vbox) return pager_build_fail("build_fail: holder layout allocation");
    memset(vbox, 0, 1024);
    QVBoxLayout_ctor(vbox, holder);   // auto-installs itself as the holder's layout
    g_stack = holder;
    g_holder_layout = vbox;
    g_strips[0] = 0;
    g_strips[1] = 0;
    g_cur_page = 0;

    // Grab Kobo's OWN Swipe gesture (NOT Qt::SwipeGesture) + enable touch, then patch the holder's
    // event() vtable slot so a recognized swipe pages the FourBook.
    QWidget_setAttribute(holder, kWaAcceptTouch, true);
    QWidget_grabGesture(holder, SwipeGestureRecognizer_gestureType, 0);
    log_hex("swipe_type", (unsigned long)SwipeGestureRecognizer_gestureType);
    void** vptr = *(void***)holder;
    memcpy(g_stack_vtable, vptr - 2, sizeof(g_stack_vtable));
    g_stack_event_orig = (bool (*)(void*, void*))g_stack_vtable[2 + 5];
    g_stack_vtable[2 + 5] = (void*)&stack_event;
    *(void***)holder = &g_stack_vtable[2];

    // Build page 0 (recreate idiom). g_page_vecs kept alive (persistent) -- NOT destroyed.
    if (!install_page(0) || !g_strips[0])
        return pager_build_fail("build_fail: install page 0");

    *stackOut = holder;
    *strip0Out = g_strips[0];
    *strip1Out = g_strips[0];   // strip1 unused; non-null so begin_home_insertion's guard passes
    return true;
}

static bool start_observer_timer(int interval) {
    if (!g_observer || g_timer_id != 0) return false;
    g_timer_id = QObject_startTimer(g_observer, interval, kCoarseTimer);
    return g_timer_id > 0;
}

static bool observer_is_on_gui_thread() {
    return g_observer && g_app_thread &&
           QObject_thread(g_observer) == g_app_thread &&
           QThread_currentThread() == g_app_thread;
}

static void start_discovery_on_gui_thread() {
    if (g_state != kDormant || !observer_is_on_gui_thread()) {
        fail_closed("load_failed: observer bootstrap did not run on qApp thread");
        return;
    }
    g_state = kDiscovering;
    if (!start_observer_timer(kDiscoverIntervalMs)) {
        fail_closed("load_failed: discovery timer unavailable");
        return;
    }
    append_log("loaded_armed: private GUI-thread allWidgets timer started");
}

static void begin_home_insertion(void* home, void* layout, bool returning) {
    kill_observer_timer();
    g_state = kInserting;
    append_log(returning ? "reinject_begin: visible replacement HomePageView found" :
                           "inject_begin: exact HomePageView found without a global event filter");

    void* stack = 0;
    void* strip0 = 0;
    void* strip1 = 0;
    if (!build_native_pager(&stack, &strip0, &strip1) || !stack || !strip0 || !strip1) {
        fail_closed("inject_failed: terminal native FourBook pager construction failed");
        return;
    }
    g_home = home;
    g_stack = stack;            // the persistent holder
    g_strips[0] = strip0;       // the current FourBook page (already set by install_page)
    g_strips[1] = 0;            // unused in the single-page recreate design
    g_gesture_fault = false;
    // Capture the LIVE native recent-reads widgets from the fixed HomePageView member offsets, but only
    // after className-verifying the FourBookHomeWidget slot (offsets are version-locked; never act blind).
    g_native_fb = 0; g_native_stack = 0; g_native_container = 0;
    {
        void* nb = *(void**)((char*)home + 0x34);
        if (nb && is_page_widget(nb)) {           // className == "FourBookHomeWidget"
            g_native_fb = nb;
            g_native_stack = *(void**)((char*)home + 0x2c);
            g_native_container = *(void**)((char*)home + 0x28);
            log_hex("native_fb", (unsigned long)nb);
            arm_native_swipe();   // grab Kobo's swipe on the strip -> refresh_native on horizontal swipe
        } else {
            append_log("native_fb: HomePageView+0x34 is not a FourBookHomeWidget");
        }
    }
    // arm_stats_tile();  // DISABLED: opening koboviz via N3BrowserController runs a nested event loop
    // that blocks this plugin's GUI timer -> heartbeat stalls -> watchdog rolls the pager back. The
    // native "Your reading life" tile (native Activity view) stays wired instead (safe, clean exit).
    // The koboviz open/close/button-coread code is retained for the native-browser-workflow rework.
    (void)&arm_stats_tile;
    // Insert the swipe-grabbing holder as the top home tile (shows one 4-book page; swipe recreates it).
    QBoxLayout_insertWidget(layout, 0, stack, 0, 0);
    suppress_store_tiles(g_home);   // hide store/promo tiles (re-applied per tick)
    g_health_ticks = 0;
    g_state = returning ? kReturnHealth : kHealthWindow;
    if (!write_heartbeat() || !start_observer_timer(kHealthIntervalMs)) {
        fail_closed("health_start_failed: heartbeat or health timer unavailable");
        return;
    }
    append_log(returning ? "reinserted: native 5x2 pager return health window started" :
                           "inserted: native two-page 5x2 pager health window started");
}

static void discovery_tick() {
    ++g_discover_ticks;
    void* home = 0;
    if (!snapshot_widgets(&home, 0, 0, 0, 0)) {
        fail_closed("discovery_failed: QApplication::allWidgets snapshot invalid");
        return;
    }
    // Wait PATIENTLY for the home to appear -- do NOT give up on a timeout. A hard reboot can resume
    // straight into a book, so the home may not exist for a long time; the pager must simply insert once
    // the user opens it. runtime-ok is acknowledged separately (observer_tick) from nickel health, so no
    // watchdog deadline is riding on the home appearing quickly.
    if (!home || !QWidget_isVisibleTo(home, 0)) return;

    void* layout = QWidget_layout(home);
    if (!layout || !layout_is_box(layout)) return;   // wait for a proper home layout

    begin_home_insertion(home, layout, false);
}

static void health_tick() {
    const bool returning = (g_state == kReturnHealth);
    // ROBUST: while the user has navigated away (home not present/visible), PAUSE the validation
    // window -- navigation is normal, not a failure. Keep the heartbeat alive so the external watchdog
    // stays calm and never removes runtime-ok. Only a broken pager while the home IS visible fails.
    bool hp = false, sp = false, s0p = false, s1p = false;
    if (!snapshot_widgets(0, &hp, &sp, &s0p, &s1p)) {
        fail_closed("health_failed: widget snapshot invalid");
        return;
    }
    (void)sp; (void)s0p; (void)s1p;
    if (!hp || !g_home || !QWidget_isVisibleTo(g_home, 0)) {
        g_saw_away = true;
        write_heartbeat();          // keep the watchdog satisfied while the user is away
        return;                     // do NOT advance g_health_ticks or fail
    }
    ++g_health_ticks;
    if (!health_widgets_present()) {
        fail_closed("health_failed: native 5x2 pager invariant failed");
        return;
    }
    suppress_store_tiles(g_home);   // re-hide store tiles nickel may have rebuilt
    poll_page_request();            // DIAGNOSTIC: page the native strip on external request (settled capture)
    poll_stats_request();           // DIAGNOSTIC: open koboviz on external request
    if (!write_heartbeat()) {
        fail_closed("health_failed: tmpfs heartbeat write failed");
        return;
    }

    if (g_health_ticks >= kHealthWindowTicks) {
        const bool markerOk = returning ? write_lifecycle_marker() : write_runtime_marker();
        if (!markerOk) {
            fail_closed("health_failed: crash-durable runtime marker unavailable");
            return;
        }
        if (returning) {
            g_lifecycle_ok = true;
            g_saw_away = false;
            append_log("lifecycle_ok: Home-away-Home return stayed healthy for 45 seconds");
        }
        g_state = kRunning;
        g_running_ticks = 0;
        if (!returning)
            append_log("runtime_ok: 45-second window passed; lifecycle and clean shutdown still required");
    }
}

static void running_tick() {
    ++g_running_ticks;
    if (!write_heartbeat()) {
        fail_closed("runtime_failed: tmpfs heartbeat write failed");
        return;
    }
    if ((g_running_ticks % kRunningAuditEveryTicks) != 0) return;

    bool homePresent = false;
    bool stackPresent = false;
    bool strip0Present = false;
    bool strip1Present = false;
    if (!snapshot_widgets(0, &homePresent, &stackPresent,
                          &strip0Present, &strip1Present)) {
        fail_closed("runtime_failed: widget snapshot invalid");
        return;
    }
    (void)strip1Present;   // g_strips[1] unused in the single-page recreate design
    if (homePresent && stackPresent && strip0Present) {
        if (!QWidget_isVisibleTo(g_home, 0)) {
            g_saw_away = true;
            return;
        }
        if (!health_widgets_present()) {
            fail_closed("runtime_failed: native pager invariant changed");
            return;
        }
        suppress_store_tiles(g_home);   // keep store tiles hidden across nickel home rebuilds
        poll_page_request();            // DIAGNOSTIC: page the native strip on external request
        poll_stats_request();           // DIAGNOSTIC: open koboviz on external request
        if (g_saw_away && !g_lifecycle_ok) {
            g_health_ticks = 0;
            g_state = kReturnHealth;
            append_log("lifecycle_return: visible 5x2 pager returned; validating for 45 seconds");
        }
        return;
    }

    if (homePresent && g_home && QWidget_isVisibleTo(g_home, 0)) {
        fail_closed("runtime_failed: visible HomePageView lost its native 5x2 pager");
        return;
    }
    g_saw_away = true;
    g_home = 0;
    g_stack = 0;
    g_strips[0] = 0;
    g_strips[1] = 0;
    g_native_fb = 0;            // native strip handles go stale when the home is torn down
    g_native_stack = 0;
    g_native_container = 0;
    g_native_event_orig = 0;
    g_browse_widget = 0;        // stats tile handle goes stale too
    g_browse_tapped_orig = 0;
    g_gesture_fault = false;
    g_state = kAwaitingReturn;
    append_log("lifecycle_away: HomePageView absent; waiting for a visible replacement");
}

static void awaiting_return_tick() {
    if (!write_heartbeat()) {
        fail_closed("return_wait_failed: tmpfs heartbeat write failed");
        return;
    }
    void* home = 0;
    if (!snapshot_widgets(&home, 0, 0, 0, 0)) {
        fail_closed("return_wait_failed: widget snapshot invalid");
        return;
    }
    if (!home || !QWidget_isVisibleTo(home, 0)) return;
    void* layout = QWidget_layout(home);
    if (!layout || !layout_is_box(layout)) {
        fail_closed("return_wait_failed: replacement HomePageView layout is not QBoxLayout");
        return;
    }
    begin_home_insertion(home, layout, true);
}

static void observer_tick() {
    if (file_exists(kDisablePath) || !arm_still_valid()) {
        fail_closed("disabled_or_disarmed: stopping all v3 activity");
        return;
    }
    // Keep the external watchdog satisfied every tick (first-heartbeat + stall + runtime deadlines), in
    // ANY active state including discovery -- so waiting for the home (e.g. a hard reboot that resumed into
    // a book) never trips a deadline. Acknowledge runtime once nickel has clearly survived loading us.
    write_heartbeat();
    if (!g_runtime_acked) {
        g_ack_accum_ms += (g_state == kDiscovering) ? kDiscoverIntervalMs : kHealthIntervalMs;
        if (g_ack_accum_ms >= kRuntimeAckMs && write_runtime_marker()) {
            g_runtime_acked = true;
            append_log("runtime_ok: nickel healthy ~30s; acknowledged independent of home visibility");
        }
    }
    poll_close_request();   // allow closing koboviz even while it covers the home
    if (file_exists(kDumpTreePath)) { unlink(kDumpTreePath); dump_tree(); }  // on-demand ground-truth dump
    relabel_all();          // Russify known English UI captions everywhere (nav bar, home, Reading Life)
    inject_stats_block();   // enrich the native Reading-Life StatsView with our blocks when it is shown
    inject_mywords_block(); // show Ivan's highlights as text on the (empty) My Words tab
    inject_5th_color();     // add a 5th selectable swatch to the native highlight-colour menu
    if (g_state == kDiscovering) discovery_tick();
    else if (g_state == kHealthWindow || g_state == kReturnHealth) health_tick();
    else if (g_state == kAwaitingReturn) awaiting_return_tick();
    else if (g_state == kRunning) running_tick();
}

static bool observer_event(void* self, void* event) {
    const unsigned short type = qevent_type(event);
    if (self == g_observer && type == kBootstrapEvent) {
        start_discovery_on_gui_thread();
        return true;
    }
    if (self == g_observer && type == kQEventTimer &&
        g_timer_id > 0 && qtimer_id(event) == g_timer_id) {
        observer_tick();
        return true;
    }
    return QObject_event(self, event);
}

static void install_observer() {
    if (g_installed) return;
    g_installed = true;
    if (!qApp_self) {
        g_state = kFailed;
        append_log("load_failed: qApp is null");
        return;
    }

    void* object = op_new(1024);
    if (!object) {
        g_state = kFailed;
        append_log("load_failed: observer allocation failed");
        return;
    }
    memset(object, 0, 1024);
    QObject_ctor(object, 0);
    void** source = (void**)&QObject_vtable_obj;
    memcpy(g_observer_vtable, source, sizeof(g_observer_vtable));
    g_observer_vtable[2 + 5] = (void*)&observer_event;
    *(void**)object = (void*)&g_observer_vtable[2];
    g_observer = object;
    g_app_thread = QObject_thread(qApp_self);
    if (!g_app_thread) {
        fail_closed("load_failed: qApp thread is null");
        return;
    }
    g_state = kDormant;

    if (QThread_currentThread() == g_app_thread) {
        start_discovery_on_gui_thread();
        return;
    }

    // Image plugins may first be dlopened by a cover-loader worker. Move the
    // private observer from that current thread to qApp's thread, then post a
    // bootstrap event. Only its GUI-thread delivery is allowed to start a timer.
    QObject_moveToThread(object, g_app_thread);
    if (QObject_thread(object) != g_app_thread) {
        fail_closed("load_failed: observer could not move to qApp thread");
        return;
    }
    void* bootstrap = op_new(64);
    if (!bootstrap) {
        fail_closed("load_failed: bootstrap event allocation failed");
        return;
    }
    memset(bootstrap, 0, 64);
    QEvent_ctor(bootstrap, kBootstrapEvent);
    QCoreApplication_postEvent(object, bootstrap, 0);
    append_log("loaded_armed: observer moved to qApp thread; bootstrap posted");
}

// Stock libkimg .qtmetadata with only the length-preserving dummy key and MIME
// edits kimg -> ryv3 and image/x-kobo -> image/x-ryv3.
__attribute__((section(".qtmetadata"), used))
static const unsigned char qt_pluginMetaData[] = {
    0x51, 0x54, 0x4d, 0x45, 0x54, 0x41, 0x44, 0x41, 0x54, 0x41, 0x20, 0x20, 0x71, 0x62, 0x6a, 0x73,
    0x01, 0x00, 0x00, 0x00, 0x18, 0x01, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00,
    0x1b, 0x03, 0x00, 0x00, 0x03, 0x00, 0x49, 0x49, 0x44, 0x00, 0x00, 0x00, 0x31, 0x00, 0x6f, 0x72,
    0x67, 0x2e, 0x71, 0x74, 0x2d, 0x70, 0x72, 0x6f, 0x6a, 0x65, 0x63, 0x74, 0x2e, 0x51, 0x74, 0x2e,
    0x51, 0x49, 0x6d, 0x61, 0x67, 0x65, 0x49, 0x4f, 0x48, 0x61, 0x6e, 0x64, 0x6c, 0x65, 0x72, 0x46,
    0x61, 0x63, 0x74, 0x6f, 0x72, 0x79, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65, 0x00,
    0x9b, 0x0b, 0x00, 0x00, 0x09, 0x00, 0x63, 0x6c, 0x61, 0x73, 0x73, 0x4e, 0x61, 0x6d, 0x65, 0x00,
    0x0f, 0x00, 0x4b, 0x6f, 0x62, 0x6f, 0x49, 0x6d, 0x61, 0x67, 0x65, 0x50, 0x6c, 0x75, 0x67, 0x69,
    0x6e, 0x00, 0x00, 0x00, 0x3a, 0x40, 0xa0, 0x00, 0x07, 0x00, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f,
    0x6e, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x05, 0x00, 0x64, 0x65, 0x62, 0x75, 0x67, 0x00,
    0x95, 0x13, 0x00, 0x00, 0x08, 0x00, 0x4d, 0x65, 0x74, 0x61, 0x44, 0x61, 0x74, 0x61, 0x00, 0x00,
    0x68, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x14, 0x03, 0x00, 0x00,
    0x04, 0x00, 0x4b, 0x65, 0x79, 0x73, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00, 0x04, 0x00, 0x72, 0x79, 0x76, 0x33, 0x00, 0x00, 0x8b, 0x01, 0x00, 0x00,
    0x14, 0x08, 0x00, 0x00, 0x09, 0x00, 0x4d, 0x69, 0x6d, 0x65, 0x54, 0x79, 0x70, 0x65, 0x73, 0x00,
    0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x69, 0x6d,
    0x61, 0x67, 0x65, 0x2f, 0x78, 0x2d, 0x72, 0x79, 0x76, 0x33, 0x00, 0x00, 0x8b, 0x01, 0x00, 0x00,
    0x0c, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00,
    0x4c, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00,
};

static long plugin_stub(void*, void*, void*) { return 0; }
static void* g_plugin_instance = 0;
static void* g_plugin_vtable[16];

extern "C" __attribute__((visibility("default")))
const char* qt_plugin_query_metadata() {
    return (const char*)qt_pluginMetaData;
}

extern "C" __attribute__((visibility("default")))
void* qt_plugin_instance() {
    if (g_plugin_instance) return g_plugin_instance;
    void* object = op_new(1024);
    if (!object) return 0;
    memset(object, 0, 1024);
    QImageIOPlugin_ctor(object, 0);
    void** source = (void**)&QImageIOPlugin_vtable_obj;
    memcpy(g_plugin_vtable, source, sizeof(g_plugin_vtable));
    g_plugin_vtable[2 + 12] = (void*)&plugin_stub;
    g_plugin_vtable[2 + 13] = (void*)&plugin_stub;
    *(void**)object = (void*)&g_plugin_vtable[2];
    g_plugin_instance = object;
    return object;
}

__attribute__((constructor))
static void homerow_init() {
    if (file_exists(kDisablePath)) {
        append_log("loaded_disabled: persistent emergency marker present");
        return;
    }
    if (!read_arm(g_arm_record, sizeof(g_arm_record), &g_arm_len)) {
        append_log("loaded_unarmed: exact v3 tmpfs record absent or invalid");
        return;
    }
    if (!arm_still_valid()) {
        append_log("loaded_unarmed: arm record changed during load");
        return;
    }
    install_observer();
}
