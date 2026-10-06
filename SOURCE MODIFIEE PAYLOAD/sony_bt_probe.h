#pragma once
#include <stdint.h>

#define SONY_BT_HCI_FUNCTIONS_MAX 4
#define SONY_BT_PROBE_RADIOS_MAX  8

typedef struct {
    int iface;
    uint8_t ep_events;
    uint8_t ep_acl_in;
    uint8_t ep_acl_out;
    uint16_t mps_events;
    uint16_t mps_acl_in;
    uint16_t mps_acl_out;
} SonyBtHciFunction;

typedef struct {
    char dev_path[32];
    uint16_t vid;
    uint16_t pid;
    int hci_count;
    SonyBtHciFunction hci[SONY_BT_HCI_FUNCTIONS_MAX];
} SonyBtRadioProbe;

/*
 * Parse a USB configuration descriptor and return Bluetooth HCI functions:
 * interface class/subclass/protocol e0/01/01, alternate setting 0, with
 * interrupt-IN events plus bulk-IN and bulk-OUT ACL endpoints.
 */
int sony_bt_find_hci_functions(const uint8_t *cfg, uint32_t len,
                               SonyBtHciFunction *out, unsigned out_cap);

#ifdef __PROSPERO__
/*
 * Read-only PS5 discovery. Enumerates /dev/ugen*.* and reads descriptors only.
 * It does not detach drivers, initialize usb_fs, claim endpoints, reset the
 * Bluetooth chip, or send any HCI command.
 */
int sony_bt_probe_radios_readonly(SonyBtRadioProbe *out, unsigned out_cap);
#endif
