#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "controller_generic_hid.h"

static void test_hori(void) {
    uint8_t b[7] = {0};
    ScePadData p;
    b[0] = 0x04; /* A -> Cross */
    b[1] = 0x02; /* Start -> Options */
    b[2] = 0x02; /* Right */
    b[3] = 10; b[4] = 20; b[5] = 30; b[6] = 40;
    assert(generic_hid_parse(0x0f0d,0x00c1,b,sizeof(b),&p)==1);
    assert(p.buttons & SCE_PAD_BUTTON_CROSS);
    assert(p.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(p.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(p.leftStick.x==10 && p.leftStick.y==20);
    assert(p.rightStick.x==30 && p.rightStick.y==40);
    assert(p.connected==1);
}

static void test_ps2(void) {
    uint8_t b[8] = {1, 11,22,33,44, 0x22, 0x21, 0};
    ScePadData p;
    assert(generic_hid_parse(0x0810,0x0003,b,sizeof(b),&p)==1);
    assert(p.leftStick.x==11 && p.leftStick.y==22);
    assert(p.rightStick.x==33 && p.rightStick.y==44);
    assert(p.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(p.buttons & SCE_PAD_BUTTON_CROSS);
    assert(p.buttons & SCE_PAD_BUTTON_L1);
    assert(p.buttons & SCE_PAD_BUTTON_OPTIONS);
}

int main(void) {
    assert(generic_hid_is_supported(0x0810,0x0003));
    assert(generic_hid_is_supported(0x0f0d,0x00c1));
    test_hori();
    test_ps2();
    return 0;
}
