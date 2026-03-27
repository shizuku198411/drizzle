ENTRY(boot_entry)

SECTIONS {
    /* kernel start address = 0x80200000 */
    . = 0x80200000;
    __kernel_start_addr = .;

    .text : {
        KEEP(*(.text.boot_entry));
        KEEP(*(.text.trap_entry));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.* .srodata .srodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.* .sdata .sdata.*);
    }

    .bss : ALIGN(4) {
        __bss_start_addr = .;
        *(.bss .bss.* .sbss .sbss.*);
        *(COMMON);
        __bss_end_addr = .;
    }

    __kernel_end_addr = ALIGN(16);

    .kstack (NOLOAD) : ALIGN(16) {
        . = ALIGN(16);
        __stack_start_addr = .;
        . += 64 * 1024;
        __stack_end_addr = .;
    }

    /* 64MB free ram */
    . = ALIGN(4096);
    .fram (NOLOAD) : ALIGN(4096) {
        __free_ram_start_addr = .;
        . += 64 * 1024 * 1024;
        __free_ram_end_addr = .;
    }
}
