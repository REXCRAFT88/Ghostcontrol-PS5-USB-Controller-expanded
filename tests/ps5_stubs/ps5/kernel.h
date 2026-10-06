#pragma once
#include <stdint.h>
#include <sys/types.h>
static inline int kernel_set_ucred_authid(pid_t p, uint64_t authid) {
    (void)p; (void)authid; return 0;
}
static inline int kernel_set_ucred_caps(pid_t p, const uint8_t caps[16]) {
    (void)p; (void)caps; return 0;
}
