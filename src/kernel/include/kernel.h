#pragma once

// macro: read csr registry
#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })


// macro: write csr registry
#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint32_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)

#define USER_BASE      0x80300000u
#define USER_STACK_TOP 0x80310000u
#define SSTATUS_SPIE   (1u << 5)

void kernel_main(void);
