#include "controller_generic_hid.h"
#include <string.h>

static void set_hat(ScePadData *o, uint8_t h) {
    switch (h & 0x0f) {
        case 0: o->buttons |= SCE_PAD_BUTTON_UP; break;
        case 1: o->buttons |= SCE_PAD_BUTTON_UP|SCE_PAD_BUTTON_RIGHT; break;
        case 2: o->buttons |= SCE_PAD_BUTTON_RIGHT; break;
        case 3: o->buttons |= SCE_PAD_BUTTON_RIGHT|SCE_PAD_BUTTON_DOWN; break;
        case 4: o->buttons |= SCE_PAD_BUTTON_DOWN; break;
        case 5: o->buttons |= SCE_PAD_BUTTON_DOWN|SCE_PAD_BUTTON_LEFT; break;
        case 6: o->buttons |= SCE_PAD_BUTTON_LEFT; break;
        case 7: o->buttons |= SCE_PAD_BUTTON_LEFT|SCE_PAD_BUTTON_UP; break;
        default: break;
    }
}

int generic_hid_is_supported(uint16_t vid, uint16_t pid) {
    return (vid == GENERIC_PS2_VID && pid == GENERIC_PS2_PID) ||
           (vid == GENERIC_HORI_VID && pid == GENERIC_HORI_PID);
}

const char *generic_hid_name(uint16_t vid, uint16_t pid) {
    if (vid == GENERIC_PS2_VID && pid == GENERIC_PS2_PID)
        return "PS1/PS2 USB Gamepad Adapter";
    if (vid == GENERIC_HORI_VID && pid == GENERIC_HORI_PID)
        return "HORI/Switch-compatible USB Gamepad";
    return "Generic USB HID Gamepad";
}

static int parse_hori_00c1(const uint8_t *b, uint32_t len, ScePadData *o) {
    if (len < 7) return 0;
    memset(o, 0, sizeof(*o));

    /* Public Nintendont mappings for 0F0D:00C1:
     * byte0 buttons, byte1 system/start, byte2 hat,
     * byte3/4 left X/Y, byte5/6 right X/Y. */
    if (b[0] & 0x01) o->buttons |= SCE_PAD_BUTTON_SQUARE;   /* Y-position */
    if (b[0] & 0x02) o->buttons |= SCE_PAD_BUTTON_CIRCLE;   /* B */
    if (b[0] & 0x04) o->buttons |= SCE_PAD_BUTTON_CROSS;    /* A */
    if (b[0] & 0x08) o->buttons |= SCE_PAD_BUTTON_TRIANGLE; /* X */
    if (b[0] & 0x20) o->buttons |= SCE_PAD_BUTTON_R1;       /* Z/R */
    if (b[0] & 0x40) o->buttons |= SCE_PAD_BUTTON_L1;
    if (b[0] & 0x80) o->buttons |= SCE_PAD_BUTTON_R2;

    if (b[1] & 0x01) o->buttons |= SCE_PAD_BUTTON_PS;
    if (b[1] & 0x02) o->buttons |= SCE_PAD_BUTTON_OPTIONS;
    if (b[1] & 0x04) { o->buttons |= SCE_PAD_BUTTON_L2; o->analogButtons.l2=255; }
    if (b[1] & 0x08) { o->buttons |= SCE_PAD_BUTTON_R2; o->analogButtons.r2=255; }
    if (b[1] & 0x10) o->buttons |= SCE_PAD_BUTTON_SHARE;
    if (b[1] & 0x20) o->buttons |= SCE_PAD_BUTTON_L3;
    if (b[1] & 0x40) o->buttons |= SCE_PAD_BUTTON_R3;

    set_hat(o, b[2]);
    o->leftStick.x=b[3]; o->leftStick.y=b[4];
    o->rightStick.x=b[5]; o->rightStick.y=b[6];
    o->connected=1; o->quat.w=1.0f;
    return 1;
}

static int parse_ps2_0810(const uint8_t *b, uint32_t len, ScePadData *o) {
    if (len < 6) return 0;
    memset(o, 0, sizeof(*o));

    /* 0810 family commonly uses:
     * [report-id?] LX LY RX RY HAT+FACE MISC [optional]
     * Accept either 7/8-byte report-ID form or 6/7-byte raw form. */
    uint32_t base = 0;
    if (len >= 7 && (b[0] == 0x01 || b[0] == 0x00))
        base = 1;
    if (len < base + 6) return 0;

    o->leftStick.x  = b[base+0];
    o->leftStick.y  = b[base+1];
    o->rightStick.x = b[base+2];
    o->rightStick.y = b[base+3];

    uint8_t fb = b[base+4];
    uint8_t mb = b[base+5];
    set_hat(o, fb & 0x0f);

    if (fb & 0x10) o->buttons |= SCE_PAD_BUTTON_TRIANGLE;
    if (fb & 0x20) o->buttons |= SCE_PAD_BUTTON_CROSS;
    if (fb & 0x40) o->buttons |= SCE_PAD_BUTTON_CIRCLE;
    if (fb & 0x80) o->buttons |= SCE_PAD_BUTTON_SQUARE;

    if (mb & 0x01) o->buttons |= SCE_PAD_BUTTON_L1;
    if (mb & 0x02) o->buttons |= SCE_PAD_BUTTON_R1;
    if (mb & 0x04) { o->buttons |= SCE_PAD_BUTTON_L2; o->analogButtons.l2=255; }
    if (mb & 0x08) { o->buttons |= SCE_PAD_BUTTON_R2; o->analogButtons.r2=255; }
    if (mb & 0x10) o->buttons |= SCE_PAD_BUTTON_SHARE;
    if (mb & 0x20) o->buttons |= SCE_PAD_BUTTON_OPTIONS;
    if (mb & 0x40) o->buttons |= SCE_PAD_BUTTON_L3;
    if (mb & 0x80) o->buttons |= SCE_PAD_BUTTON_R3;

    o->connected=1; o->quat.w=1.0f;
    return 1;
}

int generic_hid_parse(uint16_t vid, uint16_t pid,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out) {
    if (!buf || !out) return 0;
    if (vid == GENERIC_HORI_VID && pid == GENERIC_HORI_PID)
        return parse_hori_00c1(buf, len, out);
    if (vid == GENERIC_PS2_VID && pid == GENERIC_PS2_PID)
        return parse_ps2_0810(buf, len, out);
    return 0;
}
