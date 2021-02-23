#pragma once

#ifndef __SCHEDUEL_H
#define __SCHEDUEL_H

#include <Uefi.h>
#include <Uefilib.h>
#include <PiMultiPhase.h>
#include <MpService.h>
#include "stdio.h"
#include <stdatomic.h>

#define kmalloc(x, y) gBS->AllocatePool(EfiReservedMemoryType, x, y) //IN* , OUT**
#define free(x) gBS->FreePool(x)
#define TIMER_PERIOD_MILLISECONDS(Milliseconds) (UINT64)(Milliseconds) * 10000

EFI_MP_SERVICES_PROTOCOL *MpProto;

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

extern void testPrint(char *s);

typedef int pid_t;
typedef long long register_t;

typedef volatile int mutex_t;

typedef struct
{
    register_t rax;
    register_t rbx;
    register_t rcx;
    register_t rdx;
    register_t eflags;
    register_t cs;
    register_t ds;
    register_t ss;
    register_t eip;
} registers_t;

typedef struct proc_t
{
    pid_t pid;
    registers_t regs;
    //void *memLower;
    //void *memUpper;
    void *args;
    struct proc_t *next;
} proc_t;

typedef struct
{
    pid_t pid;
    proc_t *currentProc;
    EFI_EVENT callingEvent;
    UINTN status;
} proc_info_t;

typedef struct
{
    UINTN numCores;
    proc_info_t *procs;
} procs_info_t;

static UINT32 pidCount = 1;
static procs_info_t procInfo;
static proc_t *pqueue = NULL;

typedef struct
{
    UINTN Signature;
    EFI_EVENT PeriodicTimer;
    EFI_EVENT OneShotTimer;
    //
    // Other device specific fields
    //
} EXAMPLE_DEVICE;

EXAMPLE_DEVICE *Device;

void TimerHandler(IN EFI_EVENT Event, IN VOID *Context);
EFI_STATUS addProcToQueue(void *func, void *args);
EFI_STATUS initScheduler(UINTN CoreCount);

mutex_t mutexes[4];

void acquireMutex(mutex_t *mutex);
void releaseMutex(mutex_t *mutex);

#endif
