#include "common/kernel.h"
#include "bootstrap/bootstrap.h"
#include "user/kernel_user.h"
#include "libs_stdio.h"

void kernel_main(void) {
    kernel_bootstrap();
    load_user_binary();
    printf("[boot] enter user mode\n");
    enter_user_mode(USER_BASE, USER_STACK_TOP);
}
