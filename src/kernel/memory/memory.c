#include "libs_stdtypes.h"
#include "common/kernel_panic.h"
#include "memory/memory.h"
#include "libs_stdlib.h"

// One bit tracks one managed page: 1 = allocated, 0 = free.
// The bitmap itself lives at the beginning of free RAM and is therefore not
// included in the allocatable managed region.
static uint8_t *bitmap;
// First physical address that palloc() may return to callers.
static paddr_t managed_region_start;
static uint32_t managed_page_count;
static uint32_t bitmap_page_count;
static bool memory_initialized;

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
