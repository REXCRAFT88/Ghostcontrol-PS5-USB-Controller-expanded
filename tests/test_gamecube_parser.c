#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/controller_gamecube.h"
#include "../SOURCE MODIFIEE PAYLOAD/usb_helpers.h"

static uint8_t g_last_out[64];
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
    memset(g_last_out, 0, sizeof(g_last_out));
    if (data && len) {
        uint32_t n = len < sizeof(g_last_out) ? len : (uint32_t)sizeof(g_last_out);
        memcpy(g_last_out, data, n);
    }
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

static void test_nintendo_rumble_packet(void) {
    struct usb_fs_endpoint ep[2];
    uint8_t state[GAMECUBE_ADAPTER_PORTS] = {0, 1, 7, 0};
    memset(ep, 0, sizeof(ep));
    g_out_calls = 0;
    g_last_out_len = 0;
    memset(g_last_out, 0, sizeof(g_last_out));

    assert(gamecube_send_nintendo_rumble(-1, ep, state) == 0);
    assert(g_out_calls == 1);
    assert(g_last_out_len == 5);
    assert(g_last_out[0] == GAMECUBE_ADAPTER_RUMBLE_CMD);
    assert(g_last_out[1] == 0);
    assert(g_last_out[2] == 1);
    assert(g_last_out[3] == 1);
    assert(g_last_out[4] == 0);
}

static void test_feedback_rumble_parse(void) {
    uint8_t feedback[17] = {0};

    assert(gamecube_feedback_wants_rumble(NULL, 0) == 0);
    assert(gamecube_feedback_wants_rumble(feedback, 4) == 0);
    assert(gamecube_feedback_wants_rumble(feedback, sizeof(feedback)) == 0);

    feedback[3] = 1;
    assert(gamecube_feedback_wants_rumble(feedback, sizeof(feedback)) == 1);
    feedback[3] = 0;
    feedback[4] = 0xff;
    assert(gamecube_feedback_wants_rumble(feedback, sizeof(feedback)) == 1);
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

static void test_pc_mode_legacy_report(void) {
    uint8_t r[10] = {0};
    ScePadData pad;
    unsigned port = 99;

    /* Legacy packet: byte 0 is 1-based controller slot. */
    r[0] = 3; /* physical adapter port 3 -> zero-based port 2 */
    r[1] = 0x02 | 0x01 | 0x80 | 0x20 | 0x10; /* A + X + Z + R/L clicks */
    r[2] = 0x20 | 0x10 | 0x02; /* right + up + Start */
    r[3] = 10;  /* LX */
    r[4] = 20;  /* LY */
    r[5] = 30;  /* C-stick Y in legacy orientation */
    r[6] = 40;  /* C-stick X in legacy orientation */
    r[7] = 77;  /* L analog */
    r[8] = 88;  /* R analog */
    r[9] = 0;

    memset(&pad, 0, sizeof(pad));
    assert(gamecube_parse_pc_packet(r, sizeof(r), &port, &pad) == 1);
    assert(port == 2);
    assert(pad.buttons & SCE_PAD_BUTTON_CROSS);
    assert(pad.buttons & SCE_PAD_BUTTON_SQUARE);
    assert(pad.buttons & SCE_PAD_BUTTON_R1);
    assert(pad.buttons & SCE_PAD_BUTTON_R2);
    assert(pad.buttons & SCE_PAD_BUTTON_L2);
    assert(pad.buttons & SCE_PAD_BUTTON_RIGHT);
    assert(pad.buttons & SCE_PAD_BUTTON_UP);
    assert(pad.buttons & SCE_PAD_BUTTON_OPTIONS);
    assert(pad.leftStick.x == 10);
    assert(pad.leftStick.y == 235);
    assert(pad.rightStick.x == 215); /* 255 - p[5] */
    assert(pad.rightStick.y == 30);  /* p[4] */
    assert(pad.analogButtons.l2 == 77);
    assert(pad.analogButtons.r2 == 88);
    assert(pad.connected == 1);
}

static void test_pc_mode_v7_report(void) {
    uint8_t r[9] = {0};
    ScePadData pad;
    unsigned port = 99;

    r[0] = 0x04 | 0x08; /* B + Y */
    r[1] = 0x80 | 0x40; /* left + down */
    r[2] = 50;
    r[3] = 60;
    r[4] = 70;
    r[5] = 80;
    r[6] = 90;
    r[7] = 100;

    memset(&pad, 0, sizeof(pad));
    assert(gamecube_parse_pc_packet(r, sizeof(r), &port, &pad) == 1);
    assert(port == 0);
    assert(pad.buttons & SCE_PAD_BUTTON_CIRCLE);
    assert(pad.buttons & SCE_PAD_BUTTON_TRIANGLE);
    assert(pad.buttons & SCE_PAD_BUTTON_LEFT);
    assert(pad.buttons & SCE_PAD_BUTTON_DOWN);
    assert(pad.leftStick.x == 50);
    assert(pad.leftStick.y == 195);
    assert(pad.rightStick.x == 80);
    assert(pad.rightStick.y == 185);
    assert(pad.analogButtons.l2 == 90);
    assert(pad.analogButtons.r2 == 100);
}

static void test_pc_mode_rejections(void) {
    uint8_t bad_slot[10] = {0};
    uint8_t bad_len[8] = {0};
    ScePadData pad;
    unsigned port = 0;

    bad_slot[0] = 0;
    assert(gamecube_parse_pc_packet(
        bad_slot, sizeof(bad_slot), &port, &pad) == 0);

    bad_slot[0] = 5;
    assert(gamecube_parse_pc_packet(
        bad_slot, sizeof(bad_slot), &port, &pad) == 0);

    assert(gamecube_parse_pc_packet(
        bad_len, sizeof(bad_len), &port, &pad) == 0);
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
    test_nintendo_rumble_packet();
    test_feedback_rumble_parse();
    assert(gamecube_is_adapter(0x057e, 0x0337));
    assert(gamecube_is_nintendo_adapter(0x057e, 0x0337));
    assert(gamecube_is_pc_adapter(0x0079, 0x1843));
    assert(gamecube_is_pc_adapter(0x0079, 0x1844));
    assert(gamecube_is_pc_adapter(0x0079, 0x1846));
    assert(gamecube_is_adapter(0x0079, 0x1843));
    assert(!gamecube_is_pc_adapter(0x0079, 0x1847));
    assert(!gamecube_is_adapter(0x057e, 0x2009));

    test_pc_mode_legacy_report();
    test_pc_mode_v7_report();
    test_pc_mode_rejections();
    test_port_zero_mapping();
    test_all_four_ports_are_independent();
    test_empty_port_and_invalid_report();

    puts("GameCube adapter parser tests passed");
    return 0;
}
