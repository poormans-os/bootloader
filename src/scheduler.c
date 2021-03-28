#include "scheduler.h"

/**
 * @brief This function runs every second. It Checks if there are any tasks in the scheduler queue and 
 *        sends them to available cores to be executed.
 * 
 * @param _ UEFI must send an event, but it is not used here.
 * @param Context UEFI must send context, but it is not used here.
 */
void TimerHandler(IN EFI_EVENT _, IN VOID *Context)
{

    EFI_STATUS Status = EFI_SUCCESS; //Variable to check status of UEFI function results.

    //////// Critical Code Section Begin ////////
    acquireMutex(&schedulerMtx);

    if (!procQueue) //Scheduler queue is empty.
    {
        releaseMutex(&schedulerMtx);
        return;
    }

    //Clean first available proccessor and assign a new task for it.
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

    // Execute task on the available core by calling StartupThisAP in non-blocking mode.
    if (Status == EFI_SUCCESS)
    {
        // Start a Task on the specified Processor.
        Status = MpProto->StartupThisAP(MpProto, (void *)procInfo.procs[coreNum].currentProc->regs.eip, coreNum + 1, procInfo.procs[coreNum].callingEvent, 0, procInfo.procs[coreNum].currentProc->args, (void *)&procInfo.procs[coreNum].status);
        if (Status == EFI_SUCCESS)
            printf("Event Created On Core %d\r\n", coreNum + 1);
        else
        {
            printf("Failed to start Task on CPU %d\r\n", coreNum + 1);
            printf("\r\nStatus %d\r\n", Status);
        }
    }
    else
        printf("Event creation failed: %d\r\n", Status);
}

/**
 * @brief This function will add a new task to the scheduler queue.
 * 
 * @param func Task to add to the queue.
 * @param args Arguments of the task.
 * @return EFI_STATUS The status of the addition of a new task to the queue.
 */
EFI_STATUS addProcToQueue(void *func, void *args)
{
    proc_t *proc = NULL; //A new process.
    EFI_STATUS status = kmalloc(sizeof(proc_t), (void **)&proc);
    if (status != EFI_SUCCESS)
        return status;

    //Assigning values of the new task to the new process.
    pidCount++;
    memset(&(proc->regs), 0, sizeof(registers_t));
    proc->regs.eip = (INT64)func;
    proc->pid = pidCount;
    proc->args = args;
    proc->next = NULL;

    proc_t *last = procQueue;
    //Adding the new process to the queue.
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

/**
 * @brief This function initializes the scheduler.
 * 
 * @return EFI_STATUS Status of the initialization of the scheduler.
 */
EFI_STATUS initScheduler()
{
    schedulerMtx = 0; //Initializing the scheduler mutex to unlocked.

    //Initializing necessary variables.
    UINTN NumEnabled = 0;
    UINTN NumProc = 0;
    EFI_PROCESSOR_INFORMATION Tcb = {0};

    // Find the MP Services Protocol.
    EFI_STATUS Status = gBS->LocateProtocol(&gEfiMpServiceProtocolGuid, NULL, (void **)&MpProto);
    if (Status != EFI_SUCCESS)
        printf("Unable to locate the MpService procotol: %d\r\n", Status);
    // Get Number of Processors and Number of Enabled Processors.
    Status = MpProto->GetNumberOfProcessors(MpProto, &NumProc, &NumEnabled);
    if (Status != EFI_SUCCESS)
        printf("Unable to get the number of processors: %d\r\n", Status);
    printf("Number of proccesors: %d\r\n", NumProc);

    //Set all found cores to Application-Processor (AP), except for the first core who will be the Boot-Strap-Processor (BSP)
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
        Status = MpProto->GetProcessorInfo(MpProto, i, &Tcb); //Checking processor is good to work by getting its information.
        if (Status != EFI_SUCCESS)
        {
            printf("Unable to get information for proc. %d: %d\r\n", i, Status);
            continue;
        }
    }

    //Creating event for the TimerHandler function.
    Status = gBS->CreateEvent(
        EVT_TIMER | EVT_NOTIFY_SIGNAL, // Type
        TPL_NOTIFY,                    // NotifyTpl
        TimerHandler,                  // NotifyFunction
        Device,                        // NotifyContext
        &Device->PeriodicTimer         // Event
    );

    if (EFI_ERROR(Status))
        return Status;

    //Set timer so that TimerHandler will run every second.
    Status = gBS->SetTimer(
        Device->PeriodicTimer,
        TimerPeriodic,
        TIMER_PERIOD_MILLISECONDS(1000));
    if (EFI_ERROR(Status))
        return Status;

    //Allocate memory for the cores struct.
    Status = kmalloc(sizeof(proc_info_t), (void **)&procInfo);
    if (Status != EFI_SUCCESS)
        return Status;

    procInfo.numCores = NumProc - 1;
    //Allocate memory for the cores array.
    Status = kmalloc(sizeof(proc_info_t) * procInfo.numCores, (void **)&(procInfo.procs));
    if (Status != EFI_SUCCESS)
        return Status;

    //Create event for each core and set it to status available.
    for (size_t i = 0; i < procInfo.numCores; i++)
    {
        procInfo.procs[i].currentProc = NULL; //(void *)-1;
        Status = gBS->CreateEvent(0, TPL_APPLICATION, NULL, NULL, &procInfo.procs[i].callingEvent);
        if (Status != EFI_SUCCESS)
            printf("CreateEvent Failed %d\r\n", Status);

        procInfo.procs[i].status = TRUE;
    }
    return EFI_SUCCESS;
}
