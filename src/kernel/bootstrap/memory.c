#include "common/kernel_panic.h"
#include "bootstrap/memory.h"
#include "libs_stdio.h"
#include "libs_stdlib.h"

// variables defined in "kernel.ld"
extern char __kernel_start_addr[], __kernel_end_addr[];
extern char __bss_start_addr[], __bss_end_addr[];
extern char __stack_start_addr[], __stack_end_addr[];
extern char __free_ram_start_addr[], __free_ram_end_addr[];

static bool is_zeroed(const char *buf, size_t len) {
    while (len--) {
        if (*buf++ != 0) {
            return false;
        }
    }
    return true;
}

static size_t get_range_size(const char *start, const char *end) {
    return (size_t)end - (size_t)start;
}

void bootstrap_memory(void) {
    printf("[boot] initialize memory\n");

    printf("[boot]   bss clear...");
    size_t bss_size = get_range_size(__bss_start_addr, __bss_end_addr);
    memset(__bss_start_addr, 0, bss_size);
    if (!is_zeroed(__bss_start_addr, bss_size)) {
        PANIC("\nbss clear failed. __bss_start_addr: %p, __bss_end_addr: %p", __bss_start_addr, __bss_end_addr);
    }
    printf("OK\n");

    printf("[boot]   memory layout\n");
    printf("[boot]     kernel       : %p - %p (size=%x)\n",
           __kernel_start_addr,
           __kernel_end_addr,
           (unsigned)get_range_size(__kernel_start_addr, __kernel_end_addr));
    printf("[boot]     bss          : %p - %p (size=%x)\n",
           __bss_start_addr,
           __bss_end_addr,
           (unsigned)get_range_size(__bss_start_addr, __bss_end_addr));
    printf("[boot]     kernel stack : %p - %p (size=%x)\n",
           __stack_start_addr,
           __stack_end_addr,
           (unsigned)get_range_size(__stack_start_addr, __stack_end_addr));
    printf("[boot]     free ram     : %p - %p (size=%x)\n",
           __free_ram_start_addr,
           __free_ram_end_addr,
           (unsigned)get_range_size(__free_ram_start_addr, __free_ram_end_addr));
}
