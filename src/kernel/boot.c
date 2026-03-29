#include "common/kernel.h"

// variables defined in "kernel.ld"
extern char __stack_end_addr[];

__attribute__((section(".text.boot_entry")))
__attribute__((naked))
void boot_entry(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_end_addr]\n"
        "j kernel_main\n"
        :
        : [stack_end_addr] "r" (__stack_end_addr)
    );
}
