#include "sony_bt_transport.h"
#include <stddef.h>
#include <string.h>

static uint16_t le16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int sony_bt_extract_hid_input(const uint8_t *acl, uint32_t len,
                              SonyBtHidInputView *out) {
    if (!acl || !out || len < 10)
        return 0;

    memset(out, 0, sizeof(*out));

    const uint16_t handle_pb_bc = le16(&acl[0]);
    const uint16_t acl_data_len = le16(&acl[2]);

    /* The supplied buffer must contain the complete ACL payload. */
    if ((uint32_t)acl_data_len + 4u > len)
        return 0;

    if (acl_data_len < 5u)
        return 0;

    const uint16_t l2cap_len = le16(&acl[4]);
    const uint16_t cid = le16(&acl[6]);

    /* L2CAP payload follows its 4-byte header and must be fully present. */
    if ((uint32_t)l2cap_len + 4u > acl_data_len)
        return 0;

    if (cid != BT_L2CAP_CID_HID_INTERRUPT || l2cap_len < 2u)
        return 0;

    /* HIDP DATA with report type INPUT is 0xA1. */
    if (acl[8] != BT_HIDP_DATA_INPUT)
        return 0;

    out->connection_handle = handle_pb_bc & 0x0fffu;
    out->cid = cid;
    out->report = &acl[9];
    out->report_len = (uint16_t)(l2cap_len - 1u);
    return out->report_len > 0;
}
