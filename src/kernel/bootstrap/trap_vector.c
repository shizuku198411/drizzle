#include "common/kernel.h"
#include "bootstrap/trap_vector.h"
#include "trap/trap_handle.h"
#include "libs_stdio.h"

void bootstrap_trap_vector(void) {
    printf("[boot] set trap vector\n");
    WRITE_CSR(stvec, (uint32_t) trap_entry);
    printf("[boot]   stvec: %p\n", READ_CSR(stvec));
}
