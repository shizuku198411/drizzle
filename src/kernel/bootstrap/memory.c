#include "common/kernel_panic.h"
#include "bootstrap/memory.h"
#include "memory/memory.h"
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

static void init_bss(void) {
    size_t bss_size = get_range_size(__bss_start_addr, __bss_end_addr);
    memset(__bss_start_addr, 0, bss_size);
    if (!is_zeroed(__bss_start_addr, bss_size)) {
        PANIC("\nbss clear failed. __bss_start_addr: %p, __bss_end_addr: %p", __bss_start_addr, __bss_end_addr);
    }
}

static void init_page_allocator(void) {
    paddr_t free_start = (paddr_t) __free_ram_start_addr;
    paddr_t free_end   = (paddr_t) __free_ram_end_addr;

    memory_init(free_start, free_end);
}

static void print_memory_info(void) {
    printf("[boot]   memory information\n");

    printf("[boot]     memory layout\n");
    printf("[boot]       kernel       : %p - %p (size=%x)\n",
           __kernel_start_addr,
           __kernel_end_addr,
           (unsigned)get_range_size(__kernel_start_addr, __kernel_end_addr));
    printf("[boot]       bss          : %p - %p (size=%x)\n",
           __bss_start_addr,
           __bss_end_addr,
           (unsigned)get_range_size(__bss_start_addr, __bss_end_addr));
    printf("[boot]       kernel stack : %p - %p (size=%x)\n",
           __stack_start_addr,
           __stack_end_addr,
           (unsigned)get_range_size(__stack_start_addr, __stack_end_addr));
    printf("[boot]       free ram     : %p - %p (size=%x)\n",
           __free_ram_start_addr,
           __free_ram_end_addr,
           (unsigned)get_range_size(__free_ram_start_addr, __free_ram_end_addr));

    printf("[boot]     pages\n");
    printf("[boot]       bitmap range  : %p - %p (pages=%d)\n",
           memory_bitmap_start(),
           memory_bitmap_end(),
           memory_bitmap_pages());
    printf("[boot]       managed range : %p - %p (pages=%d)\n",
           memory_managed_base(),
           memory_managed_end(),
           memory_managed_pages());
}

void bootstrap_memory(void) {
    printf("[boot] initialize memory\n");

    // bss clear
    printf("[boot]   init bss...");
    init_bss();
    printf("OK\n");
    
    // init page allocator
    printf("[boot]   init page allocator...");
    init_page_allocator();
    printf("OK\n");

    // print memory information
    print_memory_info();
}
