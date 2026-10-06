/*
 * GhostControl Expanded - read-only Bluetooth radio topology probe.
 *
 * This diagnostic intentionally performs descriptor discovery only. It never
 * detaches a USB driver, initializes usb_fs endpoints, resets the Bluetooth
 * controller, sends HCI commands, pairs controllers, or writes ACL traffic.
 */
#include "sony_bt_probe.h"

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

#define PROBE_LOG_DIR  "/data/ghostpad"
#define PROBE_LOG_PATH "/data/ghostpad/bt_probe.log"

static int g_log_fd = -1;

static void log_line(const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

#ifdef __PROSPERO__
    klog_printf("[GC-BT-PROBE] %s\n", buf);
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

    /* Same payload-process elevation model already used by GhostControl.
     * This changes only this short-lived diagnostic process. */
    kernel_set_ucred_authid(p, 0x3800000000010003l);
    kernel_set_ucred_caps(p, caps);
}
#endif

int main(void) {
    mkdir(PROBE_LOG_DIR, 0755);
    g_log_fd = open(PROBE_LOG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0600);

    log_line("GhostControl Expanded Bluetooth read-only probe starting");

#ifndef __PROSPERO__
    log_line("This binary is intended for ProsperoOS/PS5.");
    if (g_log_fd >= 0) close(g_log_fd);
    return 1;
#else
    elevate_own_credentials();

    SonyBtRadioProbe radios[SONY_BT_PROBE_RADIOS_MAX];
    memset(radios, 0, sizeof(radios));

    int count = sony_bt_probe_radios_readonly(
        radios, SONY_BT_PROBE_RADIOS_MAX);

    log_line("descriptor scan complete: %d Bluetooth HCI USB device(s)", count);

    for (int i = 0; i < count; i++) {
        const SonyBtRadioProbe *r = &radios[i];
        log_line("radio[%d]: %s VID=0x%04x PID=0x%04x HCI functions=%d",
                 i, r->dev_path, r->vid, r->pid, r->hci_count);

        for (int j = 0; j < r->hci_count; j++) {
            const SonyBtHciFunction *f = &r->hci[j];
            log_line("  hci[%d]: iface=%d events=0x%02x mps=%u "
                     "acl_in=0x%02x mps=%u acl_out=0x%02x mps=%u",
                     j, f->iface,
                     f->ep_events, (unsigned)f->mps_events,
                     f->ep_acl_in, (unsigned)f->mps_acl_in,
                     f->ep_acl_out, (unsigned)f->mps_acl_out);
        }
    }

    if (count == 0) {
        log_line("No complete e0/01/01 Bluetooth HCI function was found.");
        log_line("No radio state was changed; see klog for any filesystem/open errors.");
    }

    log_line("Probe finished. No USB driver was detached and no HCI traffic was sent.");

    if (g_log_fd >= 0) {
        close(g_log_fd);
        g_log_fd = -1;
    }
    return count > 0 ? 0 : 2;
#endif
}
