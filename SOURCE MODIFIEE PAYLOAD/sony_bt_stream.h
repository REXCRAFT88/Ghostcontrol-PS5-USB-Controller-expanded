#pragma once
#include <stdint.h>

#define SONY_BT_HCI_EVENT_MAX 257u
#define SONY_BT_HCI_ACL_MAX   1028u

typedef void (*SonyBtPacketCallback)(const uint8_t *packet,
                                     uint32_t len,
                                     void *user);

typedef struct {
    uint8_t buf[SONY_BT_HCI_EVENT_MAX];
    uint32_t used;
} SonyBtEventStream;

typedef struct {
    uint8_t buf[SONY_BT_HCI_ACL_MAX];
    uint32_t used;
} SonyBtAclStream;

void sony_bt_event_stream_init(SonyBtEventStream *s);
void sony_bt_acl_stream_init(SonyBtAclStream *s);

/*
 * Feed arbitrary USB read fragments. Complete HCI packets are emitted through
 * the callback. Invalid/oversized headers reset only the affected stream.
 */
void sony_bt_event_stream_feed(SonyBtEventStream *s,
                               const uint8_t *data, uint32_t len,
                               SonyBtPacketCallback cb, void *user);

void sony_bt_acl_stream_feed(SonyBtAclStream *s,
                             const uint8_t *data, uint32_t len,
                             SonyBtPacketCallback cb, void *user);
