#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/controller_ds4.h"
#include "../SOURCE MODIFIEE PAYLOAD/controller_ds3.h"

static void test_ds4(void) {
    uint8_t r[64];
    ScePadData pad;
    memset(r, 0, sizeof(r));
    memset(&pad, 0, sizeof(pad));

    r[0] = 0x01;
    r[1] = 10;
    r[2] = 20;
    r[3] = 30;
    r[4] = 40;
    r[5] = 0x20 | 0x02;
    r[6] = 0x21;
    r[7] = 0x03;
    r[8] = 77;
    r[9] = 88;

    assert(ds4_handle_packet(-1, NULL, r, sizeof(r), &pad) == 1);
    assert(pad.leftStick.x == 10);
    assert(pad.leftStick.y == 20);
    assert(pad.rightStick.x == 30);
    assert(pad.rightStick.y == 40);
    assert(pad.analogButtons.l2 == 77);
    assert(pad.analogButtons.r2 == 88);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.buttons & SCE_PAD_BUTTON_TOUCH_PAD);
    assert(pad.connected == 1);
}

static void test_ds3(void) {
    uint8_t r[49];
    ScePadData pad;
    memset(r, 0, sizeof(r));
    memset(&pad, 0, sizeof(pad));

    r[0] = 0x01;
    r[2] = 0x08 | 0x10;
    r[3] = 0x40 | 0x04 | 0x01;
    r[4] = 0x01;
    r[6] = 100;
    r[7] = 110;
    r[8] = 120;
    r[9] = 130;

    assert(ds3_handle_packet(-1, NULL, r, sizeof(r), &pad) == 1);
    assert(pad.leftStick.x == 100);
    assert(pad.leftStick.y == 110);
    assert(pad.rightStick.x == 120);
    assert(pad.rightStick.y == 130);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_UP);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_L2);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.analogButtons.l2 == 255);
    assert(pad.connected == 1);
}

static void test_device_matching(void) {
    assert(ds4_is_supported_vidpid(0x054c, 0x05c4));
    assert(ds4_is_supported_vidpid(0x054c, 0x09cc));
    assert(!ds4_is_supported_vidpid(0x054c, 0x0268));
    assert(ds3_is_supported_vidpid(0x054c, 0x0268));
    assert(!ds3_is_supported_vidpid(0x054c, 0x05c4));
}

int main(void) {
    test_device_matching();
    test_ds4();
    test_ds3();
    puts("Sony controller parser tests passed");
    return 0;
}
