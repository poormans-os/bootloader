#pragma once

#include "mutex.h"
#include <Uefi.h>

extern EFI_SYSTEM_TABLE *SystemTable;

mutex_t printfMutex;
mutex_t scanfMutex;

int scanfPID; //In Use
unsigned char scanfBuffer;

int printf(const char *format, ...);
#define putchar(x) printf("%c", x);
void *memset(void *s, const int c, const size_t count);
size_t strlen(const char *str);

char *fgets(char *str, int n);
int scanf(const char *format, ...);
int kernelScanf();
