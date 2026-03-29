#pragma once

#include "common/kernel_panic.h"

// Kernel self-tests stop immediately on failure so the first broken invariant
// is visible in the boot log.
#define KTEST_ASSERT(cond, fmt, ...)                                           \
    do {                                                                       \
        if (!(cond)) {                                                         \
            PANIC("[test] %s: " fmt, __func__, ##__VA_ARGS__);                 \
        }                                                                      \
    } while (0)
