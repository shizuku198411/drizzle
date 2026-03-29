#include "common/kernel.h"
#include "bootstrap/timer.h"
#include "timer/timer.h"
#include "trap/trap_handle.h"
#include "libs_stdio.h"

void bootstrap_timer(void) {
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
