#include <Uefi.h>
#include "stdio.h"
#include "scheduler.h"

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;

void testPrint(char *s)
{
    printf("Hello Threading %s\r\n", s);
}

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    EFI_MP_SERVICES_PROTOCOL *MpProto = NULL;
    UINTN NumEnabled = 0;
    UINTN ProcNum = 0;
    UINTN NumProc = 0;
    EFI_PROCESSOR_INFORMATION Tcb = {0};

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

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

    for (size_t i = 0; i < ProcNum; i++)
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
        printf("%d: ProcID %d, Flags: 0x%x\r\n", i, Tcb.ExtendedInformation, Tcb.StatusFlag);
    }
    // printf("proccesor id: %d\r\nproccesor flags: %d\r\n", Tcb.ProcessorId, Tcb.StatusFlag);

    // void *Event = NULL;
    // //void *Procedure = testPrint;
    // //void *ProcedureArgument = "1";
    // // Create an Event, required to call StartupThisAP in non-blocking mode
    // Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &Event);
    // if (Status == EFI_SUCCESS)
    // {
    //     printf("Successful Event creation.\r\n");
    //     // Start a Task on the specified Processor.
    //     //Status = MpProto->StartupThisAP(MpProto, Procedure, 1, Event, 0, ProcedureArgument, NULL);
    //     Status = EFI_SUCCESS;
    //     if (Status == EFI_SUCCESS)
    //     {
    //         printf("Task successfully started.\r\n");
    //     }
    //     else
    //     {
    //         printf("Failed to start Task on CPU %d\r\n", ProcNum);
    //         printf("\r\nStatus %d\r\n", Status);
    //     }
    // }
    // else
    // {
    //     printf("Event creation failed: %d\r\n", Status);
    // }
    printf("Init\r\n");

    // EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    if (initScheduler(ProcNum) != EFI_SUCCESS)
        printf("Error\r\n");
    addProcToQueue(testPrint, "1");
    addProcToQueue(testPrint, "2");
    addProcToQueue(testPrint, "3");
    addProcToQueue(testPrint, "4");
    addProcToQueue(testPrint, "5");

    while (1)
    {
        SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
        SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
        printf("The value is %c", Key.UnicodeChar);
    }

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
