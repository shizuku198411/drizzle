#include "libs_stdtypes.h"
#include "kernel_stdio.h"

static void print_unsigned(unsigned value) {
    unsigned divisor = 1;
    while (value / divisor > 9) {
        divisor *= 10;
    }
    while (divisor > 0) {
        putchar('0' + value / divisor);
        value %= divisor;
        divisor /= 10;
    }
}

static void print_hex(unsigned value, int uppercase) {
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    for (int i = 7; i >= 0; i--) {
        unsigned nibble = (value >> (i * 4)) & 0xf;
        putchar(digits[nibble]);
    }
}

void vprintf(const char *fmt, va_list vargs) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case '\0':
                    putchar('%');
                    return;
                case '%':
                    putchar('%');
                    break;
                case 'c': {
                    int value = va_arg(vargs, int);
                    putchar((char)value);
                    break;
                }
                // print a string
                case 's': {
                    const char *s = va_arg(vargs, const char *);
                    if (!s) {
                        s = "(null)";
                    }
                    while (*s) {
                        putchar(*s);
                        s++;
                    }
                    break;
                }
                // print an integer in decimal
                case 'd': {
                    int value = va_arg(vargs, int);
                    unsigned magnitude = value;
                    if (value < 0) {
                        putchar('-');
                        magnitude = -magnitude;
                    }
                    print_unsigned(magnitude);
                    break;
                }
                // print an unsigned integer in decimal
                case 'u': {
                    unsigned value = va_arg(vargs, unsigned);
                    print_unsigned(value);
                    break;
                }
                // print an integer in hexadecimal
                case 'x': {
                    unsigned value = va_arg(vargs, unsigned);
                    print_hex(value, 0);
                    break;
                }
                // print an integer in uppercase hexadecimal
                case 'X': {
                    unsigned value = va_arg(vargs, unsigned);
                    print_hex(value, 1);
                    break;
                }
                // print a pointer-like value
                case 'p': {
                    unsigned value = va_arg(vargs, unsigned);
                    putchar('0');
                    putchar('x');
                    print_hex(value, 0);
                    break;
                }
                default:
                    putchar('%');
                    putchar(*fmt);
                    break;
            }
        } else {
            putchar(*fmt);
        }

        fmt++;
    }
}

void printf(const char *fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);

    vprintf(fmt, vargs);

    va_end(vargs);
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *) dst;
    const uint8_t *s = (const uint8_t *) src;

    while (n--) {
        *d++ = *s++;
    }

    return dst;
}

void *memset(void *buf, int c, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    while (n--) {
        *p++ = c;
    }
    return buf;
}
