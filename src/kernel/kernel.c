#include "common/kernel.h"
#include "bootstrap/bootstrap.h"
#include "memory/memory.h"
#include "tests/test_runner.h"
#include "user/kernel_user.h"
#include "libs_stdio.h"

void kernel_main(void) {
    kernel_bootstrap();
    size_t user_size = load_user_binary();
    paddr_t user_page_table = create_user_page_table(USER_BASE,
                                                     USER_LOAD_PADDR,
                                                     user_size,
                                                     USER_STACK_TOP,
                                                     USER_STACK_PAGES);
#if RUN_KERNEL_TESTS
    run_kernel_tests();
#endif
    printf("[boot] enter user mode\n");
    enter_user_mode(USER_BASE, USER_STACK_TOP, user_page_table);
}
