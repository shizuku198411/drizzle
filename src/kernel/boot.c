#include "kernel.h"

// variables defined in "kernel.ld"
extern char __stack_top_addr[];

__attribute__((section(".text.boot_entry")))
__attribute__((naked))
void boot_entry(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top_addr]\n"
        "csrw sscratch, sp\n"
        "j kernel_main\n"
        :
        : [stack_top_addr] "r" (__stack_top_addr)
    );
}