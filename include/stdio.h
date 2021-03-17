#pragma once

#include "mutex.h"
#include <Uefi.h>

extern EFI_SYSTEM_TABLE *SystemTable;

mutex_t printfMutex;
mutex_t scanfMutex;

int scanfPID; //In Use
unsigned char scanfBuffer;

long strtol(const char *nptr, char **endptr, unsigned int base);
char *convert(unsigned long long num, const int base);

int printf(const char *format, ...);
#define putchar(x) printf("%c", x);
void *memset(void *s, const int c, const size_t count);
size_t strlen(const char *str);

int scanf(const char *format, ...);
int kernel_scanf();
