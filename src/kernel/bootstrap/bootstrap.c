#include "bootstrap/bootstrap.h"
#include "bootstrap/memory.h"
#include "bootstrap/timer.h"
#include "bootstrap/trap_vector.h"

void kernel_bootstrap(void) {
    bootstrap_memory();
    bootstrap_trap_vector();
    bootstrap_timer();
}
