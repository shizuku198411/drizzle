#include "libs_stdtypes.h"
#include "common/kernel_panic.h"
#include "memory/memory.h"
#include "libs_stdlib.h"

static uint8_t *bitmap;
static paddr_t managed_region_start;
static uint32_t managed_page_count;
static uint32_t bitmap_page_count;

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
    free_start = align_up_to_page(free_start);
    free_end   = align_down_to_page(free_end);

    if (free_end <= free_start) {
        PANIC("invalid free ram range");
    }

    uint32_t total_pages = calc_total_pages(free_start, free_end);
    if (total_pages == 0) {
        PANIC("no allocatable pages");
    }

    uint32_t new_managed_pages = total_pages;
    do {
        managed_page_count = new_managed_pages;
        bitmap_page_count = calc_bitmap_page_count(managed_page_count);
        if (bitmap_page_count >= total_pages) {
            PANIC("bitmap too large for free ram");
        }
        new_managed_pages = total_pages - bitmap_page_count;
    } while (new_managed_pages != managed_page_count);

    bitmap = (uint8_t *) free_start;
    memset(bitmap, 0, bitmap_page_count * PAGE_SIZE);

    managed_region_start = free_start + bitmap_page_count * PAGE_SIZE;
    managed_page_count = total_pages - bitmap_page_count;

    if (managed_page_count == 0) {
        PANIC("no managed pages after bitmap allocation");
    }
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
