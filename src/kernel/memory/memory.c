#include "libs_stdtypes.h"
#include "common/kernel_panic.h"
#include "memory/memory.h"
#include "libs_stdlib.h"

// variables defined in "kernel.ld"
extern char __kernel_start_addr[], __kernel_end_addr[];
extern char __stack_start_addr[], __stack_end_addr[];
extern char __free_ram_start_addr[], __free_ram_end_addr[];

// One bit tracks one managed page: 1 = allocated, 0 = free.
// The bitmap itself lives at the beginning of free RAM and is therefore not
// included in the allocatable managed region.
static uint8_t *bitmap;
// First physical address that palloc() may return to callers.
static paddr_t managed_region_start;
static uint32_t managed_page_count;
static uint32_t bitmap_page_count;
static bool memory_initialized;
static paddr_t kernel_root_page_table;

static uint32_t calc_total_pages(paddr_t start, paddr_t end) {
    return (end - start) / PAGE_SIZE;
}

static uint32_t calc_bitmap_bytes(uint32_t page_count) {
    return (page_count + 7) / 8;
}

static uint32_t calc_bitmap_page_count(uint32_t page_count) {
    return align_up_to_page(calc_bitmap_bytes(page_count)) / PAGE_SIZE;
}

void memory_init(paddr_t free_start, paddr_t free_end) {
    // The linker keeps the region aligned today, but the allocator should only
    // ever reason about whole pages.
    free_start = align_up_to_page(free_start);
    free_end   = align_down_to_page(free_end);

    if (free_end <= free_start) {
        PANIC("invalid free ram range");
    }

    uint32_t total_pages = calc_total_pages(free_start, free_end);
    if (total_pages == 0) {
        PANIC("no allocatable pages");
    }

    // The bitmap is stored inside the same free RAM range it describes.
    // Recalculate until the bitmap size and the remaining managed pages agree.
    uint32_t new_managed_pages = total_pages;
    do {
        managed_page_count = new_managed_pages;
        bitmap_page_count = calc_bitmap_page_count(managed_page_count);
        if (bitmap_page_count >= total_pages) {
            PANIC("bitmap too large for free ram");
        }
        new_managed_pages = total_pages - bitmap_page_count;
    } while (new_managed_pages != managed_page_count);

    // Place the bitmap at the beginning of free RAM, then start handing out
    // pages from the first page after that bitmap region.
    bitmap = (uint8_t *) free_start;
    memset(bitmap, 0, bitmap_page_count * PAGE_SIZE);

    managed_region_start = free_start + bitmap_page_count * PAGE_SIZE;
    managed_page_count = total_pages - bitmap_page_count;

    if (managed_page_count == 0) {
        PANIC("no managed pages after bitmap allocation");
    }

    memory_initialized = true;
}

uint8_t *memory_bitmap(void) {
    return bitmap;
}

paddr_t memory_bitmap_start(void) {
    return (paddr_t) bitmap;
}

paddr_t memory_bitmap_end(void) {
    return memory_bitmap_start() + bitmap_page_count * PAGE_SIZE;
}

paddr_t memory_managed_base(void) {
    return managed_region_start;
}

paddr_t memory_managed_end(void) {
    return managed_region_start + managed_page_count * PAGE_SIZE;
}

uint32_t memory_managed_pages(void) {
    return managed_page_count;
}

uint32_t memory_bitmap_pages(void) {
    return bitmap_page_count;
}

static bool bitmap_test(uint32_t idx) {
    return (bitmap[idx / 8] >> (idx % 8)) & 1;
}

static void bitmap_set(uint32_t idx) {
    bitmap[idx / 8] |= (uint8_t) (1u << (idx % 8));
}

static void bitmap_clear(uint32_t idx) {
    bitmap[idx / 8] &= (uint8_t) ~(1u << (idx % 8));
}

paddr_t palloc(uint32_t n) {
    if (!memory_initialized) {
        PANIC("memory allocator is not initialized");
    }
    if (n == 0 || n > managed_page_count) {
        PANIC("invalid allocate page count: %d\n", n);
    }

    // Scan linearly for n consecutive free pages.
    // "run" counts the current streak of free pages ending at index i.
    uint32_t run = 0;
    for (uint32_t i = 0; i < managed_page_count; i++) {
        if (bitmap_test(i)) {
            run = 0;
            continue;
        }

        run++;
        if (run == n) {
            uint32_t start = i + 1 - n;
            for (uint32_t j = start; j <= i; j++) {
                bitmap_set(j);
            }

            paddr_t paddr = managed_region_start + start * PAGE_SIZE;
            // Clear recycled contents before handing memory to the caller.
            memset((void *) paddr, 0, n * PAGE_SIZE);
            return paddr;
        }
    }

    return (paddr_t) 0;
}

void pfree(paddr_t paddr, uint32_t n) {
    if (!memory_initialized) {
        PANIC("memory allocator is not initialized");
    }
    if (n == 0 || n > managed_page_count) {
        PANIC("invalid allocate page count: %d\n", n);
    }
    if (!is_aligned(paddr, PAGE_SIZE)) {
        PANIC("unaligned target address: %p", paddr);
    }
    if (paddr < managed_region_start) {
        PANIC("invalid target address before managed region start: %p", paddr);
    }

    // Convert the physical address back into a page index relative to the
    // first managed page.
    uint32_t start = (paddr - managed_region_start) / PAGE_SIZE;
    if (start >= managed_page_count || start + n > managed_page_count) {
        PANIC("free out of range addr: %p size: %d", paddr, n);
    }

    // Validate the entire range first so that a partial error never leaves the
    // bitmap or page contents half-updated.
    for (uint32_t i = 0; i < n; i++) {
        uint32_t idx = start + i;
        if (!bitmap_test(idx)) {
            PANIC("double free detected addr: %p", paddr + i * PAGE_SIZE);
        }
    }

    // Clear page contents first, then release the range in the bitmap.
    memset((void *) paddr, 0, n * PAGE_SIZE);

    for (uint32_t i = 0; i < n; i++) {
        uint32_t idx = start + i;
        bitmap_clear(idx);
    }
}

void map_page(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, uint32_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE)) {
        PANIC("unaligned vaddr %p", vaddr);
    }
    if (!is_aligned(paddr, PAGE_SIZE)) {
        PANIC("unaligned paddr %p", paddr);
    }

    uint32_t vpn1 = (vaddr >> 22) & 0x3ff;
    if ((table1[vpn1] & PAGE_V) == 0) {
        // create page table
        paddr_t pt_paddr = palloc(1);
        if (pt_paddr == 0) {
            PANIC("page allocate failed");
        }
        table1[vpn1] = ((pt_paddr / PAGE_SIZE) << 10) | PAGE_V;
    }

    uint32_t vpn0 = (vaddr >> 12) & 0x3ff;
    uint32_t *table0 = (uint32_t *) ((table1[vpn1] >> 10) * PAGE_SIZE);
    if ((table0[vpn0] & PAGE_V) != 0) {
        PANIC("double map occured");
    }
    table0[vpn0] = ((paddr / PAGE_SIZE) << 10) | flags | PAGE_V;
}

void map_kernel_region(uint32_t *table1, vaddr_t vaddr, paddr_t paddr, size_t size, uint32_t flags) {
    if (size == 0) {
        return;
    }
    if (!is_aligned(vaddr, PAGE_SIZE)) {
        PANIC("unaligned region vaddr %p", vaddr);
    }
    if (!is_aligned(paddr, PAGE_SIZE)) {
        PANIC("unaligned region paddr %p", paddr);
    }

    size_t mapped_size = align_up_to_page(size);
    for (size_t offset = 0; offset < mapped_size; offset += PAGE_SIZE) {
        map_page(table1, vaddr + offset, paddr + offset, flags);
    }
}

paddr_t create_kernel_page_table(void) {
    if (kernel_root_page_table != 0) {
        return kernel_root_page_table;
    }

    paddr_t root_paddr = palloc(1);
    if (root_paddr == 0) {
        PANIC("failed to allocate kernel root page table");
    }

    uint32_t *root_table = (uint32_t *) root_paddr;

    // Start with a coarse identity mapping so that enabling paging later can
    // continue to execute the current kernel image, stack, and allocator state.
    map_kernel_region(root_table,
                      align_down_to_page((vaddr_t) __kernel_start_addr),
                      align_down_to_page((paddr_t) __kernel_start_addr),
                      (size_t) (__stack_end_addr - __kernel_start_addr),
                      PAGE_R | PAGE_W | PAGE_X);
    map_kernel_region(root_table,
                      align_down_to_page((vaddr_t) __free_ram_start_addr),
                      align_down_to_page((paddr_t) __free_ram_start_addr),
                      (size_t) (__free_ram_end_addr - __free_ram_start_addr),
                      PAGE_R | PAGE_W);

    kernel_root_page_table = root_paddr;
    return kernel_root_page_table;
}

paddr_t kernel_page_table(void) {
    return kernel_root_page_table;
}

paddr_t create_user_page_table(vaddr_t user_vaddr,
                               paddr_t user_paddr,
                               size_t user_size,
                               vaddr_t user_stack_top,
                               uint32_t user_stack_pages) {
    if (kernel_root_page_table == 0) {
        PANIC("kernel page table is not initialized");
    }
    if (user_size == 0) {
        PANIC("invalid user image size");
    }
    if (user_stack_pages == 0) {
        PANIC("invalid user stack pages");
    }
    if (!is_aligned(user_vaddr, PAGE_SIZE)) {
        PANIC("unaligned user vaddr %p", user_vaddr);
    }
    if (!is_aligned(user_paddr, PAGE_SIZE)) {
        PANIC("unaligned user paddr %p", user_paddr);
    }
    if (!is_aligned(user_stack_top, PAGE_SIZE)) {
        PANIC("unaligned user stack top %p", user_stack_top);
    }

    paddr_t root_paddr = palloc(1);
    if (root_paddr == 0) {
        PANIC("failed to allocate user root page table");
    }

    uint32_t *root_table = (uint32_t *) root_paddr;
    memcpy(root_table, (void *) kernel_root_page_table, PAGE_SIZE);

    map_kernel_region(root_table,
                      user_vaddr,
                      user_paddr,
                      user_size,
                      PAGE_R | PAGE_X | PAGE_U);

    paddr_t user_stack_paddr = palloc(user_stack_pages);
    if (user_stack_paddr == 0) {
        PANIC("failed to allocate user stack pages");
    }

    vaddr_t user_stack_base = user_stack_top - user_stack_pages * PAGE_SIZE;
    map_kernel_region(root_table,
                      user_stack_base,
                      user_stack_paddr,
                      user_stack_pages * PAGE_SIZE,
                      PAGE_R | PAGE_W | PAGE_U);

    return root_paddr;
}
