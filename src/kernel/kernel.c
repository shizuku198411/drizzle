#include "libs_stdio.h"

void kernel_main(void) {
    printf("\nprintf format test start\n");
    printf("string   : %s\n", "Hello World");
    printf("string   : %s\n", (const char *)0);
    printf("char     : %c %c %c\n", 'O', 'S', '!');
    printf("signed   : %d %d %d\n", 0, 42, -42);
    printf("unsigned : %u %u\n", 0u, 1234567890u);
    printf("hex      : %x\n", 0x1234abcd);
    printf("HEX      : %X\n", 0x1234abcd);
    printf("pointer  : %p\n", 0x80200000u);
    printf("percent  : %%\n");
    printf("unknown  : %q\n");
    printf("printf format test end\n");

    __asm__ __volatile__("wfi");
}
