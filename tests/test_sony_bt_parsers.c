#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/controller_ds4.h"
#include "../SOURCE MODIFIEE PAYLOAD/controller_ds3.h"

static void assert_same_core_state(const ScePadData *a, const ScePadData *b) {
    assert(a->buttons == b->buttons);
    assert(a->leftStick.x == b->leftStick.x);
    assert(a->leftStick.y == b->leftStick.y);
    assert(a->rightStick.x == b->rightStick.x);
    assert(a->rightStick.y == b->rightStick.y);
    assert(a->analogButtons.l2 == b->analogButtons.l2);
    assert(a->analogButtons.r2 == b->analogButtons.r2);
    assert(a->connected == b->connected);
}

static void fill_common(uint8_t *p) {
    p[0] = 11;                     /* LX */
    p[1] = 22;                     /* LY */
    p[2] = 33;                     /* RX */
    p[3] = 44;                     /* RY */
    p[4] = 0x20 | 0x02;            /* Cross + dpad right */
    p[5] = 0x01 | 0x20 | 0x80;     /* L1 + Options + R3 */
    p[6] = 0x01 | 0x02;            /* PS + touchpad */
    p[7] = 77;                     /* L2 analog */
    p[8] = 88;                     /* R2 analog */
}

static void test_ds4_full_bt_matches_usb(void) {
    uint8_t usb[64] = {0};
    uint8_t bt[78] = {0};
    ScePadData usb_pad, bt_pad;

    usb[0] = 0x01;
    fill_common(&usb[1]);

    bt[0] = 0x11;
    bt[1] = 0xc0; /* transport flags/reserved */
    bt[2] = 0x00;
    fill_common(&bt[3]);

    assert(ds4_handle_packet(-1, NULL, usb, sizeof(usb), &usb_pad) == 1);
    assert(ds4_parse_bt_input(bt, sizeof(bt), &bt_pad) == 1);
    assert_same_core_state(&usb_pad, &bt_pad);
}

static void test_ds4_minimal_bt(void) {
    uint8_t bt[10] = {0};
    ScePadData pad;

    bt[0] = 0x01;
    fill_common(&bt[1]);

    assert(ds4_parse_bt_input(bt, sizeof(bt), &pad) == 1);
    assert(pad.leftStick.x == 11);
    assert(pad.rightStick.y == 44);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.buttons & SCE_PAD_BUTTON_TOUCH_PAD);
    assert(pad.analogButtons.l2 == 77);
    assert(pad.analogButtons.r2 == 88);
}

static void test_ds4_rejects_invalid_bt(void) {
    uint8_t short_full[77] = {0};
    ScePadData pad;
    short_full[0] = 0x11;
    assert(ds4_parse_bt_input(short_full, sizeof(short_full), &pad) == 0);

    uint8_t unknown[10] = {0};
    unknown[0] = 0x31;
    assert(ds4_parse_bt_input(unknown, sizeof(unknown), &pad) == 0);
}

static void test_ds3_bt_state_and_bogus_filter(void) {
    uint8_t report[49] = {0};
    ScePadData pad;

    report[0] = 0x01;
    report[2] = 0x08 | 0x10;        /* Start + up */
    report[3] = 0x40 | 0x04;        /* Cross + L1 */
    report[4] = 0x01;               /* PS */
    report[6] = 101;
    report[7] = 102;
    report[8] = 103;
    report[9] = 104;

    assert(ds3_parse_bt_input(report, sizeof(report), &pad) == 1);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_UP);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.leftStick.x == 101);
    assert(pad.rightStick.y == 104);

    report[1] = 0xff;
    assert(ds3_parse_bt_input(report, sizeof(report), &pad) == 0);
}

int main(void) {
    test_ds4_full_bt_matches_usb();
    test_ds4_minimal_bt();
    test_ds4_rejects_invalid_bt();
    test_ds3_bt_state_and_bogus_filter();
    puts("Sony Bluetooth parser tests passed");
    return 0;
}
