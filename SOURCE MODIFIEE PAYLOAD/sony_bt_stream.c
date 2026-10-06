#include "sony_bt_stream.h"
#include <string.h>

static uint16_t le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

void sony_bt_event_stream_init(SonyBtEventStream *s) {
    if (s) memset(s, 0, sizeof(*s));
}

void sony_bt_acl_stream_init(SonyBtAclStream *s) {
    if (s) memset(s, 0, sizeof(*s));
}

static void event_consume(SonyBtEventStream *s, uint32_t n) {
    if (n >= s->used) {
        s->used = 0;
        return;
    }
    memmove(s->buf, s->buf + n, s->used - n);
    s->used -= n;
}

static void acl_consume(SonyBtAclStream *s, uint32_t n) {
    if (n >= s->used) {
        s->used = 0;
        return;
    }
    memmove(s->buf, s->buf + n, s->used - n);
    s->used -= n;
}

void sony_bt_event_stream_feed(SonyBtEventStream *s,
                               const uint8_t *data, uint32_t len,
                               SonyBtPacketCallback cb, void *user) {
    if (!s || (!data && len))
        return;

    while (len) {
        uint32_t room = (uint32_t)sizeof(s->buf) - s->used;
        if (room == 0) {
            s->used = 0;
            room = (uint32_t)sizeof(s->buf);
        }

        uint32_t take = len < room ? len : room;
        memcpy(s->buf + s->used, data, take);
        s->used += take;
        data += take;
        len -= take;

        for (;;) {
            if (s->used < 2u)
                break;

            const uint32_t total = 2u + (uint32_t)s->buf[1];
            if (total > sizeof(s->buf)) {
                s->used = 0;
                break;
            }
            if (s->used < total)
                break;

            if (cb)
                cb(s->buf, total, user);
            event_consume(s, total);
        }
    }
}

void sony_bt_acl_stream_feed(SonyBtAclStream *s,
                             const uint8_t *data, uint32_t len,
                             SonyBtPacketCallback cb, void *user) {
    if (!s || (!data && len))
        return;

    while (len) {
        uint32_t room = (uint32_t)sizeof(s->buf) - s->used;
        if (room == 0) {
            s->used = 0;
            room = (uint32_t)sizeof(s->buf);
        }

        uint32_t take = len < room ? len : room;
        memcpy(s->buf + s->used, data, take);
        s->used += take;
        data += take;
        len -= take;

        for (;;) {
            if (s->used < 4u)
                break;

            const uint32_t total = 4u + (uint32_t)le16(&s->buf[2]);
            if (total > sizeof(s->buf)) {
                s->used = 0;
                break;
            }
            if (s->used < total)
                break;

            if (cb)
                cb(s->buf, total, user);
            acl_consume(s, total);
        }
    }
}
