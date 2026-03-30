#include "common/kernel.h"
#include "memory/memory.h"
#include "user/kernel_user.h"
#include "libs_stdio.h"
#include "libs_stdlib.h"

// variables defined by objcopy-generated user image object
extern char _binary_bin_user_bin_start[], _binary_bin_user_bin_end[];

static size_t get_user_binary_size(void) {
    return (size_t)(_binary_bin_user_bin_end - _binary_bin_user_bin_start);
}

size_t load_user_binary(void) {
    size_t user_size = get_user_binary_size();

    memcpy((void *)USER_LOAD_PADDR, _binary_bin_user_bin_start, user_size);

    printf("[boot] load user binary\n");
    printf("[boot]   user base      : %p\n", (void *)USER_BASE);
    printf("[boot]   user load addr : %p\n", (void *)USER_LOAD_PADDR);
    printf("[boot]   user size      : %x\n", (unsigned)user_size);
    printf("[boot]   user stack     : %p - %p (size=%x)\n",
           (void *)USER_STACK_BASE,
           (void *)USER_STACK_TOP,
           (unsigned) USER_STACK_SIZE);
    printf("[boot]   user stack top : %p\n", (void *)USER_STACK_TOP);

    return user_size;
}

__attribute__((noreturn))
__attribute__((naked))
void enter_user_mode(unsigned int entry, unsigned int user_sp, paddr_t page_table) {
    __asm__ __volatile__(
        "srli a2, a2, 12\n"
        "li t0, %[satp_sv32]\n"
        "or a2, a2, t0\n"
        "csrw satp, a2\n"
        "sfence.vma\n"
        "csrw sepc, a0\n"
        "mv sp, a1\n"
        "li t0, %[sstatus_spie]\n"
        "csrw sstatus, t0\n"
        "sret\n"
        :
        : [satp_sv32] "i" (SATP_SV32),
          [sstatus_spie] "i" (SSTATUS_SPIE)
    );
}
