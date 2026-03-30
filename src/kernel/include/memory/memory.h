#pragma once

#include "libs_stdtypes.h"

#define PAGE_SIZE   4096u

#define SATP_SV32   (1u << 31)
#define PAGE_V      (1 << 0)    // enable bit
#define PAGE_R      (1 << 1)    // readable
#define PAGE_W      (1 << 2)    // writable
#define PAGE_X      (1 << 3)    // executable
#define PAGE_U      (1 << 4)    // accessable from U-mode


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

void map_page(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags);
void map_kernel_region(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, size_t size, uint32_t flags);
paddr_t create_kernel_page_table(void);
paddr_t kernel_page_table(void);
paddr_t create_user_page_table(vaddr_t user_vaddr,
                               paddr_t user_paddr,
                               size_t user_size,
                               vaddr_t user_stack_top,
                               uint32_t user_stack_pages);
