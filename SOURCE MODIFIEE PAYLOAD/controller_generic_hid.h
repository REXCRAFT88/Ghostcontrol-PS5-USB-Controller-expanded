#pragma once
#include <stdint.h>
#include "gc_types.h"

#define GENERIC_PS2_VID 0x0810u
#define GENERIC_PS2_PID 0x0003u
#define GENERIC_HORI_VID 0x0f0du
#define GENERIC_HORI_PID 0x00c1u
#define GENERIC_HID_EP_IN 0x81u

int generic_hid_is_supported(uint16_t vid, uint16_t pid);
const char *generic_hid_name(uint16_t vid, uint16_t pid);
int generic_hid_parse(uint16_t vid, uint16_t pid,
                      const uint8_t *buf, uint32_t len,
                      ScePadData *out);
