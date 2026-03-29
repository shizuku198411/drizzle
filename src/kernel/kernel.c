#include "common/kernel.h"
#include "bootstrap/bootstrap.h"
#include "tests/test_runner.h"
#include "user/kernel_user.h"
#include "libs_stdio.h"

void kernel_main(void) {
    kernel_bootstrap();
    load_user_binary();
#if RUN_KERNEL_TESTS
    run_kernel_tests();
#endif
    printf("[boot] enter user mode\n");
    enter_user_mode(USER_BASE, USER_STACK_TOP);
}
