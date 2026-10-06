#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/controller_gamecube.h"
#include "../SOURCE MODIFIEE PAYLOAD/usb_helpers.h"

static uint8_t g_last_out_byte;
static uint32_t g_last_out_len;
static int g_out_calls;

/* Capture adapter initialization output without needing PS5 USB hardware. */
int usb_send_out(int fd, struct usb_fs_endpoint *ep,
                 const uint8_t *data, uint32_t len, const char *tag) {
    (void)fd; (void)ep; (void)tag;
    g_out_calls++;
    g_last_out_len = len;
    g_last_out_byte = (data && len) ? data[0] : 0;
    return 0;
}

static void test_adapter_init_command(void) {
    struct usb_fs_endpoint ep[2];
    memset(ep, 0, sizeof(ep));
    g_out_calls = 0;
    g_last_out_len = 0;
    g_last_out_byte = 0;

    assert(gamecube_send_init(-1, ep) == 0);
    assert(g_out_calls == 1);
    assert(g_last_out_len == 1);
    assert(g_last_out_byte == GAMECUBE_ADAPTER_INIT_CMD);
}

static void fill_port(uint8_t *report, unsigned port,
                      uint8_t status, uint8_t b1, uint8_t b2,
                      uint8_t lx, uint8_t ly, uint8_t cx, uint8_t cy,
                      uint8_t lt, uint8_t rt) {
    uint8_t *p = &report[1 + 9 * port];
    p[0] = status;
    p[1] = b1;
    p[2] = b2;
    p[3] = lx;
    p[4] = ly;
    p[5] = cx;
    p[6] = cy;
    p[7] = lt;
    p[8] = rt;
}

static void test_port_zero_mapping(void) {
    uint8_t r[GAMECUBE_ADAPTER_REPORT_SIZE] = {0};
    ScePadData pad;
    r[0] = GAMECUBE_ADAPTER_REPORT_ID;

    fill_port(r, 0, 0x10,
              0x01 | 0x20, /* A + D-right */
              0x01 | 0x02 | 0x08, /* Start + Z + L hard click */
              10, 20, 30, 40, 77, 88);

    memset(&pad, 0, sizeof(pad));
    assert(gamecube_parse_port(r, sizeof(r), 0, &pad) == 1);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.buttons & SCE_PAD_BUTTON_R1);
    assert(pad.buttons & SCE_PAD_BUTTON_L2);
    assert(pad.leftStick.x == 10);
    assert(pad.leftStick.y == 235);
    assert(pad.rightStick.x == 30);
    assert(pad.rightStick.y == 215);
    assert(pad.analogButtons.l2 == 77);
    assert(pad.analogButtons.r2 == 88);
    assert(pad.connected == 1);
}

static void test_all_four_ports_are_independent(void) {
    uint8_t r[GAMECUBE_ADAPTER_REPORT_SIZE] = {0};
    ScePadData pad;
    r[0] = GAMECUBE_ADAPTER_REPORT_ID;

    fill_port(r, 0, 0x10, 0x01, 0, 1, 128, 2, 128, 3, 4);
    fill_port(r, 1, 0x10, 0x02, 0, 11, 128, 12, 128, 13, 14);
    fill_port(r, 2, 0x20, 0x04, 0, 21, 128, 22, 128, 23, 24);
    fill_port(r, 3, 0x10, 0x08, 0, 31, 128, 32, 128, 33, 34);

    assert(gamecube_parse_port(r, sizeof(r), 0, &pad) == 1);
    assert((pad.buttons & SCE_PAD_BUTTON_CROSS) && pad.leftStick.x == 1);

    assert(gamecube_parse_port(r, sizeof(r), 1, &pad) == 1);
    assert((pad.buttons & SCE_PAD_BUTTON_CIRCLE) && pad.leftStick.x == 11);

    assert(gamecube_parse_port(r, sizeof(r), 2, &pad) == 1);
    assert((pad.buttons & SCE_PAD_BUTTON_SQUARE) && pad.leftStick.x == 21);

    assert(gamecube_parse_port(r, sizeof(r), 3, &pad) == 1);
    assert((pad.buttons & SCE_PAD_BUTTON_TRIANGLE) && pad.leftStick.x == 31);
}

static void test_empty_port_and_invalid_report(void) {
    uint8_t r[GAMECUBE_ADAPTER_REPORT_SIZE] = {0};
    ScePadData pad;
    r[0] = GAMECUBE_ADAPTER_REPORT_ID;

    assert(gamecube_parse_port(r, sizeof(r), 0, &pad) == 0);
    assert(gamecube_parse_port(r, sizeof(r), 4, &pad) == 0);

    r[0] = 0x00;
    fill_port(r, 0, 0x10, 0x01, 0, 128, 128, 128, 128, 0, 0);
    assert(gamecube_parse_port(r, sizeof(r), 0, &pad) == 0);
}

int main(void) {
    test_adapter_init_command();
    assert(gamecube_is_adapter(0x057e, 0x0337));
    assert(!gamecube_is_adapter(0x057e, 0x2009));

    test_port_zero_mapping();
    test_all_four_ports_are_independent();
    test_empty_port_and_invalid_report();

    puts("GameCube adapter parser tests passed");
    return 0;
}
