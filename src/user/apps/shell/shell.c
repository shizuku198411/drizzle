void main(void) {
    volatile unsigned counter = 0;

    for (;;) {
        counter++;
        if ((counter % 100000000u) == 0) {
            __asm__ __volatile__("ecall");
        }
        __asm__ __volatile__("nop");
    }
}
