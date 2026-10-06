#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif

#include "sony_bt_passive.h"
#include "sony_bt_stream.h"

#ifndef __PROSPERO__

int sony_bt_passive_observe(const SonyBtRadioProbe *radio,
                            unsigned hci_index,
                            unsigned duration_ms,
                            SonyBtObservedPacket cb,
                            void *user) {
    (void)radio; (void)hci_index; (void)duration_ms; (void)cb; (void)user;
    return -1;
}

#else

#include <dev/usb/usb.h>
#include <dev/usb/usb_ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

#define PASSIVE_XFERS 2
#define XFER_EVENT    0
#define XFER_ACL      1
#define EVENT_BUF     64
#define ACL_BUF       1024

typedef struct {
    SonyBtObservedPacket cb;
    void *user;
    int is_acl;
} EmitCtx;

static void emit_packet(const uint8_t *packet, uint32_t len, void *user) {
    EmitCtx *e = (EmitCtx *)user;
    if (e->cb)
        e->cb(e->is_acl, packet, len, e->user);
}

static uint64_t now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000u + (uint64_t)tv.tv_usec / 1000u;
}

static int start_xfer(int fd, struct usb_fs_endpoint *eps,
                      uint32_t *lengths, int index, uint32_t len) {
    struct usb_fs_start st;
    memset(&st, 0, sizeof(st));
    lengths[index] = len;
    eps[index].nFrames = 1;
    eps[index].aFrames = 0;
    eps[index].status = 0;
    st.ep_index = (uint8_t)index;
    return ioctl(fd, USB_FS_START, &st);
}

int sony_bt_passive_observe(const SonyBtRadioProbe *radio,
                            unsigned hci_index,
                            unsigned duration_ms,
                            SonyBtObservedPacket cb,
                            void *user) {
    if (!radio || hci_index >= (unsigned)radio->hci_count || duration_ms == 0)
        return -1;

    const SonyBtHciFunction *f = &radio->hci[hci_index];
    int fd = open(radio->dev_path, O_RDWR | O_NONBLOCK);
    if (fd < 0)
        return -2;

    struct usb_fs_endpoint eps[PASSIVE_XFERS];
    struct usb_fs_init init;
    struct usb_fs_open op;
    void *buffers[PASSIVE_XFERS][1];
    uint32_t lengths[PASSIVE_XFERS];
    uint8_t event_buf[EVENT_BUF];
    uint8_t acl_buf[ACL_BUF];
    int opened_event = 0, opened_acl = 0;

    memset(eps, 0, sizeof(eps));
    memset(&init, 0, sizeof(init));
    memset(lengths, 0, sizeof(lengths));

    buffers[XFER_EVENT][0] = event_buf;
    buffers[XFER_ACL][0] = acl_buf;

    for (int i = 0; i < PASSIVE_XFERS; i++) {
        eps[i].ppBuffer = buffers[i];
        eps[i].pLength = &lengths[i];
        eps[i].nFrames = 1;
        eps[i].flags = USB_FS_FLAG_SINGLE_SHORT_OK;
    }

    init.pEndpoints = eps;
    init.ep_index_max = PASSIVE_XFERS;
    if (ioctl(fd, USB_FS_INIT, &init) != 0) {
        close(fd);
        return -3;
    }

    memset(&op, 0, sizeof(op));
    op.ep_index = XFER_EVENT;
    op.ep_no = f->ep_events;
    op.max_bufsize = EVENT_BUF;
    op.max_frames = 1;
    if (ioctl(fd, USB_FS_OPEN, &op) != 0)
        goto fail;
    opened_event = 1;

    memset(&op, 0, sizeof(op));
    op.ep_index = XFER_ACL;
    op.ep_no = f->ep_acl_in;
    op.max_bufsize = ACL_BUF;
    op.max_frames = 1;
    if (ioctl(fd, USB_FS_OPEN, &op) != 0)
        goto fail;
    opened_acl = 1;

    SonyBtEventStream event_stream;
    SonyBtAclStream acl_stream;
    sony_bt_event_stream_init(&event_stream);
    sony_bt_acl_stream_init(&acl_stream);

    if (start_xfer(fd, eps, lengths, XFER_EVENT, EVENT_BUF) != 0)
        goto fail;
    if (start_xfer(fd, eps, lengths, XFER_ACL, ACL_BUF) != 0)
        goto fail;

    const uint64_t deadline = now_ms() + duration_ms;

    while (now_ms() < deadline) {
        struct usb_fs_complete done;
        memset(&done, 0, sizeof(done));

        if (ioctl(fd, USB_FS_COMPLETE, &done) != 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }
            break;
        }

        if (done.ep_index >= PASSIVE_XFERS)
            continue;

        const int idx = done.ep_index;
        if (eps[idx].status == 0 && eps[idx].aFrames > 0 && lengths[idx] > 0) {
            EmitCtx ec;
            ec.cb = cb;
            ec.user = user;
            ec.is_acl = (idx == XFER_ACL);

            if (idx == XFER_EVENT)
                sony_bt_event_stream_feed(&event_stream, event_buf, lengths[idx],
                                          emit_packet, &ec);
            else
                sony_bt_acl_stream_feed(&acl_stream, acl_buf, lengths[idx],
                                        emit_packet, &ec);
        }

        if (idx == XFER_EVENT) {
            if (start_xfer(fd, eps, lengths, XFER_EVENT, EVENT_BUF) != 0)
                break;
        } else {
            if (start_xfer(fd, eps, lengths, XFER_ACL, ACL_BUF) != 0)
                break;
        }
    }

    {
        struct usb_fs_stop st;
        memset(&st, 0, sizeof(st));
        st.ep_index = XFER_EVENT;
        ioctl(fd, USB_FS_STOP, &st);
        st.ep_index = XFER_ACL;
        ioctl(fd, USB_FS_STOP, &st);
    }
    {
        struct usb_fs_uninit un;
        memset(&un, 0, sizeof(un));
        ioctl(fd, USB_FS_UNINIT, &un);
    }
    close(fd);
    return 0;

fail:
    if (opened_event || opened_acl) {
        struct usb_fs_stop st;
        memset(&st, 0, sizeof(st));
        if (opened_event) {
            st.ep_index = XFER_EVENT;
            ioctl(fd, USB_FS_STOP, &st);
        }
        if (opened_acl) {
            st.ep_index = XFER_ACL;
            ioctl(fd, USB_FS_STOP, &st);
        }
    }
    {
        struct usb_fs_uninit un;
        memset(&un, 0, sizeof(un));
        ioctl(fd, USB_FS_UNINIT, &un);
    }
    close(fd);
    return -4;
}

#endif
