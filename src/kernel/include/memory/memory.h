#pragma once

#include "libs_stdtypes.h"

#define PAGE_SIZE   4096u

// The allocator manages physical memory in 4 KiB page units.
static inline paddr_t align_up_to_page(paddr_t addr) {
    return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

static inline paddr_t align_down_to_page(paddr_t addr) {
    return addr & ~(PAGE_SIZE - 1);
}

// Initialize bitmap-backed page management on top of the given free RAM range.
// The bitmap itself is placed at the beginning of that range.
void memory_init(paddr_t free_start, paddr_t free_end);

uint8_t *memory_bitmap(void);
paddr_t memory_bitmap_start(void);
paddr_t memory_bitmap_end(void);
paddr_t memory_managed_base(void);
paddr_t memory_managed_end(void);
uint32_t memory_managed_pages(void);
uint32_t memory_bitmap_pages(void);

// Allocate / free contiguous physical pages.
// Returns 0 on allocation failure.
paddr_t palloc(uint32_t n);
void pfree(paddr_t paddr, uint32_t n);
