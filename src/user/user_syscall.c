#include "libs_syscall.h"

static int syscall(int sysno, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5) {
    register int a0 __asm__("a0") = arg0;
    register int a1 __asm__("a1") = arg1;
    register int a2 __asm__("a2") = arg2;
    register int a3 __asm__("a3") = arg3;
    register int a4 __asm__("a4") = arg4;
    register int a5 __asm__("a5") = arg5;
    register int a6 __asm__("a6") = sysno;

    __asm__ __volatile__(
        "ecall"
        : "+r" (a0)
        : "r" (a1), "r" (a2), "r" (a3), "r" (a4), "r" (a5), "r" (a6)
        : "memory"
    );

    return a0;
}

int putchar(char ch) {
    return syscall(SYSCALL_PUTCHAR, (int) ch, 0, 0, 0, 0, 0);
}
