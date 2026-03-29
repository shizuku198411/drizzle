#include "trap/syscall_handle.h"
#include "common/kernel_stdio.h"

int syscall_handle_putchar(int ch) {
    return putchar((char) ch);
}
