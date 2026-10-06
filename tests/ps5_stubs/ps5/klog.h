#pragma once
#include <stdarg.h>
static inline int klog_printf(const char *fmt, ...) {
    (void)fmt; return 0;
}
