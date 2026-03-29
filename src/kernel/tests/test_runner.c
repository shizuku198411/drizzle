#include "tests/test_runner.h"
#include "tests/test_memory.h"
#include "libs_stdio.h"

void run_kernel_tests(void) {
    printf("[test] run kernel tests\n");

    // Keep the top-level runner small and delegate each subsystem test to its
    // own file so tests can grow without cluttering kernel.c.
    run_memory_tests();

    printf("[test] all kernel tests passed\n");
}
