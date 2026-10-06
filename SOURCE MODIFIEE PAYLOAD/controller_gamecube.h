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
#define GAMECUBE_ADAPTER_PORTS        4u

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
