#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../SOURCE MODIFIEE PAYLOAD/sony_bt_stream.h"

typedef struct {
    uint8_t packet[8][SONY_BT_HCI_ACL_MAX];
    uint32_t len[8];
    unsigned count;
} Capture;

static void capture_cb(const uint8_t *packet, uint32_t len, void *user) {
    Capture *c = (Capture *)user;
    assert(c->count < 8);
    assert(len <= sizeof(c->packet[0]));
    memcpy(c->packet[c->count], packet, len);
    c->len[c->count] = len;
    c->count++;
}

static void test_event_fragmentation(void) {
    SonyBtEventStream s;
    Capture c = {0};
    sony_bt_event_stream_init(&s);

    const uint8_t ev[] = {0x0e, 0x04, 0x01, 0x05, 0x10, 0x00};
    sony_bt_event_stream_feed(&s, ev, 1, capture_cb, &c);
    assert(c.count == 0);
    sony_bt_event_stream_feed(&s, ev + 1, 2, capture_cb, &c);
    assert(c.count == 0);
    sony_bt_event_stream_feed(&s, ev + 3, 3, capture_cb, &c);
    assert(c.count == 1);
    assert(c.len[0] == sizeof(ev));
    assert(memcmp(c.packet[0], ev, sizeof(ev)) == 0);
}

static void test_event_coalescing(void) {
    SonyBtEventStream s;
    Capture c = {0};
    sony_bt_event_stream_init(&s);

    const uint8_t both[] = {
        0x0f, 0x02, 0xaa, 0xbb,
        0x05, 0x04, 0x01, 0x02, 0x03, 0x04
    };
    sony_bt_event_stream_feed(&s, both, sizeof(both), capture_cb, &c);
    assert(c.count == 2);
    assert(c.len[0] == 4);
    assert(c.len[1] == 6);
}

static void test_acl_fragmentation_and_coalescing(void) {
    SonyBtAclStream s;
    Capture c = {0};
    sony_bt_acl_stream_init(&s);

    const uint8_t p1[] = {
        0x23, 0x01, 0x05, 0x00,
        0x01, 0x00, 0x13, 0x00, 0xa1
    };
    const uint8_t p2[] = {
        0x42, 0x00, 0x06, 0x00,
        0x02, 0x00, 0x13, 0x00, 0xa1, 0x01
    };
    uint8_t joined[sizeof(p1) + sizeof(p2)];
    memcpy(joined, p1, sizeof(p1));
    memcpy(joined + sizeof(p1), p2, sizeof(p2));

    sony_bt_acl_stream_feed(&s, joined, 3, capture_cb, &c);
    assert(c.count == 0);
    sony_bt_acl_stream_feed(&s, joined + 3, sizeof(joined) - 3, capture_cb, &c);

    assert(c.count == 2);
    assert(c.len[0] == sizeof(p1));
    assert(c.len[1] == sizeof(p2));
    assert(memcmp(c.packet[0], p1, sizeof(p1)) == 0);
    assert(memcmp(c.packet[1], p2, sizeof(p2)) == 0);
}

static void test_acl_oversize_header_resets(void) {
    SonyBtAclStream s;
    Capture c = {0};
    sony_bt_acl_stream_init(&s);

    const uint8_t bad[] = {0x01, 0x00, 0xff, 0xff};
    sony_bt_acl_stream_feed(&s, bad, sizeof(bad), capture_cb, &c);
    assert(c.count == 0);
    assert(s.used == 0);

    const uint8_t good[] = {0x01, 0x00, 0x01, 0x00, 0xaa};
    sony_bt_acl_stream_feed(&s, good, sizeof(good), capture_cb, &c);
    assert(c.count == 1);
    assert(c.len[0] == sizeof(good));
}

int main(void) {
    test_event_fragmentation();
    test_event_coalescing();
    test_acl_fragmentation_and_coalescing();
    test_acl_oversize_header_resets();
    puts("Sony Bluetooth HCI stream tests passed");
    return 0;
}
