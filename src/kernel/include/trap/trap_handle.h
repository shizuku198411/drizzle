#pragma once

#include "libs_stdtypes.h"

// trap_frame is laid out to match the save/restore order in trap_entry().
//
// Stack slot / offset / meaning:
//   +-------+---------+---------------------------------------------+
//   | slot  | offset  | saved register / value                      |
//   +-------+---------+---------------------------------------------+
//   | 0     | 0x00    | ra                                          |
//   | 1     | 0x04    | gp                                          |
//   | 2     | 0x08    | tp                                          |
//   | 3     | 0x0c    | t0  (restored from trap_scratch.save_t0)    |
//   | 4     | 0x10    | t1  (restored from trap_scratch.save_t1)    |
//   | 5     | 0x14    | t2  (restored from trap_scratch.save_t2)    |
//   | 6     | 0x18    | t3                                          |
//   | 7     | 0x1c    | t4                                          |
//   | 8     | 0x20    | t5                                          |
//   | 9     | 0x24    | t6                                          |
//   | 10    | 0x28    | a0  (recovered from sscratch after csrrw)   |
//   | 11-17 | 0x2c-44 | a1-a7                                       |
//   | 18-29 | 0x48-74 | s0-s11                                      |
//   | 30    | 0x78    | interrupted sp                              |
//   | 31    | 0x7c    | reserved (currently unused padding slot)    |
//   +-------+---------+---------------------------------------------+
//
// trap_entry() allocates 32 words so this C layout must stay in sync with the
// assembly save/load sequence.
struct trap_frame {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
    uint32_t reserved;
} __attribute__((packed));

struct trap_scratch {
    uint32_t save_t0;
    uint32_t save_t1;
    uint32_t save_t2;
    uint32_t save_sp;
    uint32_t kernel_sp;
};

void trap_entry(void);
void trap_init_scratch(uint32_t kernel_sp);

#define SCAUSE_INSTRUCTION_ADDRESS_MISALIGNED       0x00
#define SCAUSE_INSTRUCTION_ACCESS_FAULT             0x01
#define SCAUSE_ILLEGAL_INSTRUCTION                  0x02
#define SCAUSE_BREAKPOINT                           0x03
#define SCAUSE_LOAD_ADDRESS_MISALIGNED              0x04
#define SCAUSE_LOAD_ACCESS_FAULT                    0x05
#define SCAUSE_STORE_AMO_ADDRESS_MISALIGNED         0x06
#define SCAUSE_STORE_AMO_ACCESS_FAULT               0x07
#define SCAUSE_ECALL_FROM_U_MODE                    0x08
#define SCAUSE_ECALL_FROM_S_MODE                    0x09
// Reserved 0x0a
// Reserved 0x0b
#define SCAUSE_INSTRUCTION_PAGE_FAULT               0x0c
#define SCAUSE_LOAD_PAGE_FAULT                      0x0d
// Reserved 0x0e
#define SCAUSE_STORE_AMO_PAGE_FAULT                 0x0f

#define SCAUSE_SUPERVISOR_TIMER                     0x80000005
