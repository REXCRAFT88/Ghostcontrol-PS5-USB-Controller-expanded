#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/controller_nintendo.h"
#include "../SOURCE MODIFIEE PAYLOAD/usb_helpers.h"

int usb_send_out(int fd, struct usb_fs_endpoint *ep,
                 const uint8_t *data, uint32_t len, const char *tag) {
    (void)fd; (void)ep; (void)data; (void)len; (void)tag;
    return 0;
}

int usb_send_cmd(int fd, struct usb_fs_endpoint *ep, uint8_t a, uint8_t b) {
    (void)fd; (void)ep; (void)a; (void)b;
    return 0;
}

static void pack_left_stick(uint8_t *b, uint16_t x, uint16_t y) {
    b[6] = (uint8_t)(x & 0xffu);
    b[7] = (uint8_t)(((x >> 8) & 0x0fu) | ((y & 0x0fu) << 4));
    b[8] = (uint8_t)((y >> 4) & 0xffu);
}

static void test_n64_full_report_buttons_and_c(void) {
    uint8_t r[64] = {0};
    ScePadData pad;

    r[0] = 0x30;
    r[3] = 0x08 | 0x04 | 0x40 | 0x02 | 0x01;
    /* A, B, R, C-left, C-up */
    r[4] = 0x08 | 0x02 | 0x10;
    /* ZR, Start, Home */
    r[5] = 0x80 | 0x40 | 0x02 | 0x04;
    /* Z, L, dpad up/right */
    pack_left_stick(r, 0, 4095);

    memset(&pad, 0, sizeof(pad));
    nintendo_parse_n64_0x30(r, &pad);

    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_CIRCLE);
    assert(pad.buttons & SCE_PAD_BUTTON_L2);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_R1);
    assert(pad.buttons & SCE_PAD_BUTTON_R2);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.buttons & SCE_PAD_BUTTON_UP);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);

    assert(pad.leftStick.x == 0);
    assert(pad.leftStick.y == 255);

    /* C-left + C-up -> upper-left virtual right stick. */
    assert(pad.rightStick.x == 0);
    assert(pad.rightStick.y == 0);
    assert(pad.analogButtons.l2 == 255);
    assert(pad.analogButtons.r2 == 255);
    assert(pad.connected == 1);
}

static void test_n64_c_right_and_down(void) {
    uint8_t r[64] = {0};
    ScePadData pad;

    r[0] = 0x30;
    r[3] = 0x80; /* C-down */
    r[4] = 0x01; /* C-right */
    pack_left_stick(r, 2048, 2048);

    nintendo_parse_n64_0x30(r, &pad);

    assert(pad.rightStick.x == 255);
    assert(pad.rightStick.y == 255);
    assert(pad.leftStick.x >= 127 && pad.leftStick.x <= 128);
    assert(pad.leftStick.y >= 127 && pad.leftStick.y <= 128);
}

static void test_n64_opposite_c_buttons_center(void) {
    uint8_t r[64] = {0};
    ScePadData pad;

    r[0] = 0x30;
    r[3] = 0x02 | 0x01 | 0x80; /* C-left + C-up + C-down */
    r[4] = 0x01;               /* C-right */
    pack_left_stick(r, 2048, 2048);

    nintendo_parse_n64_0x30(r, &pad);

    assert(pad.rightStick.x == 128);
    assert(pad.rightStick.y == 128);
}

static void test_n64_simple_report(void) {
    uint8_t r[16] = {0};
    ScePadData pad;

    r[0] = 0x3f;
    r[1] = 0x08 | 0x04 | 0x40; /* A + B + R */
    r[2] = 0x08 | 0x02;        /* ZR + Start */
    r[3] = 7;                  /* dpad up-left hat */
    r[4] = 25;
    r[5] = 230;
    r[8] = 0x80 | 0x40;        /* Z + L */

    nintendo_parse_n64_0x3f(r, &pad);

    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_CIRCLE);
    assert(pad.buttons & SCE_PAD_BUTTON_R1);
    assert(pad.buttons & SCE_PAD_BUTTON_R2);
    assert(pad.buttons & SCE_PAD_BUTTON_L2);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_UP);
    assert(pad.buttons & SCE_PAD_BUTTON_LEFT);
    assert(pad.leftStick.x == 25);
    assert(pad.leftStick.y == 230);
}

static void test_profile_handler_selects_n64(void) {
    uint8_t r[64] = {0};
    ScePadData pad;
    struct usb_fs_endpoint eps[2];
    int hs = HS_STREAMING;
    uint8_t seq = 1;

    memset(eps, 0, sizeof(eps));
    r[0] = 0x30;
    r[3] = 0x01; /* N64 C-up; standard profile would map this as Square/Y */
    pack_left_stick(r, 2048, 2048);

    assert(nintendo_handle_packet_profile(
        -1, eps, r, sizeof(r), &hs, &seq,
        NINTENDO_PROFILE_N64, &pad) == 1);

    assert(pad.rightStick.y == 0);
    assert((pad.buttons & SCE_PAD_BUTTON_SQUARE) == 0);
}

static void test_standard_profile_regression(void) {
    uint8_t r[64] = {0};
    ScePadData pad;
    struct usb_fs_endpoint eps[2];
    int hs = HS_STREAMING;
    uint8_t seq = 1;

    memset(eps, 0, sizeof(eps));
    r[0] = 0x30;
    r[3] = 0x04; /* standard Nintendo B -> Cross */
    pack_left_stick(r, 2048, 2048);

    assert(nintendo_handle_packet(
        -1, eps, r, sizeof(r), &hs, &seq, &pad) == 1);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
}

int main(void) {
    test_n64_full_report_buttons_and_c();
    test_n64_c_right_and_down();
    test_n64_opposite_c_buttons_center();
    test_n64_simple_report();
    test_profile_handler_selects_n64();
    test_standard_profile_regression();

    puts("Nintendo N64 profile tests passed");
    return 0;
}
