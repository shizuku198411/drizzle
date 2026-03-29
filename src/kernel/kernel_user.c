#include "common/kernel.h"
#include "user/kernel_user.h"
#include "libs_stdio.h"
#include "libs_stdlib.h"

// variables defined by objcopy-generated user image object
extern char _binary_bin_user_bin_start[], _binary_bin_user_bin_end[];

static size_t get_user_binary_size(void) {
    return (size_t)(_binary_bin_user_bin_end - _binary_bin_user_bin_start);
}

void load_user_binary(void) {
    size_t user_size = get_user_binary_size();

    memcpy((void *)USER_BASE, _binary_bin_user_bin_start, user_size);

    printf("[boot] load user binary\n");
    printf("[boot]   user base      : %p\n", (void *)USER_BASE);
    printf("[boot]   user size      : %x\n", (unsigned)user_size);
    printf("[boot]   user stack top : %p\n", (void *)USER_STACK_TOP);
}

__attribute__((noreturn))
__attribute__((naked))
void enter_user_mode(unsigned int entry, unsigned int user_sp) {
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
