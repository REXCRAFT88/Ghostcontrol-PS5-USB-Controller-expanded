#pragma once
#include <stdint.h>
#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include "gc_types.h"

/*
 * Nintendo GameCube Controller Adapter (Wii U / Switch USB adapter protocol).
 *
 * Official Nintendo adapter:
 *   VID:PID       057e:0337
 *   IN endpoint   0x81
 *   OUT endpoint  0x02
 *   input report  37 bytes: 0x21 + four 9-byte controller-port records
 *
 * Many third-party adapters in "Wii U"/"Switch" mode emulate this exact
 * protocol and VID/PID, so they can use the same backend.
 */

#define GAMECUBE_ADAPTER_VID          0x057eu
#define GAMECUBE_ADAPTER_PID          0x0337u
#define GAMECUBE_ADAPTER_EP_IN        0x81u
#define GAMECUBE_ADAPTER_EP_OUT       0x02u
#define GAMECUBE_ADAPTER_REPORT_ID    0x21u
#define GAMECUBE_ADAPTER_REPORT_SIZE  37u
#define GAMECUBE_ADAPTER_INIT_CMD     0x13u
#define GAMECUBE_ADAPTER_RUMBLE_CMD   0x11u
#define GAMECUBE_ADAPTER_PORTS        4u

/* Mayflash/EVORETRO/DragonRise PC-mode identities used by current SDL/Linux. */
#define GAMECUBE_PC_VID                0x0079u
#define GAMECUBE_PC_PID_1843           0x1843u
#define GAMECUBE_PC_PID_1844           0x1844u
#define GAMECUBE_PC_PID_1846           0x1846u

int gamecube_is_nintendo_adapter(uint16_t vid, uint16_t pid);
int gamecube_is_pc_adapter(uint16_t vid, uint16_t pid);
int gamecube_is_adapter(uint16_t vid, uint16_t pid);
const char *gamecube_name(void);

/* Send the adapter's one-byte 0x13 start command on eps[1] (OUT). */
int gamecube_send_init(int fd, struct usb_fs_endpoint *eps);

/*
 * Parse one of the adapter's four controller ports into ScePadData.
 *
 * port: 0..3
 * Returns 1 when a controller is connected and out_pad was populated,
 *         0 when that port is empty or the report is not valid.
 */
int gamecube_parse_port(const uint8_t *buf, uint32_t len, unsigned port,
                        ScePadData *out_pad);

/*
 * Parse Mayflash/DragonRise PC-mode GameCube reports.
 *
 * Legacy firmware:
 *   10 bytes total
 *   byte 0 = controller slot (1..4)
 *   bytes 1..9 = controller report
 *
 * Newer firmware (v0x7+):
 *   9 bytes total
 *   no slot prefix; one controller stream is exposed by the HID device
 *
 * out_port receives 0..3 for legacy reports and 0 for the 9-byte form.
 */
int gamecube_parse_pc_packet(const uint8_t *buf, uint32_t len,
                             unsigned *out_port, ScePadData *out_pad);

/* Send Nintendo/Wii-U adapter rumble state for ports 1..4.
 * Each state byte is normalized to 0 (off) or 1 (on). */
int gamecube_send_nintendo_rumble(int fd, struct usb_fs_endpoint *eps,
                                  const uint8_t state[GAMECUBE_ADAPTER_PORTS]);

/* Parse VDA remote-setting bytes into a simple GameCube rumble request.
 * The observed DS4-style layout stores small/large motor intensity at [3]/[4].
 * Returns 1 when either motor is non-zero, otherwise 0. */
int gamecube_feedback_wants_rumble(const uint8_t *buf, uint32_t len);
