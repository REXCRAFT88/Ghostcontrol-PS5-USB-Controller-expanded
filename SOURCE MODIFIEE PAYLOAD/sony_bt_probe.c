/*
 * Bluetooth USB descriptor discovery.
 *
 * Endpoint-selection rules are adapted from the GPL-3.0 AnyPad-PS5 project
 * by sinfiltros and contributors. This module intentionally implements only
 * descriptor parsing/read-only discovery; it does not share or control HCI
 * endpoints.
 */
#include "sony_bt_probe.h"
#include <string.h>

#define USB_DT_INTERFACE_LOCAL 0x04u
#define USB_DT_ENDPOINT_LOCAL  0x05u

int sony_bt_find_hci_functions(const uint8_t *cfg, uint32_t len,
                               SonyBtHciFunction *out, unsigned out_cap) {
    if (!cfg || !out || out_cap == 0)
        return 0;

    uint32_t off = 0;
    unsigned found = 0;
    int current = -1;
    int in_hci_iface = 0;

    while (off + 2u <= len) {
        const uint8_t dlen = cfg[off];
        const uint8_t dtype = cfg[off + 1u];

        if (dlen < 2u || off + dlen > len)
            break;

        const uint8_t *d = &cfg[off];

        if (dtype == USB_DT_INTERFACE_LOCAL && dlen >= 9u) {
            const uint8_t iface = d[2];
            const uint8_t alt = d[3];
            const uint8_t klass = d[5];
            const uint8_t sub = d[6];
            const uint8_t proto = d[7];

            in_hci_iface = (alt == 0u &&
                            klass == 0xe0u &&
                            sub == 0x01u &&
                            proto == 0x01u);
            current = -1;

            if (in_hci_iface && found < out_cap) {
                current = (int)found++;
                memset(&out[current], 0, sizeof(out[current]));
                out[current].iface = iface;
            }
        } else if (dtype == USB_DT_ENDPOINT_LOCAL && dlen >= 7u &&
                   in_hci_iface && current >= 0) {
            const uint8_t addr = d[2];
            const uint8_t attr = d[3] & 0x03u;
            const uint16_t mps =
                (uint16_t)d[4] | ((uint16_t)d[5] << 8);
            SonyBtHciFunction *f = &out[current];

            if (attr == 3u && (addr & 0x80u) && !f->ep_events) {
                f->ep_events = addr;
                f->mps_events = mps;
            } else if (attr == 2u && (addr & 0x80u) && !f->ep_acl_in) {
                f->ep_acl_in = addr;
                f->mps_acl_in = mps;
            } else if (attr == 2u && !(addr & 0x80u) && !f->ep_acl_out) {
                f->ep_acl_out = addr;
                f->mps_acl_out = mps;
            }
        }

        off += dlen;
    }

    /* Compact away Bluetooth-class interfaces that are not complete HCI
     * functions (for example an isochronous voice interface). */
    unsigned keep = 0;
    for (unsigned i = 0; i < found; i++) {
        if (!out[i].ep_events || !out[i].ep_acl_in || !out[i].ep_acl_out)
            continue;
        if (keep != i)
            out[keep] = out[i];
        keep++;
    }

    return (int)keep;
}

#ifdef __PROSPERO__
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include <dev/usb/usb_endian.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int probe_one_readonly(const char *path, SonyBtRadioProbe *out) {
    int fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        return 0;

    struct usb_device_descriptor dd;
    memset(&dd, 0, sizeof(dd));
    if (ioctl(fd, USB_GET_DEVICE_DESC, &dd) != 0) {
        close(fd);
        return 0;
    }

    uint8_t cfg[1024];
    struct usb_gen_descriptor gd;
    memset(&gd, 0, sizeof(gd));
    memset(cfg, 0, sizeof(cfg));
    gd.ugd_data = cfg;
    gd.ugd_maxlen = sizeof(cfg);
    gd.ugd_config_index = 0xff; /* current configuration */

    if (ioctl(fd, USB_GET_FULL_DESC, &gd) != 0) {
        close(fd);
        return 0;
    }

    const uint32_t actual =
        gd.ugd_actlen < sizeof(cfg) ? (uint32_t)gd.ugd_actlen : (uint32_t)sizeof(cfg);

    SonyBtHciFunction funcs[SONY_BT_HCI_FUNCTIONS_MAX];
    memset(funcs, 0, sizeof(funcs));
    int n = sony_bt_find_hci_functions(cfg, actual, funcs,
                                       SONY_BT_HCI_FUNCTIONS_MAX);
    if (n <= 0) {
        close(fd);
        return 0;
    }

    memset(out, 0, sizeof(*out));
    snprintf(out->dev_path, sizeof(out->dev_path), "%s", path);
    out->vid = UGETW(dd.idVendor);
    out->pid = UGETW(dd.idProduct);
    out->hci_count = n;
    memcpy(out->hci, funcs, (size_t)n * sizeof(funcs[0]));

    close(fd);
    return 1;
}

int sony_bt_probe_radios_readonly(SonyBtRadioProbe *out, unsigned out_cap) {
    if (!out || out_cap == 0)
        return 0;

    DIR *dp = opendir("/dev");
    if (!dp)
        return 0;

    unsigned count = 0;
    struct dirent *ent;

    while (count < out_cap && (ent = readdir(dp)) != NULL) {
        if (strncmp(ent->d_name, "ugen", 4) != 0)
            continue;

        const char *dot = strchr(ent->d_name, '.');
        if (!dot || !dot[1] || strcmp(dot, ".1") == 0)
            continue;

        char path[32];
        snprintf(path, sizeof(path), "/dev/%s", ent->d_name);

        if (probe_one_readonly(path, &out[count]))
            count++;
    }

    closedir(dp);
    return (int)count;
}
#endif
