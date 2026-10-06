#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/sony_bt_transport.h"
#include "../SOURCE MODIFIEE PAYLOAD/controller_ds4.h"
#include "../SOURCE MODIFIEE PAYLOAD/controller_ds3.h"

static uint32_t make_acl(uint8_t *dst, uint16_t handle,
                         const uint8_t *report, uint16_t report_len) {
    const uint16_t l2cap_len = (uint16_t)(1u + report_len);
    const uint16_t acl_len = (uint16_t)(4u + l2cap_len);

    memset(dst, 0, 128);
    dst[0] = (uint8_t)(handle & 0xff);
    dst[1] = (uint8_t)((handle >> 8) & 0x0f);
    dst[2] = (uint8_t)(acl_len & 0xff);
    dst[3] = (uint8_t)(acl_len >> 8);
    dst[4] = (uint8_t)(l2cap_len & 0xff);
    dst[5] = (uint8_t)(l2cap_len >> 8);
    dst[6] = 0x13;
    dst[7] = 0x00;
    dst[8] = 0xA1;
    memcpy(&dst[9], report, report_len);
    return (uint32_t)(9u + report_len);
}

static void test_ds4_acl_to_parser(void) {
    uint8_t report[78] = {0};
    uint8_t acl[128];
    SonyBtHidInputView view;
    ScePadData pad;

    report[0] = 0x11;
    report[3] = 10;
    report[4] = 20;
    report[5] = 30;
    report[6] = 40;
    report[7] = 0x20 | 0x02; /* Cross + right */
    report[8] = 0x01;        /* L1 */
    report[9] = 0x01;        /* PS */
    report[10] = 55;
    report[11] = 66;

    uint32_t acl_len = make_acl(acl, 0x0123, report, sizeof(report));
    assert(sony_bt_extract_hid_input(acl, acl_len, &view) == 1);
    assert(view.connection_handle == 0x0123);
    assert(view.cid == BT_L2CAP_CID_HID_INTERRUPT);
    assert(view.report_len == sizeof(report));
    assert(ds4_parse_bt_input(view.report, view.report_len, &pad) == 1);
    assert(pad.leftStick.x == 10);
    assert(pad.rightStick.y == 40);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(pad.buttons & SCE_PAD_BUTTON_L1);
    assert(pad.buttons & SCE_PAD_BUTTON_PS);
    assert(pad.analogButtons.l2 == 55);
    assert(pad.analogButtons.r2 == 66);
}

static void test_ds3_acl_to_parser(void) {
    uint8_t report[49] = {0};
    uint8_t acl[128];
    SonyBtHidInputView view;
    ScePadData pad;

    report[0] = 0x01;
    report[2] = 0x08;
    report[3] = 0x40;
    report[6] = 121;
    report[7] = 122;
    report[8] = 123;
    report[9] = 124;

    uint32_t acl_len = make_acl(acl, 0x0042, report, sizeof(report));
    assert(sony_bt_extract_hid_input(acl, acl_len, &view) == 1);
    assert(ds3_parse_bt_input(view.report, view.report_len, &pad) == 1);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.leftStick.x == 121);
    assert(pad.rightStick.y == 124);
}

static void test_rejections(void) {
    uint8_t report[10] = {0x01};
    uint8_t acl[128];
    SonyBtHidInputView view;
    uint32_t len = make_acl(acl, 7, report, sizeof(report));

    /* Wrong L2CAP CID. */
    acl[6] = 0x11;
    assert(sony_bt_extract_hid_input(acl, len, &view) == 0);
    acl[6] = 0x13;

    /* Wrong HIDP transaction type. */
    acl[8] = 0xA2;
    assert(sony_bt_extract_hid_input(acl, len, &view) == 0);
    acl[8] = 0xA1;

    /* Truncated ACL packet. */
    assert(sony_bt_extract_hid_input(acl, len - 1, &view) == 0);

    /* L2CAP length claims more data than ACL carries. */
    acl[4] = 0xff;
    acl[5] = 0x00;
    assert(sony_bt_extract_hid_input(acl, len, &view) == 0);
}

int main(void) {
    test_ds4_acl_to_parser();
    test_ds3_acl_to_parser();
    test_rejections();
    puts("Sony Bluetooth transport tests passed");
    return 0;
}
