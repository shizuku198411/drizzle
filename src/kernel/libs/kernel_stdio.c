#include "common/sbi.h"

int putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, SBI_PUTCHAR);
    return 0;
}
