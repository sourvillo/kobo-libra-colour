// kobo-tap: inject a finger tap into the Kobo Libra Colour (N428) touchscreen.
//
//   kobo-tap X Y            tap at portrait framebuffer coordinates (1264x1680)
//   kobo-tap --info         print the Elan controller's axis ranges
//   kobo-tap --trace MS     print raw touch events for MS milliseconds (to record a real tap)
//
// Build: zig cc -target arm-linux-musleabihf -mcpu=cortex_a7 -std=c11 -Os -static -s kobo-tap.c -o kobo-tap
// Installed on the reader as /usr/local/bin/kobo-tap (push-books.sh re-installs it if missing).
// Known targets: «Подключиться» in the «Обнаружен компьютер» dialog = 573 1170.
//
// Writing input_event structs to the evdev node injects them into the same stream
// nickel reads.  Coordinate transform is the one verified in custom-home/touch_gesture.c:
//   screen_x = raw_y,  screen_y = 1679 - raw_x   =>   raw_x = 1679 - Y,  raw_y = X

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define DEVICE "/dev/input/event1"
enum { SCREEN_W = 1264, SCREEN_H = 1680 };

static void sleep_ms(long ms) {
    struct timespec ts = {ms / 1000, (ms % 1000) * 1000000L};
    while (nanosleep(&ts, &ts) < 0 && errno == EINTR) {
    }
}

static int emit(int fd, unsigned short type, unsigned short code, int value) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = value;
    return write(fd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev) ? 0 : -1;
}

static int abs_info(int fd, unsigned code, struct input_absinfo *info) {
    memset(info, 0, sizeof(*info));
    return ioctl(fd, EVIOCGABS(code), info);
}

static int mid_of(int fd, unsigned code, int fallback) {
    struct input_absinfo info;
    if (abs_info(fd, code, &info) < 0 || info.maximum <= info.minimum) {
        return fallback;
    }
    return info.minimum + (info.maximum - info.minimum) / 8;
}

static int do_tap(int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) {
        fprintf(stderr, "coordinates out of range: %d %d\n", x, y);
        return 64;
    }
    const int fd = open(DEVICE, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", DEVICE, strerror(errno));
        return 2;
    }
    const int raw_x = (SCREEN_H - 1) - y;
    const int raw_y = x;
    const int pressure = mid_of(fd, ABS_MT_PRESSURE, 30);
    const int major = mid_of(fd, ABS_MT_TOUCH_MAJOR, 8);
    const int tid = (int)(time(NULL) & 0x7fff);
    int rc = 0;

    for (int frame = 0; frame < 3 && rc == 0; ++frame) {
        rc |= emit(fd, EV_ABS, ABS_MT_SLOT, 0);
        rc |= emit(fd, EV_ABS, ABS_MT_TRACKING_ID, tid);
        rc |= emit(fd, EV_ABS, ABS_MT_TOUCH_MAJOR, major);
        rc |= emit(fd, EV_ABS, ABS_MT_PRESSURE, pressure);
        rc |= emit(fd, EV_ABS, ABS_MT_POSITION_X, raw_x);
        rc |= emit(fd, EV_ABS, ABS_MT_POSITION_Y, raw_y);
        if (frame == 0) {
            rc |= emit(fd, EV_KEY, BTN_TOUCH, 1);
        }
        rc |= emit(fd, EV_ABS, ABS_X, raw_x);
        rc |= emit(fd, EV_ABS, ABS_Y, raw_y);
        rc |= emit(fd, EV_SYN, SYN_REPORT, 0);
        sleep_ms(25);
    }
    rc |= emit(fd, EV_ABS, ABS_MT_SLOT, 0);
    rc |= emit(fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
    rc |= emit(fd, EV_KEY, BTN_TOUCH, 0);
    rc |= emit(fd, EV_SYN, SYN_REPORT, 0);
    close(fd);
    if (rc != 0) {
        fprintf(stderr, "write to %s failed: %s\n", DEVICE, strerror(errno));
        return 3;
    }
    printf("tap %d %d (raw %d %d)\n", x, y, raw_x, raw_y);
    return 0;
}

static int do_info(void) {
    const int fd = open(DEVICE, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", DEVICE, strerror(errno));
        return 2;
    }
    char name[128] = {0};
    ioctl(fd, EVIOCGNAME(sizeof(name)), name);
    printf("device=%s name=%s sizeof(input_event)=%zu\n", DEVICE, name, sizeof(struct input_event));
    static const struct { unsigned code; const char *name; } axes[] = {
        {ABS_X, "ABS_X"}, {ABS_Y, "ABS_Y"}, {ABS_MT_SLOT, "MT_SLOT"},
        {ABS_MT_TOUCH_MAJOR, "MT_TOUCH_MAJOR"}, {ABS_MT_POSITION_X, "MT_POSITION_X"},
        {ABS_MT_POSITION_Y, "MT_POSITION_Y"}, {ABS_MT_TOOL_TYPE, "MT_TOOL_TYPE"},
        {ABS_MT_TRACKING_ID, "MT_TRACKING_ID"}, {ABS_MT_PRESSURE, "MT_PRESSURE"},
        {ABS_MT_DISTANCE, "MT_DISTANCE"},
    };
    for (size_t i = 0; i < sizeof(axes) / sizeof(axes[0]); ++i) {
        struct input_absinfo info;
        if (abs_info(fd, axes[i].code, &info) == 0) {
            printf("%-15s min=%d max=%d value=%d\n", axes[i].name, info.minimum, info.maximum, info.value);
        }
    }
    close(fd);
    return 0;
}

static int do_trace(int ms) {
    const int fd = open(DEVICE, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", DEVICE, strerror(errno));
        return 2;
    }
    struct pollfd pfd = {.fd = fd, .events = POLLIN};
    while (poll(&pfd, 1, ms) > 0) {
        struct input_event ev[32];
        const ssize_t n = read(fd, ev, sizeof(ev));
        if (n <= 0) {
            break;
        }
        for (size_t i = 0; i < (size_t)n / sizeof(ev[0]); ++i) {
            printf("%ld.%06ld type=%u code=%u value=%d\n", (long)ev[i].input_event_sec,
                   (long)ev[i].input_event_usec, ev[i].type, ev[i].code, ev[i].value);
        }
        fflush(stdout);
    }
    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--info") == 0) {
        return do_info();
    }
    if (argc == 3 && strcmp(argv[1], "--trace") == 0) {
        return do_trace(atoi(argv[2]));
    }
    if (argc == 3) {
        return do_tap(atoi(argv[1]), atoi(argv[2]));
    }
    fprintf(stderr, "usage: %s X Y | --info | --trace MS\n", argv[0]);
    return 64;
}
