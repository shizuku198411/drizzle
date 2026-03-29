ENTRY(user_start)

SECTIONS {
    . = 0x80300000;

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
}
