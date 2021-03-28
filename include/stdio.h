#pragma once

#include "mutex.h"
#include <Uefi.h>

#define INT_MAX 2147483647 // Max 32bit int value

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

mutex_t printfMutex; //Mutexes
mutex_t scanfMutex;

int scanfPID;              //Flag Variable To Signal That A Function Is Waiting For Input
unsigned char scanfBuffer; //The Input Should Be Stored In This Variable

int printf(const char *format, ...);
#define putchar(x) printf("%c", x);
void *memset(void *s, const int c, const size_t count);
size_t strlen(const char *str);
unsigned char getchar();
char *fgets(char *str, int n);
int scanf(const char *format, ...);
int kernelScanf();
