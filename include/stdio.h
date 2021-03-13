#pragma once

#include "mutex.h"
//#include <BaseMemoryLib.h>

mutex_t printfMutex;

#include <Uefi.h>

extern EFI_SYSTEM_TABLE *SystemTable;

int printf(const char *format, ...);
#define putchar(x) printf("%c", x);
void *memset(void *s, const int c, const size_t count);
size_t strlen(const char *str);
