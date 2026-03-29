#include "libs_stdio.h"

void main(void) {
    volatile unsigned counter = 0;

    for (;;) {
        counter++;
        if ((counter % 100000000u) == 0) {
            printf("ecall occured!\n");
        }
        __asm__ __volatile__("nop");
    }
}
