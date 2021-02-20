#include "scheduler.h"

void TimerHandler(IN EFI_EVENT _, IN VOID *Context) //scheduler
{
    EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

    // if (current_proc->next != NULL)
    //     current_proc = current_proc->next;
    // else
    // {
    //     current_proc = pqueue;
    //     pqueue = pqueue->next;
    // }
    current_proc = pqueue;

    if (!current_proc)
    {
        return;
    }

    void *Event = NULL;
    EFI_MP_SERVICES_PROTOCOL *MpProto = NULL;
    EFI_STATUS Status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, (void **)&MpProto);
    int coreNum = -1;
    for (int i = 0; i < procInfo.numCores; i++)
    {
        if (procInfo.cores[i] == 0) //core is not used
        {
            procInfo.cores[i] = 1;
            coreNum = i;
            break;
        }
    }
    if (coreNum != -1)
    {
        // Create an Event, required to call StartupThisAP in non-blocking mode
        Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &Event);
        if (Status == EFI_SUCCESS)
        {
            // Start a Task on the specified Processor.
            Status = MpProto->StartupThisAP(MpProto, (void *)current_proc->regs.eip, coreNum, Event, 0, current_proc->args, NULL);
            Status = EFI_SUCCESS;
            if (Status == EFI_SUCCESS)
            {
                //remove task from queue
                proc_t *tmp = pqueue;
                pqueue = pqueue->next;
                free(tmp);
                Status = gBS->CloseEvent(Event);
                if (Status == EFI_SUCCESS)
                {
                    procInfo.cores[coreNum] = 0;
                    printf("Event close Successfully. %d\r\n", coreNum);
                }
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
    else
    {
        printf("No core available.\r\n");
    }
}

EFI_STATUS addProcToQueue(void *func, void *args)
{
    proc_t *proc = NULL;
    EFI_STATUS status = kmalloc(sizeof(proc_t), (void **)&proc);
    if (status != EFI_SUCCESS)
        return status;

    //memcpy
    memset(&(proc->regs), 0, sizeof(registers_t));
    proc->regs.eip = (INT64)func;
    proc->pid = ++pidCount;
    proc->args = args;
    proc->next = NULL;

    proc_t *last = pqueue;
    if (!last)
    {
        pqueue = proc;
        return status;
    }
    while (last->next != NULL)
        last = last->next;
    last->next = proc;

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
        TIMER_PERIOD_MILLISECONDS(100));
    if (EFI_ERROR(Status))
        return Status;

    Status = kmalloc(sizeof(proc_info_t), (void **)&procInfo);
    if (Status != EFI_SUCCESS)
        return Status;

    procInfo.numCores = CoreCount - 1;

    Status = kmalloc(sizeof(int) * procInfo.numCores, (void **)&(procInfo.cores));
    if (Status != EFI_SUCCESS)
        return Status;
    printf("2\r\n");

    for (int i = 0; i < procInfo.numCores; i++)
    {
        procInfo.cores[i] = 0;
    }
    return EFI_SUCCESS;
}
