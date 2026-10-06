#include "controller_ds3.h"
#include <string.h>
#include <sys/ioctl.h>

#ifdef __PROSPERO__
#include <dev/usb/usb_endian.h>
#endif

int ds3_is_supported_vidpid(uint16_t vid, uint16_t pid) {
    return vid == DS3_VID && pid == DS3_PID;
}

const char *ds3_name(void) {
    return "Sony DualShock 3 / Sixaxis";
}

int ds3_send_init(int fd) {
#ifdef __PROSPERO__
    if (fd < 0) return -1;

    /*
     * DS3 USB wake-up / operational-mode report.
     * Public PS3 HID implementations use this SET_REPORT before polling input.
     */
    uint8_t init_report[4] = { 0x42, 0x03, 0x00, 0x00 };
    struct usb_ctl_request req;
    memset(&req, 0, sizeof(req));

    req.ucr_request.bmRequestType = 0x21; /* host->device, class, interface */
    req.ucr_request.bRequest = 0x09;      /* SET_REPORT */
    USETW(req.ucr_request.wValue, 0x03f4);
    USETW(req.ucr_request.wIndex, 0x0000);
    USETW(req.ucr_request.wLength, sizeof(init_report));
    req.ucr_data = init_report;

    return ioctl(fd, USB_DO_REQUEST, &req);
#else
    (void)fd;
    return 0;
#endif
}

int ds3_parse_input(const uint8_t *b, uint32_t len, ScePadData *o) {
    if (!b || !o || len < 10)
        return 0;

    /*
     * Standard DS3 USB input reports normally use report ID 0x01.
     * Some compatible devices may present the same payload without exposing
     * the report ID through the userspace transport, so accept either shape
     * as long as the core offsets are present.
     */
    memset(o, 0, sizeof(*o));

    const uint8_t b1 = b[2];
    const uint8_t b2 = b[3];

    if (b1 & 0x01u) o->buttons |= SCE_PAD_BUTTON_SHARE;   /* Select */
    if (b1 & 0x02u) o->buttons |= SCE_PAD_BUTTON_L3;
    if (b1 & 0x04u) o->buttons |= SCE_PAD_BUTTON_R3;
    if (b1 & 0x08u) o->buttons |= SCE_PAD_BUTTON_OPTIONS; /* Start */
    if (b1 & 0x10u) o->buttons |= SCE_PAD_BUTTON_UP;
    if (b1 & 0x20u) o->buttons |= SCE_PAD_BUTTON_RIGHT;
    if (b1 & 0x40u) o->buttons |= SCE_PAD_BUTTON_DOWN;
    if (b1 & 0x80u) o->buttons |= SCE_PAD_BUTTON_LEFT;

    if (b2 & 0x01u) { o->buttons |= SCE_PAD_BUTTON_L2; o->analogButtons.l2 = 255; }
    if (b2 & 0x02u) { o->buttons |= SCE_PAD_BUTTON_R2; o->analogButtons.r2 = 255; }
    if (b2 & 0x04u) o->buttons |= SCE_PAD_BUTTON_L1;
    if (b2 & 0x08u) o->buttons |= SCE_PAD_BUTTON_R1;
    if (b2 & 0x10u) o->buttons |= SCE_PAD_BUTTON_TRIANGLE;
    if (b2 & 0x20u) o->buttons |= SCE_PAD_BUTTON_CIRCLE;
    if (b2 & 0x40u) o->buttons |= SCE_PAD_BUTTON_CROSS;
    if (b2 & 0x80u) o->buttons |= SCE_PAD_BUTTON_SQUARE;

    if (len > 4 && (b[4] & 0x01u))
        o->buttons |= SCE_PAD_BUTTON_PS;

    o->leftStick.x  = b[6];
    o->leftStick.y  = b[7];
    o->rightStick.x = b[8];
    o->rightStick.y = b[9];

    /*
     * Pressure-sensitive DS3 buttons live later in the report. Keep the
     * first implementation conservative: digital trigger bits above are
     * always correct, and full pressure preservation can be enabled after
     * hardware/report validation.
     */
    o->connected = 1;
    o->quat.w = 1.0f;
    return 1;
}

int ds3_handle_packet(int fd, struct usb_fs_endpoint *eps,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out_pad) {
    (void)fd;
    (void)eps;
    return ds3_parse_input(buf, len, out_pad);
}
