/* controller_ds4.c — DualShock 4 / DS4-compatible (XIM4 etc.) for Ghost-Control
 *
 * DS4 USB wire format is the public Sony report (ID 0x01, 64 bytes).
 * No init or handshake required: open the IN endpoint, reports stream at ~250 Hz.
 */

#include "controller_ds4.h"
#include <string.h>

int ds4_is_supported_vidpid(uint16_t vid, uint16_t pid) {
    if (vid == VID_SONY)
        return pid == PID_DS4_V1 || pid == PID_DS4_V2;
    if (vid == VID_HORI)
        return pid == PID_HORIPAD_FPSPLUS ||
               pid == PID_HORIPAD4 ||
               pid == PID_HORIPAD_FPSPLUS_X ||
               pid == PID_HORIPAD_MINI4;
    return 0;
}

const char *ds4_name(uint16_t vid, uint16_t pid) {
    (void)pid;
    return vid == VID_SONY ? "Sony DualShock 4" : "DS4-compatible HORI controller";
}


/* Hat lookup: index 0..8 → (up, right, down, left) bits */
static const uint8_t HAT_DPAD[9] = {
    /* 0 N  */ SCE_PAD_BUTTON_UP,
    /* 1 NE */ SCE_PAD_BUTTON_UP   | SCE_PAD_BUTTON_RIGHT,
    /* 2 E  */ SCE_PAD_BUTTON_RIGHT,
    /* 3 SE */ SCE_PAD_BUTTON_DOWN | SCE_PAD_BUTTON_RIGHT,
    /* 4 S  */ SCE_PAD_BUTTON_DOWN,
    /* 5 SW */ SCE_PAD_BUTTON_DOWN | SCE_PAD_BUTTON_LEFT,
    /* 6 W  */ SCE_PAD_BUTTON_LEFT,
    /* 7 NW */ SCE_PAD_BUTTON_UP   | SCE_PAD_BUTTON_LEFT,
    /* 8 -- */ 0u,
};

static void ds4_parse_common(const uint8_t *p, ScePadData *o) {
    if (!p || !o) return;

    memset(o, 0, sizeof(*o));
    o->leftStick.x      = p[0];
    o->leftStick.y      = p[1];
    o->rightStick.x     = p[2];
    o->rightStick.y     = p[3];
    o->analogButtons.l2 = p[7];
    o->analogButtons.r2 = p[8];

    uint32_t btn = 0;

    /* p[4]: dpad (low nibble, hat 0..8) + face buttons (high nibble). */
    uint8_t hat = p[4] & 0x0Fu;
    if (hat <= 8) btn |= HAT_DPAD[hat];
    if (p[4] & 0x10u) btn |= SCE_PAD_BUTTON_SQUARE;
    if (p[4] & 0x20u) btn |= SCE_PAD_BUTTON_CROSS;
    if (p[4] & 0x40u) btn |= SCE_PAD_BUTTON_CIRCLE;
    if (p[4] & 0x80u) btn |= SCE_PAD_BUTTON_TRIANGLE;

    /* p[5]: shoulders + Share/Options + stick clicks. */
    if (p[5] & 0x01u) btn |= SCE_PAD_BUTTON_L1;
    if (p[5] & 0x02u) btn |= SCE_PAD_BUTTON_R1;
    if (p[5] & 0x04u) btn |= SCE_PAD_BUTTON_L2;
    if (p[5] & 0x08u) btn |= SCE_PAD_BUTTON_R2;
    if (p[5] & 0x10u) btn |= SCE_PAD_BUTTON_SHARE;
    if (p[5] & 0x20u) btn |= SCE_PAD_BUTTON_OPTIONS;
    if (p[5] & 0x40u) btn |= SCE_PAD_BUTTON_L3;
    if (p[5] & 0x80u) btn |= SCE_PAD_BUTTON_R3;

    /* p[6]: PS + touchpad-click in low bits. */
    if (p[6] & 0x01u) btn |= SCE_PAD_BUTTON_PS;
    if (p[6] & 0x02u) btn |= SCE_PAD_BUTTON_TOUCH_PAD;

    o->buttons = btn;
    o->connected = 1;
    o->quat.w = 1.0f;
}

void ds4_parse_input(const uint8_t *b, ScePadData *o) {
    /* USB report 0x01: common state starts immediately after report ID. */
    ds4_parse_common(&b[1], o);
}

int ds4_parse_bt_input(const uint8_t *buf, uint32_t len, ScePadData *out_pad) {
    if (!buf || !out_pad)
        return 0;

    /*
     * Full DS4 Bluetooth report:
     *   report ID 0x11, total 78 bytes
     *   two transport/reserved bytes
     *   common gamepad state begins at byte 3.
     */
    if (len >= 78 && buf[0] == 0x11) {
        ds4_parse_common(&buf[3], out_pad);
        return 1;
    }

    /*
     * Bluetooth minimal report:
     *   report ID 0x01, 10 bytes
     *   first nine bytes are the same common state used by USB.
     */
    if (len >= 10 && buf[0] == 0x01) {
        ds4_parse_common(&buf[1], out_pad);
        return 1;
    }

    return 0;
}

int ds4_handle_packet(int fd, struct usb_fs_endpoint *eps,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out_pad) {
    (void)fd; (void)eps;

    /* DS4 USB uses report ID 0x01. Real DS4 sends 64 bytes; HORI third-party
     * pads (and XIM4) often truncate to ~27 bytes — bytes [1..9] are identical
     * so anything ≥ 10 bytes with [0]==0x01 is parseable. */
    if (len >= 10 && buf[0] == 0x01) {
        ds4_parse_input(buf, out_pad);
        return 1;
    }
    return 0;
}
