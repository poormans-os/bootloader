#include "scheduler.h"

void TimerHandler(IN EFI_EVENT _, IN VOID *Context) //scheduler
{
    proc_t *current_proc = NULL;

    // if (current_proc->next != NULL)
    //     current_proc = current_proc->next;
    // else
    // {
    //     current_proc = pqueue;
    //     pqueue = pqueue->next;
    // }
    current_proc = pqueue;
    if (!current_proc)
        return;

    EFI_STATUS Status = EFI_SUCCESS;

    //////// Critical Code Section Begin ////////

    acquireMutex(&mutexes[0]);
    //Clean Proccesses
    for (size_t i = 0; i < procInfo.numCores; i++)
    {
        proc_t *temp = NULL;
        int found = 0;
        if (procInfo.procs[i].status == TRUE)
        {
            //Remove the proc with the same pid
#pragma region find
            if (current_proc == procInfo.procs[i].currentProc)
            {
                found = 1;
                temp = current_proc;
            }
            else
            {
                while (current_proc->next || !found)
                {
                    if (current_proc == procInfo.procs[i].currentProc)
                    {
                        temp = current_proc->next;
                        current_proc->next = temp->next ? temp->next : NULL;
                        found = 1;
                    }
                    else
                        current_proc = current_proc->next;
                }
            }
#pragma endregion find
            if (found)
            {
                procInfo.procs[i].status = FALSE;
                procInfo.procs[i].currentProc = 0;
                gBS->CloseEvent(procInfo.procs[i].callingEvent);
                current_proc = current_proc->next;
                printf("Event close Successfully. %d\r\n", i);
                //Free The struct, Stored at procInfo.procs[i].currentProc
            }
            else
                //The problem is with the fact that the proc with the pid should have just stopped running, And thats a problem
                printf("PROBLEM\r\n");
        }
    }

    //Select A proc
    int coreNum = 0;
    for (coreNum = 0; coreNum < procInfo.numCores; coreNum++)
    {
        if (procInfo.procs[coreNum].currentProc == 0)
        {
            procInfo.procs[coreNum].currentProc = current_proc;
            break;
        }
    }

    releaseMutex(&mutexes[0]);
    //////// Critical Code Section End ////////

    if (!current_proc)
        return;

    // int coreNum = 0;
    // for (coreNum = 0; coreNum < procInfo.numCores; coreNum++)
    // {
    //     if (procInfo.cores[coreNum] == 0) //core is not used
    //     {
    //         procInfo.cores[coreNum] = 1;
    //         break;
    //     }
    // }

    //No core was found
    if (coreNum == procInfo.numCores)
    {
        //printf("No Core was found\r\n");
        return;
    }
    printf("Core Found %d, 0x%x 0x%x, %d\r\n", coreNum, procInfo.procs[coreNum].currentProc->regs.eip, testPrint, current_proc->pid);

    // Create an Event, required to call StartupThisAP in non-blocking mode
    Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &procInfo.procs[coreNum].callingEvent);
    if (Status == EFI_SUCCESS)
    {
        // Start a Task on the specified Processor.
        Status = MpProto->StartupThisAP(MpProto, (void *)current_proc->regs.eip, coreNum + 1, procInfo.procs[coreNum].callingEvent, 0, current_proc->args, (void *)&procInfo.procs[coreNum].status);

        if (Status == EFI_SUCCESS)
        {
            printf("Event Created On Core %d\r\n", coreNum);
            //remove task from queue
            // proc_t *tmp = pqueue;
            // pqueue = pqueue->next;
            // free(tmp);
            //Status = gBS->CloseEvent(Event);
            // if (Status == EFI_SUCCESS)
            // {
            //     procInfo.cores[coreNum] = 0;
            // }
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
    printf("acquireMutex\r\n");
}

void releaseMutex(mutex_t *mutex)
{
    *mutex = 0;
    printf("releaseMutex\r\n");
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

    proc_t *last = pqueue;

    acquireMutex(&mutexes[0]);
    if (!last)
        pqueue = proc;
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
        procInfo.procs[i].status = FALSE;
        mutexes[i] = 0;
    }

    // Status = kmalloc(sizeof(int) * procInfo.numCores, (void **)&(procInfo.cores));
    // if (Status != EFI_SUCCESS)
    //     return Status;

    // Status = kmalloc(sizeof(int) * procInfo.numCores, (void **)&(procInfo.finished));
    // if (Status != EFI_SUCCESS)
    //     return Status;

    // for (int i = 0; i < procInfo.numCores; i++)
    // {
    //     procInfo.cores[i] = 0;
    //     procInfo.finished[i] = 0;
    // }

    return EFI_SUCCESS;
}
