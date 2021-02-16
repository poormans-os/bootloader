#include <Uefi.h>
#include <Uefilib.h>
#include <PiMultiPhase.h>
#include <MpService.h>
#include "stdio.h"

#define kmalloc(x, y) gBS->AllocatePool(EfiReservedMemoryType, x, y); //IN* , OUT**
#define free(x) gBS->FreePool(x);
#define TIMER_PERIOD_MILLISECONDS(Milliseconds) (UINT64)(Milliseconds) * 10000

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

typedef int pid_t;
typedef long long register_t;

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

static UINT32 pidCount = 0;
static UINT32 coreCount = 1;
proc_t *pqueue;
proc_t *current_proc;

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
EFI_STATUS initScheduler();