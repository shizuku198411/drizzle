#include "tests/test_memory.h"
#include "tests/kernel_test.h"
#include "memory/memory.h"
#include "libs_stdio.h"

void run_memory_tests(void) {
    // Basic contiguous allocation:
    // the first request should start from the managed base, and the next
    // request should follow immediately after the consumed 2-page range.
    paddr_t first_alloc = palloc(2);
    paddr_t second_alloc = palloc(1);

    KTEST_ASSERT(first_alloc == memory_managed_base(),
                 "expected first alloc at %p, got %p",
                 memory_managed_base(),
                 first_alloc);
    KTEST_ASSERT(second_alloc == memory_managed_base() + 2 * PAGE_SIZE,
                 "expected second alloc at %p, got %p",
                 memory_managed_base() + 2 * PAGE_SIZE,
                 second_alloc);

    // Fragmentation check:
    // freeing the first 2 pages is not enough for a 3-page request because
    // second_alloc still occupies the page right after that gap.
    pfree(first_alloc, 2);

    paddr_t third_alloc = palloc(3);
    KTEST_ASSERT(third_alloc == memory_managed_base() + 3 * PAGE_SIZE,
                 "expected realloc at %p, got %p",
                 memory_managed_base() + 3 * PAGE_SIZE,
                 third_alloc);

    // Merge check:
    // once both adjacent allocations are freed, the allocator should be able
    // to reuse the managed base again for a 3-page request.
    pfree(second_alloc, 1);
    pfree(third_alloc, 3);

    paddr_t merged_alloc = palloc(3);
    KTEST_ASSERT(merged_alloc == memory_managed_base(),
                 "expected merged alloc at %p, got %p",
                 memory_managed_base(),
                 merged_alloc);

    pfree(merged_alloc, 3);

    printf("[test] memory allocator: OK\n");
}
