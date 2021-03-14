#include "scheduler.h"

void TimerHandler(IN EFI_EVENT _, IN VOID *Context) //scheduler
{
    // printf("TESTS: ");
    // for (size_t i = 0; i < 5; i++)
    // {
    //     putchar(tests[i]);
    // }
    // putchar('\r');
    // putchar('\n');

    EFI_STATUS Status = EFI_SUCCESS;

    //////// Critical Code Section Begin ////////
    acquireMutex(&schedulerMtx);

    if (!procQueue)
    {
        releaseMutex(&schedulerMtx);
        return;
    }

    //Clean first available proccessor and assign a new task for it
    int coreNum = 0;
    for (coreNum = 0; coreNum < procInfo.numCores; coreNum++)
    {
        if (procInfo.procs[coreNum].status == TRUE)
        {
            if (procInfo.procs[coreNum].currentProc)
                free(procInfo.procs[coreNum].currentProc);

            procInfo.procs[coreNum].currentProc = procQueue;
            procQueue = procQueue->next;
            procInfo.procs[coreNum].status = FALSE;
            break;
        }
    }
    releaseMutex(&schedulerMtx);
    //////// Critical Code Section End ////////

    printf("Core Found %d, %d\r\n", coreNum + 1, procInfo.procs[coreNum].currentProc->pid);

    // Create an Event, required to call StartupThisAP in non-blocking mode
    if (Status == EFI_SUCCESS)
    {
        // Start a Task on the specified Processor.
        Status = MpProto->StartupThisAP(MpProto, (void *)procInfo.procs[coreNum].currentProc->regs.eip, coreNum + 1, procInfo.procs[coreNum].callingEvent, 0, procInfo.procs[coreNum].currentProc->args, (void *)&procInfo.procs[coreNum].status);
        if (Status == EFI_SUCCESS)
        {
            printf("Event Created On Core %d\r\n", coreNum + 1);
        }
        else
        {
            printf("Failed to start Task on CPU %d\r\n", coreNum + 1);
            printf("\r\nStatus %d\r\n", Status);
        }
    }
    else
    {
        printf("Event creation failed: %d\r\n", Status);
    }
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

    acquireMutex(&schedulerMtx);
    if (!last)
        procQueue = proc;
    else
    {
        while (last->next != NULL)
            last = last->next;
        last->next = proc;
    }
    releaseMutex(&schedulerMtx);

    return status;
}

EFI_STATUS initScheduler()
{
    UINTN NumEnabled = 0;
    UINTN NumProc = 0;
    EFI_PROCESSOR_INFORMATION Tcb = {0};

    // Find the MP Services Protocol
    EFI_STATUS Status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, (void **)&MpProto);
    if (Status != EFI_SUCCESS)
    {
        printf("Unable to locate the MpService procotol: %d\r\n", Status);
    }
    // Get Number of Processors and Number of Enabled Processors
    Status = MpProto->GetNumberOfProcessors(MpProto, &NumProc, &NumEnabled);
    if (Status != EFI_SUCCESS)
    {
        printf("Unable to get the number of processors: %d\r\n", Status);
    }
    printf("number of proccesors: %d\r\n", NumProc);
    // Get Processor Health and Location information

    for (size_t i = 0; i < NumProc; i++)
    {
        if (i != 0)
        {
            Status = MpProto->EnableDisableAP(MpProto, i, TRUE, NULL);
            if (Status != EFI_SUCCESS)
            {
                printf("Unable to get information for proc. %d: %d\r\n", i, Status);
                continue;
            }
        }
        Status = MpProto->GetProcessorInfo(MpProto, i, &Tcb);
        if (Status != EFI_SUCCESS)
        {
            printf("Unable to get information for proc. %d: %d\r\n", i, Status);
            continue;
        }
    }

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

    procInfo.numCores = NumProc - 1;

    Status = kmalloc(sizeof(proc_info_t) * procInfo.numCores, (void **)&(procInfo.procs));
    if (Status != EFI_SUCCESS)
        return Status;

    for (size_t i = 0; i < procInfo.numCores; i++)
    {
        procInfo.procs[i].currentProc = NULL; //(void *)-1;
        Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &procInfo.procs[i].callingEvent);
        if (Status != EFI_SUCCESS)
            printf("CreateEvent Failed %d\r\n", Status);

        procInfo.procs[i].status = TRUE;
    }
    return EFI_SUCCESS;
}
