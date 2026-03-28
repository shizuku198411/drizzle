## QEMU Standby boot
```
make qemu-debug
```

## Connect QEMU via GDB
```
gdb-multiarch ./bin/kernel.elf

# gdb console
# turn pagination and confirm off
set pagination off
set confirm off

# set architecture
set architecture riscv:rv32

# connect to qemu
target remote :1234
```

## GDB commands
```
# set breakpoint
b path/to/target:linenum

# continue
c

# print
p <variable>

# print address
p/x <variable>

# backtrace
bt

# watch
watch <variable>
```