#pragma once

__attribute__((noreturn))
void kernel_panic(const char *file, int line, const char *fmt, ...);

#define PANIC(fmt, ...) \
    kernel_panic(__FILE__, __LINE__, fmt, ##__VA_ARGS__)