#pragma once

#include "libs_stdtypes.h"

#define PAGE_SIZE   4096u

static inline paddr_t align_up_to_page(paddr_t addr) {
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

static inline paddr_t align_down_to_page(paddr_t addr) {
    return addr & ~(PAGE_SIZE - 1);
}

void memory_init(paddr_t free_start, paddr_t free_end);

uint8_t *memory_bitmap(void);
paddr_t memory_bitmap_start(void);
paddr_t memory_bitmap_end(void);
paddr_t memory_managed_base(void);
paddr_t memory_managed_end(void);
uint32_t memory_managed_pages(void);
uint32_t memory_bitmap_pages(void);
