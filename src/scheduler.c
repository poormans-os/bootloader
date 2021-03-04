#include "scheduler.h"

void TimerHandler(IN EFI_EVENT _, IN VOID *Context) //scheduler
{
    printf("TESTS: ");
    for (size_t i = 0; i < 5; i++)
    {
        putchar(tests[i]);
    }
    putchar('\r');
    putchar('\n');

    EFI_STATUS Status = EFI_SUCCESS;

    //////// Critical Code Section Begin ////////
    acquireMutex(&mutexes[0]);

    if (!procQueue)
        return;

    //Clean first available proccessor and assign a new task for it
    proc_t *procToFree = NULL;
    int coreNum = 0;
    for (coreNum = 0; coreNum < procInfo.numCores; coreNum++)
    {
        if (procInfo.procs[coreNum].status == TRUE)
        {
            if (procInfo.procs[coreNum].currentProc != NULL)
            {
                gBS->CloseEvent(procInfo.procs[coreNum].callingEvent);
                printf("Event close Successfully. %d\r\n", coreNum);
            }
            procToFree = procQueue;
            procInfo.procs[coreNum].currentProc = procQueue;
            procQueue = procQueue->next;
            procInfo.procs[coreNum].status = FALSE;
            break;
        }
    }

    releaseMutex(&mutexes[0]);
    //////// Critical Code Section End ////////

    printf("Core Found %d, %d\r\n", coreNum + 1, procInfo.procs[coreNum].currentProc->pid);

    // Create an Event, required to call StartupThisAP in non-blocking mode
    Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &procInfo.procs[coreNum].callingEvent);
    if (Status == EFI_SUCCESS)
    {
        // Start a Task on the specified Processor.
        Status = MpProto->StartupThisAP(MpProto, (void *)procInfo.procs[coreNum].currentProc->regs.eip, coreNum + 1, procInfo.procs[coreNum].callingEvent, 0, procInfo.procs[coreNum].currentProc->args, (void *)&procInfo.procs[coreNum].status);
        free(procToFree); //////////////////////////////////////////////// NOT SURE IF IT WORKS
        if (Status == EFI_SUCCESS)
        {
            printf("Event Created On Core %d\r\n", coreNum);
        }
        else
        {
            printf("Failed to start Task on CPU %d\r\n", coreNum);
            printf("\r\nStatus %d\r\n", Status);
        }
    }
    else
    {
        printf("Event creation failed: %d\r\n", Status);
    }
}

void acquireMutex(mutex_t *mutex)
{
    while (!__sync_bool_compare_and_swap(mutex, 0, 1))
        __asm__ volatile("pause");
    // printf("acquireMutex\r\n");
}

void releaseMutex(mutex_t *mutex)
{
    *mutex = 0;
    // printf("releaseMutex\r\n");
}

EFI_STATUS addProcToQueue(void *func, void *args)
{
    proc_t *proc = NULL;
    EFI_STATUS status = kmalloc(sizeof(proc_t), (void **)&proc);
    if (status != EFI_SUCCESS)
        return status;

    //memcpy
    pidCount++;
    memset(&(proc->regs), 0, sizeof(registers_t));
    proc->regs.eip = (INT64)func;
    proc->pid = pidCount;
    proc->args = args;
    proc->next = NULL;

    proc_t *last = procQueue;

    acquireMutex(&mutexes[0]);
    if (!last)
        procQueue = proc;
    else
    {
        while (last->next != NULL)
            last = last->next;
        last->next = proc;
    }
    releaseMutex(&mutexes[0]);

    return status;
}

EFI_STATUS initScheduler(UINTN CoreCount)
{
    EFI_STATUS Status;
    Status = gBS->CreateEvent(
        EVT_TIMER | EVT_NOTIFY_SIGNAL, // Type
        TPL_NOTIFY,                    // NotifyTpl
        TimerHandler,                  // NotifyFunction
        Device,                        // NotifyContext
        &Device->PeriodicTimer         // Event
    );

    if (EFI_ERROR(Status))
        return Status;

    Status = gBS->SetTimer(
        Device->PeriodicTimer,
        TimerPeriodic,
        TIMER_PERIOD_MILLISECONDS(1000));
    if (EFI_ERROR(Status))
        return Status;

    Status = kmalloc(sizeof(proc_info_t), (void **)&procInfo);
    if (Status != EFI_SUCCESS)
        return Status;

    procInfo.numCores = CoreCount - 1;

    Status = kmalloc(sizeof(proc_info_t) * procInfo.numCores, (void **)&(procInfo.procs));
    if (Status != EFI_SUCCESS)
        return Status;

    for (size_t i = 0; i < procInfo.numCores; i++)
    {
        procInfo.procs[i].currentProc = 0; //(void *)-1;
        procInfo.procs[i].status = TRUE;
        mutexes[i] = 0;
    }
    return EFI_SUCCESS;
}
