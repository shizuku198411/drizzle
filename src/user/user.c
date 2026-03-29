void main(void);

__attribute__((section(".text.user_start")))
__attribute__((naked))
void user_start(void) {
    __asm__ __volatile__(
        "call main\n"
        "1:\n"
        "j 1b\n"
    );
}
