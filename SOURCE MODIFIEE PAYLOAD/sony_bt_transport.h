#pragma once
#include <stdint.h>

/*
 * Transport-neutral helpers for Bluetooth Classic HID input carried in an
 * HCI ACL packet.
 *
 * This layer DOES NOT open or configure the PS5 Bluetooth controller.
 * It only validates a complete, already-received ACL/L2CAP/HIDP frame and
 * exposes the HID report bytes to controller-specific parsers.
 */

#define BT_L2CAP_CID_HID_INTERRUPT 0x0013u
#define BT_HIDP_DATA_INPUT          0xA1u

typedef struct {
    uint16_t connection_handle;
    uint16_t cid;
    const uint8_t *report;
    uint16_t report_len;
} SonyBtHidInputView;

/*
 * Parse one complete HCI ACL packet:
 *   4-byte HCI ACL header
 *   4-byte L2CAP header
 *   1-byte HIDP transaction header (0xA1 for DATA/INPUT)
 *   HID report
 *
 * Returns 1 for a complete HID interrupt input frame, otherwise 0.
 * ACL fragmentation/reassembly belongs to the future HCI host layer.
 */
int sony_bt_extract_hid_input(const uint8_t *acl, uint32_t len,
                              SonyBtHidInputView *out);
