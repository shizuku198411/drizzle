#pragma once

void load_user_binary(void);
void enter_user_mode(unsigned int entry, unsigned int user_sp) __attribute__((noreturn));
