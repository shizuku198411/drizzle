#include "kernel.h"
#include "kernel_panic.h"
#include "trap_handle.h"
#include "timer.h"
#include "libs_stdio.h"
#include "libs_stdtypes.h"

static struct trap_scratch boot_trap_scratch;

void trap_init_scratch(uint32_t kernel_sp) {
    boot_trap_scratch.save_t0 = 0;
    boot_trap_scratch.save_t1 = 0;
    boot_trap_scratch.save_t2 = 0;
    boot_trap_scratch.save_sp = 0;
    boot_trap_scratch.kernel_sp = kernel_sp;
    WRITE_CSR(sscratch, (uint32_t)&boot_trap_scratch);
}

__attribute__((section(".text.trap_entry")))
__attribute__((naked))
__attribute__((aligned(4)))
void trap_entry(void) {
    __asm__ __volatile__(
        // a0 <- trap_scratch, sscratch <- original a0
        "csrrw a0, sscratch, a0\n"
        "sw t0,  4 * 0(a0)\n"
        "sw t1,  4 * 1(a0)\n"
        "sw t2,  4 * 2(a0)\n"
        "sw sp,  4 * 3(a0)\n"

        // For traps from U-mode, switch to the kernel stack stored in scratch.
        // For traps from S-mode, keep the current kernel sp.
        "csrr t0, sstatus\n"
        "andi t0, t0, 0x100\n"
        "bnez t0, 1f\n"
        "lw sp,  4 * 4(a0)\n"
        "1:\n"

        // always run trap handler with interrupts disabled to avoid nested traps
        "csrc sstatus, 2\n"

        "addi sp, sp, -4 * 32\n"
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"

        "lw t0,  4 * 0(a0)\n"
        "lw t1,  4 * 1(a0)\n"
        "lw t2,  4 * 2(a0)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"

        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"

        "csrr t0, sscratch\n"
        "sw t0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "lw t0,  4 * 3(a0)\n"
        "sw t0,  4 * 30(sp)\n"

        "csrw sscratch, a0\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n"
    );
}

void handle_trap(void) {
    uint32_t scause  = READ_CSR(scause);
    uint32_t stval   = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);
    uint32_t sstatus = READ_CSR(sstatus);

    // check ecall from U-mode or S-mode
    bool from_user = (sstatus & (1u << 8)) == 0;
    (void)from_user;
    
    switch (scause) {
        case SCAUSE_INSTRUCTION_ADDRESS_MISALIGNED:
            PANIC("Instruction address misaligned. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();
        
        case SCAUSE_INSTRUCTION_ACCESS_FAULT:
            PANIC("Instruction access fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_ILLEGAL_INSTRUCTION:
            PANIC("Illegal instruction. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_BREAKPOINT:
            PANIC("Breakpoint. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_LOAD_ADDRESS_MISALIGNED:
            PANIC("Load address misaligned. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_LOAD_ACCESS_FAULT:
            PANIC("Load access fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_STORE_AMO_ADDRESS_MISALIGNED:
            PANIC("Store/AMO address misaligned. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_STORE_AMO_ACCESS_FAULT:
            PANIC("Store/AMO access fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_ECALL_FROM_U_MODE:
            PANIC("Environment call from U-mode. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_ECALL_FROM_S_MODE:
            PANIC("Environment call from S-mode. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_INSTRUCTION_PAGE_FAULT:
            PANIC("Instruction page fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();
        
        case SCAUSE_LOAD_PAGE_FAULT:
            PANIC("Load page fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_STORE_AMO_PAGE_FAULT:
            PANIC("Store/AMO page fault. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();

        case SCAUSE_SUPERVISOR_TIMER:
            count_up_timer_tick();
            set_next_timer();
            WRITE_CSR(sepc, user_pc);
            return;

        default:
            PANIC("Unexpected trap. scause=%x, stval=%x, sepc=%x\n", scause, stval, user_pc);
            __builtin_unreachable();
    }
}
