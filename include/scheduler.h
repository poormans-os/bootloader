#pragma once

#ifndef __SCHEDUEL_H
#define __SCHEDUEL_H

#include "stdio.h"
#include "mutex.h"
#include "utils.h"

#include <Uefi.h>
#include <Uefilib.h>
#include <PiMultiPhase.h>
#include <MpService.h>
#include <stdatomic.h>

#define TIMER_PERIOD_MILLISECONDS(Milliseconds) (UINT64)(Milliseconds) * 10000

EFI_MP_SERVICES_PROTOCOL *MpProto; //Handler for Multi-Processing.

extern EFI_SYSTEM_TABLE *SystemTable; //OS's System Table handler.
extern EFI_BOOT_SERVICES *gBS;        //OS's Boot Services handler.
mutex_t schedulerMtx;                 //Mutex to protect the scheduler queue.

typedef int pid_t;
typedef long long register_t;

//Struct of registers of a processor.
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

//Linked list of all processes (Scheduler Queue).
typedef struct proc_t
{
    pid_t pid;           //ID of the current process.
    registers_t regs;    //Registers of the current process.
    void *args;          //Arguments of the current process.
    struct proc_t *next; //Pointer to the next process in the queue.
} proc_t;

//Struct of a core (processor).
typedef struct
{
    pid_t pid;              //ID of the process run by this core.
    proc_t *currentProc;    //Pointer to the process run by this core.
    EFI_EVENT callingEvent; //Handler to the event of the core.
    UINTN status;           //Status of the core- Available(TRUE) \ Working(False).
} proc_info_t;

//Struct of all processors.
typedef struct
{
    UINTN numCores;     //Number of cores available to process tasks.
    proc_info_t *procs; //Array of cores.
} procs_info_t;

static UINT32 pidCount = 1;      //First value of PID.
static procs_info_t procInfo;    //Struct of all cores.
static proc_t *procQueue = NULL; //Process queue.

//Device to create events and set timer of scheduler
typedef struct
{
    UINTN Signature;
    EFI_EVENT PeriodicTimer;
    EFI_EVENT OneShotTimer;
    //
    // Other device specific fields
    //
} SCHEDULER_DEVICE;

SCHEDULER_DEVICE *Device;

void TimerHandler(IN EFI_EVENT Event, IN VOID *Context);
EFI_STATUS addProcToQueue(void *func, void *args);
EFI_STATUS initScheduler();

#endif
