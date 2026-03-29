#include "trap/syscall_handle.h"
#include "libs_syscall.h"

int syscall_handle(int sysno, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    (void) arg1;
    (void) arg2;
    (void) arg3;
    (void) arg4;
    (void) arg5;

    switch (sysno) {
        case SYSCALL_PUTCHAR:
            return syscall_handle_putchar(arg0);

        default:
            return -ERR_NOSYS;
    }
}
