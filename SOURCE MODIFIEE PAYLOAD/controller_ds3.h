#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "gc_types.h"

/*
 * Sony DualShock 3 / Sixaxis over USB.
 *
 * Common USB identity:
 *   VID:PID 054c:0268
 *   IN endpoint 0x81
 *
 * The controller requires a SET_REPORT wake/init transfer before it begins
 * normal USB input streaming.
 */

#define DS3_VID          0x054cu
#define DS3_PID          0x0268u
#define DS3_EP_IN        0x81u

int ds3_is_supported_vidpid(uint16_t vid, uint16_t pid);
const char *ds3_name(void);

/* Send the known DS3 USB initialization SET_REPORT. */
int ds3_send_init(int fd);

/* Parse one DS3 USB input report into ScePadData. */
int ds3_parse_input(const uint8_t *buf, uint32_t len, ScePadData *out_pad);

/* Packet wrapper used by the shared USB loop. */
int ds3_handle_packet(int fd, struct usb_fs_endpoint *eps,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out_pad);

/* Parse the 49-byte DS3/Sixaxis Bluetooth HID input report (ID 0x01).
 * Returns 0 for malformed reports and the known bogus byte-1=0xff frame. */
int ds3_parse_bt_input(const uint8_t *buf, uint32_t len,
                       ScePadData *out_pad);
