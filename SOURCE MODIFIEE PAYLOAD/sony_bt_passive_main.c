/*
 * GhostControl Expanded - passive Bluetooth HCI observer.
 *
 * More invasive than bt-probe: it opens the discovered event-IN and ACL-IN
 * endpoints for a bounded period. It still sends no HCI command, no ACL output,
 * performs no pairing/inquiry/reset, and never detaches the system driver.
 */
#include "sony_bt_probe.h"
#include "sony_bt_passive.h"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef __PROSPERO__
#include <ps5/kernel.h>
#include <ps5/klog.h>
#endif

#define LOG_DIR  "/data/ghostpad"
#define LOG_PATH "/data/ghostpad/bt_passive.log"
#define OBSERVE_MS 5000u
#define MAX_SAMPLES 16u
#define SAMPLE_BYTES 32u

static int g_log_fd = -1;

typedef struct {
    unsigned events;
    unsigned acl;
    unsigned samples;
} ObserveStats;

static void log_line(const char *fmt, ...) {
    char buf[640];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

#ifdef __PROSPERO__
    klog_printf("[GC-BT-PASSIVE] %s\n", buf);
#endif
    if (g_log_fd >= 0) {
        size_t n = strlen(buf);
        write(g_log_fd, buf, n);
        write(g_log_fd, "\n", 1);
    }
}

#ifdef __PROSPERO__
static void elevate_own_credentials(void) {
    uint8_t caps[16];
    memset(caps, 0xff, sizeof(caps));
    pid_t p = getpid();
    kernel_set_ucred_authid(p, 0x3800000000010003ULL);
    kernel_set_ucred_caps(p, caps);
}
#endif

static void observed_packet(int is_acl, const uint8_t *packet,
                            uint32_t len, void *user) {
    ObserveStats *s = (ObserveStats *)user;
    if (is_acl) s->acl++;
    else s->events++;

    if (s->samples >= MAX_SAMPLES)
        return;

    char hex[SAMPLE_BYTES * 3u + 1u];
    const uint32_t n = len < SAMPLE_BYTES ? len : SAMPLE_BYTES;
    for (uint32_t i = 0; i < n; i++)
        snprintf(hex + i * 3u, 4u, "%02x ", packet[i]);
    hex[n * 3u] = '\0';

    log_line("%s sample[%u] len=%u: %s",
             is_acl ? "ACL" : "EVENT",
             s->samples, (unsigned)len, hex);
    s->samples++;
}

int main(void) {
    mkdir(LOG_DIR, 0755);
    g_log_fd = open(LOG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    log_line("GhostControl Expanded passive Bluetooth observer starting");

#ifndef __PROSPERO__
    log_line("This binary is intended for ProsperoOS/PS5.");
    if (g_log_fd >= 0) close(g_log_fd);
    return 1;
#else
    elevate_own_credentials();

    SonyBtRadioProbe radios[SONY_BT_PROBE_RADIOS_MAX];
    memset(radios, 0, sizeof(radios));
    int count = sony_bt_probe_radios_readonly(radios, SONY_BT_PROBE_RADIOS_MAX);

    if (count <= 0) {
        log_line("No Bluetooth HCI radio found; no endpoints were opened.");
        if (g_log_fd >= 0) close(g_log_fd);
        return 2;
    }

    const SonyBtRadioProbe *r = &radios[0];
    if (r->hci_count <= 0) {
        log_line("Radio %s has no complete HCI function.", r->dev_path);
        if (g_log_fd >= 0) close(g_log_fd);
        return 3;
    }

    log_line("Using %s VID=0x%04x PID=0x%04x HCI[0] iface=%d "
             "events=0x%02x acl_in=0x%02x",
             r->dev_path, r->vid, r->pid,
             r->hci[0].iface, r->hci[0].ep_events, r->hci[0].ep_acl_in);
    log_line("Observation window: %u ms; INPUT endpoints only; no HCI/ACL writes.",
             OBSERVE_MS);

    ObserveStats stats;
    memset(&stats, 0, sizeof(stats));
    int ret = sony_bt_passive_observe(r, 0, OBSERVE_MS, observed_packet, &stats);

    log_line("observe ret=%d events=%u acl=%u samples=%u",
             ret, stats.events, stats.acl, stats.samples);
    log_line("Finished; USB endpoints were stopped/uninitialized and device closed.");

    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }
    return ret == 0 ? 0 : 4;
#endif
}
