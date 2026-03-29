#pragma once

#define TIMER_INTERVAL  200000  // 20ms
//#define TIMER_INTERVAL 10000000

#define SIE_STIE    (1u << 5)
#define SSTATUS_SIE (1u << 1)

void enable_timer_interrupt(void);
void set_next_timer(void);
void count_up_timer_tick(void);
