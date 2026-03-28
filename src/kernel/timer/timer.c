#include "kernel.h"
#include "timer.h"
#include "libs_stdtypes.h"

volatile uint32_t timer_tick_count = 0;

void enable_timer_interrupt(void) {
    uint32_t sie = READ_CSR(sie);
    sie |= SIE_STIE;
    WRITE_CSR(sie, sie);

    uint32_t sstatus = READ_CSR(sstatus);
    sstatus |= SSTATUS_SIE;
    WRITE_CSR(sstatus, sstatus);
}

static inline uint64_t rdtime(void) {
    uint32_t hi0, lo, hi1;

    do {
        __asm__ __volatile__("rdtimeh %0" : "=r"(hi0));
        __asm__ __volatile__("rdtime  %0" : "=r"(lo));
        __asm__ __volatile__("rdtimeh %0" : "=r"(hi1));
    } while (hi0 != hi1);

    return ((uint64_t)hi0 << 32) | lo;
}


static inline void wrtimecmp(uint64_t val) {
    uint32_t lo = val & 0xffffffff;
    uint32_t hi = val >> 32;

    // Program the high word to all ones first so the intermediate value
    // cannot transiently become an already-expired deadline on RV32.
    WRITE_CSR(stimecmph, 0xffffffffu);
    WRITE_CSR(stimecmp, lo);
    WRITE_CSR(stimecmph, hi);
}

void set_next_timer() {
    uint64_t now = rdtime();
    wrtimecmp(now + TIMER_INTERVAL);
}

void count_up_timer_tick(void) {
    timer_tick_count++;
}
