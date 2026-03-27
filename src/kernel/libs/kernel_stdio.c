#include "sbi.h"

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, SBI_PUTCHAR);
}
