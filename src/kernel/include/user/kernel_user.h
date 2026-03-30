#pragma once

#include "libs_stdtypes.h"

size_t load_user_binary(void);
void enter_user_mode(unsigned int entry, unsigned int user_sp, paddr_t page_table) __attribute__((noreturn));
