.PHONY: build kernel-start qemu-debug

# directories
MAP_DIR := map
OBJ_DIR := obj
BIN_DIR := bin
USER_SRC_DIR := src/user
USER_APP_DIR := $(USER_SRC_DIR)/apps
KERNEL_SRC_DIR := src/kernel
KERNEL_INC_DIR := src/kernel/include
LIBS_SRC_DIR := src/libs
LIBS_INC_DIR := src/libs/include

# qemu setting
QEMU := qemu-system-riscv32
QEMU_OPT := -machine virt -bios default -nographic -serial mon:stdio --no-reboot
QEMU_GDB_PORT ?= 1234

# c compiler
CC := clang
OBJCOPY := llvm-objcopy
CFLAGS := -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib -I$(KERNEL_INC_DIR) -I$(LIBS_INC_DIR)

# kernel elf
KERNEL_ELF := $(BIN_DIR)/kernel.elf
APP_NAME ?= shell
APP_SRC_DIR := $(USER_APP_DIR)/$(APP_NAME)
APP_ELF := $(BIN_DIR)/user.elf
APP_BIN := $(BIN_DIR)/user.bin
APP_OBJ := $(OBJ_DIR)/user.bin.o
APP_SRCS := $(USER_SRC_DIR)/user.c $(wildcard $(APP_SRC_DIR)/*.c)
KERNEL_SRCS := $(wildcard $(KERNEL_SRC_DIR)/*.c) $(wildcard $(KERNEL_SRC_DIR)/libs/*.c) $(wildcard $(KERNEL_SRC_DIR)/trap/*.c) $(wildcard $(KERNEL_SRC_DIR)/timer/*.c) $(wildcard $(LIBS_SRC_DIR)/*.c)
KERNEL_HDRS := $(wildcard $(KERNEL_INC_DIR)/*.h) $(wildcard $(LIBS_INC_DIR)/*.h)
KERNEL_LDSCRIPT := $(KERNEL_SRC_DIR)/kernel.ld


build: $(KERNEL_ELF)

kernel-start: $(KERNEL_ELF)
	$(QEMU) $(QEMU_OPT) -kernel $(KERNEL_ELF)

qemu-debug: $(KERNEL_ELF)
	$(QEMU) $(QEMU_OPT) -S -gdb tcp::$(QEMU_GDB_PORT) -kernel $(KERNEL_ELF)

$(KERNEL_ELF): $(KERNEL_SRCS) $(KERNEL_HDRS) $(KERNEL_LDSCRIPT) $(APP_OBJ) | $(BIN_DIR) $(MAP_DIR) $(OBJ_DIR)
	$(CC) $(CFLAGS) -Wl,-T$(KERNEL_SRC_DIR)/kernel.ld -Wl,-Map=$(MAP_DIR)/kernel.map -o $@ \
		$(KERNEL_SRCS) $(APP_OBJ)

$(APP_ELF): $(APP_SRCS) $(USER_SRC_DIR)/user.ld | $(BIN_DIR) $(MAP_DIR)
	$(CC) -std=c11 -O2 -g3 -Wall -Wextra --target=riscv32-unknown-elf -fuse-ld=lld -fno-stack-protector -ffreestanding -nostdlib \
		-Wl,-T$(USER_SRC_DIR)/user.ld -Wl,-Map=$(MAP_DIR)/user.map -o $@ \
		$(APP_SRCS)

$(APP_BIN): $(APP_ELF)
	$(OBJCOPY) --set-section-flags .bss=alloc,contents -O binary $< $@

$(APP_OBJ): $(APP_BIN) | $(OBJ_DIR)
	$(OBJCOPY) -Ibinary -Oelf32-littleriscv $< $@

$(BIN_DIR) $(MAP_DIR) $(OBJ_DIR):
	mkdir -p $@
