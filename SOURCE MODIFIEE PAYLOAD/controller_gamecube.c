#include "controller_gamecube.h"
#include "usb_helpers.h"
#include <string.h>
#include <sys/ioctl.h>

#ifdef __PROSPERO__
#include <dev/usb/usb_endian.h>
#endif

int gamecube_is_adapter(uint16_t vid, uint16_t pid) {
    return vid == GAMECUBE_ADAPTER_VID && pid == GAMECUBE_ADAPTER_PID;
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
