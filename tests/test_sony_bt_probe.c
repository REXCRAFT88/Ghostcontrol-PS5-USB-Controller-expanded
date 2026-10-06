#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/sony_bt_probe.h"

static size_t put_interface(uint8_t *b, size_t o, uint8_t iface, uint8_t alt,
                            uint8_t klass, uint8_t sub, uint8_t proto) {
    const uint8_t d[9] = {9, 4, iface, alt, 3, klass, sub, proto, 0};
    memcpy(&b[o], d, sizeof(d));
    return o + sizeof(d);
}

static size_t put_ep(uint8_t *b, size_t o, uint8_t addr,
                     uint8_t attr, uint16_t mps) {
    const uint8_t d[7] = {
        7, 5, addr, attr,
        (uint8_t)(mps & 0xff), (uint8_t)(mps >> 8), 1
    };
    memcpy(&b[o], d, sizeof(d));
    return o + sizeof(d);
}

static void test_two_hci_functions_and_voice_filter(void) {
    uint8_t cfg[160] = {0};
    size_t o = 0;
    SonyBtHciFunction f[4];

    /* Configuration descriptor header; parser safely ignores it. */
    const uint8_t ch[9] = {9, 2, 0, 0, 4, 1, 0, 0x80, 50};
    memcpy(&cfg[o], ch, sizeof(ch));
    o += sizeof(ch);

    /* HCI function 0: events 0x81, ACL in 0x82, ACL out 0x01. */
    o = put_interface(cfg, o, 0, 0, 0xe0, 0x01, 0x01);
    o = put_ep(cfg, o, 0x81, 3, 16);
    o = put_ep(cfg, o, 0x82, 2, 1024);
    o = put_ep(cfg, o, 0x01, 2, 1024);

    /* Voice-style interface: same broad class but isochronous only -> reject. */
    o = put_interface(cfg, o, 1, 0, 0xe0, 0x01, 0x01);
    o = put_ep(cfg, o, 0x83, 1, 64);
    o = put_ep(cfg, o, 0x03, 1, 64);

    /* HCI function 2 with different endpoint addresses. */
    o = put_interface(cfg, o, 2, 0, 0xe0, 0x01, 0x01);
    o = put_ep(cfg, o, 0x84, 3, 32);
    o = put_ep(cfg, o, 0x85, 2, 512);
    o = put_ep(cfg, o, 0x04, 2, 512);

    memset(f, 0, sizeof(f));
    int n = sony_bt_find_hci_functions(cfg, (uint32_t)o, f, 4);
    assert(n == 2);

    assert(f[0].iface == 0);
    assert(f[0].ep_events == 0x81);
    assert(f[0].ep_acl_in == 0x82);
    assert(f[0].ep_acl_out == 0x01);
    assert(f[0].mps_events == 16);
    assert(f[0].mps_acl_in == 1024);

    assert(f[1].iface == 2);
    assert(f[1].ep_events == 0x84);
    assert(f[1].ep_acl_in == 0x85);
    assert(f[1].ep_acl_out == 0x04);
}

static void test_alt_setting_and_non_bt_are_ignored(void) {
    uint8_t cfg[80] = {0};
    size_t o = 0;
    SonyBtHciFunction f[4];

    /* Bluetooth shape, but alternate setting 1 -> ignore. */
    o = put_interface(cfg, o, 3, 1, 0xe0, 0x01, 0x01);
    o = put_ep(cfg, o, 0x81, 3, 16);
    o = put_ep(cfg, o, 0x82, 2, 64);
    o = put_ep(cfg, o, 0x01, 2, 64);

    /* HID interface, not Bluetooth HCI -> ignore. */
    o = put_interface(cfg, o, 4, 0, 0x03, 0x00, 0x00);
    o = put_ep(cfg, o, 0x83, 3, 64);

    assert(sony_bt_find_hci_functions(cfg, (uint32_t)o, f, 4) == 0);
}

static void test_malformed_descriptor_stops_safely(void) {
    uint8_t cfg[16] = {9, 4, 0, 0, 3, 0xe0, 1, 1, 0,
                       20, 5, 0x81, 3, 16, 0, 1};
    SonyBtHciFunction f[4];
    assert(sony_bt_find_hci_functions(cfg, sizeof(cfg), f, 4) == 0);
}

static void test_output_capacity(void) {
    uint8_t cfg[160] = {0};
    size_t o = 0;
    SonyBtHciFunction f[1];

    for (int iface = 0; iface < 2; iface++) {
        o = put_interface(cfg, o, (uint8_t)iface, 0, 0xe0, 1, 1);
        o = put_ep(cfg, o, (uint8_t)(0x81 + iface * 2), 3, 16);
        o = put_ep(cfg, o, (uint8_t)(0x82 + iface * 2), 2, 64);
        o = put_ep(cfg, o, (uint8_t)(0x01 + iface), 2, 64);
    }

    assert(sony_bt_find_hci_functions(cfg, (uint32_t)o, f, 1) == 1);
}

int main(void) {
    test_two_hci_functions_and_voice_filter();
    test_alt_setting_and_non_bt_are_ignored();
    test_malformed_descriptor_stops_safely();
    test_output_capacity();
    puts("Sony Bluetooth radio descriptor tests passed");
    return 0;
}
