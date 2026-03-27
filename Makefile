.PHONY: build qemu-start

# directories
MAP_DIR := map
OBJ_DIR := obj
BIN_DIR := bin
KERNEL_SRC_DIR := src/kernel
KERNEL_INC_DIR := src/kernel/include
LIBS_SRC_DIR := src/libs
LIBS_INC_DIR := src/libs/include

# qemu setting
QEMU := qemu-system-riscv32
QEMU_OPT := -machine virt -bios default -nographic -serial mon:stdio --no-reboot

# c compiler
CC := clang
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib -I$(KERNEL_INC_DIR) -I$(LIBS_INC_DIR)

# kernel elf
KERNEL_ELF := $(BIN_DIR)/kernel.elf
KERNEL_SRCS := $(wildcard $(KERNEL_SRC_DIR)/*.c) $(wildcard $(KERNEL_SRC_DIR)/libs/*.c) $(wildcard $(LIBS_SRC_DIR)/*.c)
KERNEL_HDRS := $(wildcard $(KERNEL_INC_DIR)/*.h) $(wildcard $(LIBS_INC_DIR)/*.h)
KERNEL_LDSCRIPT := $(KERNEL_SRC_DIR)/kernel.ld


build: $(KERNEL_ELF)

qemu-start: $(KERNEL_ELF)
	$(QEMU) $(QEMU_OPT) -kernel $(KERNEL_ELF)

$(KERNEL_ELF): $(KERNEL_SRCS) $(KERNEL_HDRS) $(KERNEL_LDSCRIPT) | $(BIN_DIR) $(MAP_DIR)
	$(CC) $(CFLAGS) -Wl,-T$(KERNEL_SRC_DIR)/kernel.ld -Wl,-Map=$(MAP_DIR)/kernel.map -o $@ \
		$(KERNEL_SRCS)

$(BIN_DIR) $(MAP_DIR):
	mkdir -p $@
