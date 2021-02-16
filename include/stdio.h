#pragma once

//#include <BaseMemoryLib.h>

#include <Uefi.h>

extern EFI_SYSTEM_TABLE *SystemTable;

int printf(const char *format, ...);
#define putchar(x) printf("%c", x);
void *memset(void *s, const int c, const size_t count);
