ENTRY(user_start)

SECTIONS {
    . = 0x01000000;
    __user_start_addr = .;

    .text : ALIGN(4) {
        KEEP(*(.text.user_start));
        *(.text .text.*);
    }

    .rodata : ALIGN(4) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4) {
        *(.data .data.*);
    }

    .bss : ALIGN(4) {
        *(.bss .bss.*);
        *(COMMON);
    }

    __user_image_end_addr = .;

    .ustack (NOLOAD) : ALIGN(4096) {
        __user_stack_start_addr = .;
        . += 64 * 1024;     /* 64KiB */
        __user_stack_end_addr = .;
    }
}
