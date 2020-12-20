#pragma once

//#include <BaseMemoryLib.h>

extern EFI_SYSTEM_TABLE *SystemTable;

int printf(const char *restrict format, ...);
void *memset(void *s, const int c, const size_t count);
