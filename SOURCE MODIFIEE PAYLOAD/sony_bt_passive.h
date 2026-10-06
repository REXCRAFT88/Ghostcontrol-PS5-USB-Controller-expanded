#pragma once
#include <stdint.h>
#include "sony_bt_probe.h"

typedef void (*SonyBtObservedPacket)(int is_acl,
                                     const uint8_t *packet,
                                     uint32_t len,
                                     void *user);

/*
 * Observe naturally occurring HCI traffic for a bounded duration.
 *
 * Safety boundary:
 * - opens event-IN and ACL-IN only
 * - no driver detach
 * - no HCI commands/control requests
 * - no ACL OUT endpoint
 * - no pairing/inquiry/reset
 *
 * Returns 0 on a completed observation window, negative on setup failure.
 */
int sony_bt_passive_observe(const SonyBtRadioProbe *radio,
                            unsigned hci_index,
                            unsigned duration_ms,
                            SonyBtObservedPacket cb,
                            void *user);
