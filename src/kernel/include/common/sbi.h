#pragma once

struct sbiret {
    long error;
    long value;
};

#define SBI_PUTCHAR 1
#define SBI_GETCHAR 2

struct sbiret sbi_call(
    long arg0,
    long arg1,
    long arg2,
    long arg3,
    long arg4,
    long arg5,
    long fid,
    long eid
);
