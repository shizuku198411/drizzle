#include "kernel.h"
#include "kernel_panic.h"
#include "trap_handle.h"
#include "timer.h"
#include "libs_stdio.h"
#include "libs_stdlib.h"

// variables defined in "kernel.ld"
extern char __kernel_start_addr[], __kernel_end_addr[];
extern char __bss_start_addr[], __bss_end_addr[];
extern char __stack_start_addr[], __stack_end_addr[];
extern char __free_ram_start_addr[], __free_ram_end_addr[];
extern char _binary_bin_user_bin_start[], _binary_bin_user_bin_end[];

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

static size_t get_user_binary_size(void) {
    return (size_t)(_binary_bin_user_bin_end - _binary_bin_user_bin_start);
}

static void bootstrap_memory(void) {
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

static void boostrap_trap_vector(void) {
    printf("[boot] set trap vector\n");
    WRITE_CSR(stvec, (uint32_t) trap_entry);
    printf("[boot]   stvec: %p\n", READ_CSR(stvec));
}

static void bootstrap_timer(void) {
    // ensure trap-entry stack swap source is valid before first timer interrupt
    uint32_t kernel_sp;
    __asm__ __volatile__("mv %0, sp" : "=r"(kernel_sp));
    trap_init_scratch(kernel_sp);

    // enable timer interrupt
    // set next timer
    set_next_timer();
    enable_timer_interrupt();
    
    printf("[boot] set timer\n");
    printf("[boot]   sie            : %x\n", READ_CSR(sie));
    printf("[boot]   sstatus        : %x\n", READ_CSR(sstatus));
    printf("[boot]   timer interval : %d ms\n", TIMER_INTERVAL / 10000);
}

static void kernel_bootstrap(void) {
    bootstrap_memory();
    boostrap_trap_vector();
    bootstrap_timer();
}

static void load_user_binary(void) {
    size_t user_size = get_user_binary_size();

    memcpy((void *)USER_BASE, _binary_bin_user_bin_start, user_size);

    printf("[boot] load user binary\n");
    printf("[boot]   user base      : %p\n", (void *)USER_BASE);
    printf("[boot]   user size      : %x\n", (unsigned)user_size);
    printf("[boot]   user stack top : %p\n", (void *)USER_STACK_TOP);
}

__attribute__((noreturn))
__attribute__((naked))
static void enter_user_mode(uint32_t entry, uint32_t user_sp) {
    __asm__ __volatile__(
        "csrw sepc, a0\n"
        "mv sp, a1\n"
        "li t0, %[sstatus_spie]\n"
        "csrw sstatus, t0\n"
        "sret\n"
        :
        : [sstatus_spie] "i" (SSTATUS_SPIE)
    );
}

void kernel_main(void) {
    kernel_bootstrap();
    load_user_binary();
    printf("[boot] enter user mode\n");
    enter_user_mode(USER_BASE, USER_STACK_TOP);
}
