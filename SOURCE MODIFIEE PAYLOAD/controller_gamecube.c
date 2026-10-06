#include "controller_gamecube.h"
#include "usb_helpers.h"
#include <string.h>
#include <sys/ioctl.h>

#ifdef __PROSPERO__
#include <dev/usb/usb_endian.h>
#endif

int gamecube_is_nintendo_adapter(uint16_t vid, uint16_t pid) {
    return vid == GAMECUBE_ADAPTER_VID && pid == GAMECUBE_ADAPTER_PID;
}

int gamecube_is_pc_adapter(uint16_t vid, uint16_t pid) {
    if (vid != GAMECUBE_PC_VID)
        return 0;
    return pid == GAMECUBE_PC_PID_1843 ||
           pid == GAMECUBE_PC_PID_1844 ||
           pid == GAMECUBE_PC_PID_1846;
}

int gamecube_is_adapter(uint16_t vid, uint16_t pid) {
    return gamecube_is_nintendo_adapter(vid, pid) ||
           gamecube_is_pc_adapter(vid, pid);
}

const char *gamecube_name(void) {
    return "Nintendo GameCube Controller Adapter";
}

int gamecube_send_init(int fd, struct usb_fs_endpoint *eps) {
#ifdef __PROSPERO__
    /*
     * Dolphin sends this class/interface request because some Nyko/off-brand
     * adapters need it before they begin behaving like the Wii U adapter.
     * Mayflash Wii U mode can return a pipe/stall here and still work, so this
     * compatibility request is intentionally non-fatal.
     */
    struct usb_ctl_request req;
    memset(&req, 0, sizeof(req));
    req.ucr_request.bmRequestType = 0x21;
    req.ucr_request.bRequest = 11;
    USETW(req.ucr_request.wValue, 0x0001);
    USETW(req.ucr_request.wIndex, 0x0000);
    USETW(req.ucr_request.wLength, 0x0000);
    req.ucr_data = NULL;
    (void)ioctl(fd, USB_DO_REQUEST, &req);
#else
    (void)fd;
#endif

    const uint8_t cmd = GAMECUBE_ADAPTER_INIT_CMD;
    return usb_send_out(fd, &eps[1], &cmd, 1, "gc-init");
}

static int gamecube_port_present(uint8_t status) {
    /* Bit 4 = wired controller, bit 5 = wireless receiver/controller. */
    return (status & 0x30u) != 0;
}

int gamecube_parse_port(const uint8_t *buf, uint32_t len, unsigned port,
                        ScePadData *out_pad) {
    if (!buf || !out_pad || port >= GAMECUBE_ADAPTER_PORTS)
        return 0;
    if (len < GAMECUBE_ADAPTER_REPORT_SIZE ||
        buf[0] != GAMECUBE_ADAPTER_REPORT_ID)
        return 0;

    const uint8_t *p = &buf[1u + (9u * port)];
    if (!gamecube_port_present(p[0]))
        return 0;

    const uint8_t b1 = p[1];
    const uint8_t b2 = p[2];
    uint32_t buttons = 0;

    /* Face buttons. */
    if (b1 & 0x01u) buttons |= SCE_PAD_BUTTON_CROSS;     /* A */
    if (b1 & 0x02u) buttons |= SCE_PAD_BUTTON_CIRCLE;    /* B */
    if (b1 & 0x04u) buttons |= SCE_PAD_BUTTON_SQUARE;    /* X */
    if (b1 & 0x08u) buttons |= SCE_PAD_BUTTON_TRIANGLE;  /* Y */

    /* D-pad. */
    if (b1 & 0x10u) buttons |= SCE_PAD_BUTTON_LEFT;
    if (b1 & 0x20u) buttons |= SCE_PAD_BUTTON_RIGHT;
    if (b1 & 0x40u) buttons |= SCE_PAD_BUTTON_DOWN;
    if (b1 & 0x80u) buttons |= SCE_PAD_BUTTON_UP;

    /* Start/Z/L/R.  Preserve the analog L/R travel as L2/R2 below. */
    if (b2 & 0x01u) buttons |= SCE_PAD_BUTTON_OPTIONS; /* Start */
    if (b2 & 0x02u) buttons |= SCE_PAD_BUTTON_R1;      /* Z */
    if (b2 & 0x04u) buttons |= SCE_PAD_BUTTON_R2;      /* R click */
    if (b2 & 0x08u) buttons |= SCE_PAD_BUTTON_L2;      /* L click */

    memset(out_pad, 0, sizeof(*out_pad));
    out_pad->buttons = buttons;

    /*
     * GameCube adapter axes are centered at 128. X orientation matches PS5.
     * GameCube Y increases upward while ScePad/DualShock Y increases downward,
     * so invert the two Y axes.
     */
    out_pad->leftStick.x  = p[3];
    out_pad->leftStick.y  = (uint8_t)(255u - p[4]);
    out_pad->rightStick.x = p[5];
    out_pad->rightStick.y = (uint8_t)(255u - p[6]);

    out_pad->analogButtons.l2 = p[7];
    out_pad->analogButtons.r2 = p[8];

    /*
     * Do not synthesize the digital L2/R2 bits from analog travel here.
     * The GameCube controller has a distinct hard-click at the end of each
     * trigger; b2 preserves that click while analogButtons preserves travel.
     */

    out_pad->connected = 1;
    out_pad->quat.w = 1.0f;
    return 1;
}

int gamecube_parse_pc_packet(const uint8_t *buf, uint32_t len,
                             unsigned *out_port, ScePadData *out_pad) {
    if (!buf || !out_pad)
        return 0;

    const uint8_t *p = NULL;
    unsigned port = 0;
    int invert_c_stick = 0;

    if (len == 10u) {
        /* Older firmware: first byte identifies controller 1..4. */
        if (buf[0] < 1u || buf[0] > GAMECUBE_ADAPTER_PORTS)
            return 0;
        port = (unsigned)(buf[0] - 1u);
        p = &buf[1];
        invert_c_stick = 1;
    } else if (len == 9u) {
        /* Firmware v0x7+: one controller stream, no explicit port byte. */
        port = 0;
        p = buf;
        invert_c_stick = 0;
    } else {
        return 0;
    }

    uint32_t buttons = 0;

    /* Face buttons. SDL's HIDAPI GameCube driver documents this PC layout. */
    if (p[0] & 0x02u) buttons |= SCE_PAD_BUTTON_CROSS;     /* A */
    if (p[0] & 0x04u) buttons |= SCE_PAD_BUTTON_CIRCLE;    /* B */
    if (p[0] & 0x01u) buttons |= SCE_PAD_BUTTON_SQUARE;    /* X */
    if (p[0] & 0x08u) buttons |= SCE_PAD_BUTTON_TRIANGLE;  /* Y */

    /* D-pad + Start. */
    if (p[1] & 0x80u) buttons |= SCE_PAD_BUTTON_LEFT;
    if (p[1] & 0x20u) buttons |= SCE_PAD_BUTTON_RIGHT;
    if (p[1] & 0x40u) buttons |= SCE_PAD_BUTTON_DOWN;
    if (p[1] & 0x10u) buttons |= SCE_PAD_BUTTON_UP;
    if (p[1] & 0x02u) buttons |= SCE_PAD_BUTTON_OPTIONS;

    /* Z + physical L/R trigger clicks. */
    if (p[0] & 0x80u) buttons |= SCE_PAD_BUTTON_R1; /* Z */
    if (p[0] & 0x20u) buttons |= SCE_PAD_BUTTON_R2; /* R click */
    if (p[0] & 0x10u) buttons |= SCE_PAD_BUTTON_L2; /* L click */

    memset(out_pad, 0, sizeof(*out_pad));
    out_pad->buttons = buttons;

    out_pad->leftStick.x = p[2];
    out_pad->leftStick.y = (uint8_t)(255u - p[3]);

    /*
     * Legacy PC firmware exposes the C-stick in the opposite orientation
     * from the v0x7+ report. Keep this transport quirk local to the parser.
     */
    if (invert_c_stick) {
        out_pad->rightStick.x = (uint8_t)(255u - p[5]);
        out_pad->rightStick.y = p[4];
    } else {
        out_pad->rightStick.x = p[5];
        out_pad->rightStick.y = (uint8_t)(255u - p[4]);
    }

    out_pad->analogButtons.l2 = p[6];
    out_pad->analogButtons.r2 = p[7];
    out_pad->connected = 1;
    out_pad->quat.w = 1.0f;

    if (out_port)
        *out_port = port;
    return 1;
}

int gamecube_send_nintendo_rumble(int fd, struct usb_fs_endpoint *eps,
                                  const uint8_t state[GAMECUBE_ADAPTER_PORTS]) {
    if (!eps || !state)
        return -1;

    uint8_t packet[1u + GAMECUBE_ADAPTER_PORTS];
    packet[0] = GAMECUBE_ADAPTER_RUMBLE_CMD;
    for (unsigned i = 0; i < GAMECUBE_ADAPTER_PORTS; i++)
        packet[1u + i] = state[i] ? 1u : 0u;

    return usb_send_out(fd, &eps[1], packet, sizeof(packet), "gc-rumble");
}

int gamecube_feedback_wants_rumble(const uint8_t *buf, uint32_t len) {
    if (!buf || len < 5u)
        return 0;
    return buf[3] != 0u || buf[4] != 0u;
}

enum {
    GC_AXIS_LX = 0,
    GC_AXIS_LY,
    GC_AXIS_RX,
    GC_AXIS_RY,
    GC_AXIS_LT,
    GC_AXIS_RT,
    GC_AXIS_COUNT
};

static uint8_t gamecube_remap_axis(uint8_t value, uint8_t minv, uint8_t maxv) {
    if (maxv <= minv)
        return 128u;
    if (value <= minv)
        return 0u;
    if (value >= maxv)
        return 255u;

    uint32_t num = (uint32_t)(value - minv) * 255u;
    uint32_t den = (uint32_t)(maxv - minv);
    return (uint8_t)((num + den / 2u) / den);
}

void gamecube_calibration_reset(GameCubeCalibration *cal) {
    if (!cal)
        return;

    for (unsigned i = 0; i < GC_AXIS_COUNT; i++) {
        cal->min_axis[i] = 40u;
        cal->max_axis[i] = 216u;
    }

    /* Trigger resting values are often around 40 in HID/PC mode. The same
     * defaults are harmless for Nintendo mode because observed lower values
     * immediately expand the range down toward zero. */
    cal->min_axis[GC_AXIS_LT] = 40u;
    cal->min_axis[GC_AXIS_RT] = 40u;
}

static uint8_t gamecube_calibrate_value(GameCubeCalibration *cal,
                                        unsigned axis, uint8_t raw) {
    if (raw < cal->min_axis[axis])
        cal->min_axis[axis] = raw;
    if (raw > cal->max_axis[axis])
        cal->max_axis[axis] = raw;

    return gamecube_remap_axis(
        raw, cal->min_axis[axis], cal->max_axis[axis]);
}

void gamecube_calibration_apply(GameCubeCalibration *cal, ScePadData *pad) {
    if (!cal || !pad)
        return;

    pad->leftStick.x = gamecube_calibrate_value(
        cal, GC_AXIS_LX, pad->leftStick.x);
    pad->leftStick.y = gamecube_calibrate_value(
        cal, GC_AXIS_LY, pad->leftStick.y);
    pad->rightStick.x = gamecube_calibrate_value(
        cal, GC_AXIS_RX, pad->rightStick.x);
    pad->rightStick.y = gamecube_calibrate_value(
        cal, GC_AXIS_RY, pad->rightStick.y);
    pad->analogButtons.l2 = gamecube_calibrate_value(
        cal, GC_AXIS_LT, pad->analogButtons.l2);
    pad->analogButtons.r2 = gamecube_calibrate_value(
        cal, GC_AXIS_RT, pad->analogButtons.r2);
}
