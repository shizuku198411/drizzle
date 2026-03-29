#include "common/kernel_panic.h"
#include "libs_stdio.h"
#include "libs_stdtypes.h"

__attribute__((noreturn))
void kernel_panic(const char *file, int line, const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    printf("KERNEL PANIC: %s:%d: ", file, line);
    vprintf(fmt, vargs);
    printf("\n");

    va_end(vargs);

    for (;;) {
        __asm__ __volatile__("wfi");
    }
}
