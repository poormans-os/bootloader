#include <Uefi.h>
#include "stdio.h"
#include <PiMultiPhase.h>
#include <MpService.h>

EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

UINTN globalTest = 0;

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;

// static UINT32 EfiTypeToStivaleType[] = {
//     [EfiReservedMemoryType] = RESERVED,
//     [EfiRuntimeServicesCode] = RESERVED,
//     [EfiRuntimeServicesData] = RESERVED,
//     [EfiMemoryMappedIO] = RESERVED,
//     [EfiMemoryMappedIOPortSpace] = RESERVED,
//     [EfiPalCode] = RESERVED,
//     [EfiUnusableMemory] = BAD_MEMORY,
//     [EfiACPIReclaimMemory] = ACPI_RECLAIM,
//     [EfiLoaderCode] = KERNEL_MODULES,
//     [EfiLoaderData] = KERNEL_MODULES,
//     [EfiBootServicesCode] = BOOTLODAER_RECLAIM,
//     [EfiBootServicesData] = BOOTLODAER_RECLAIM,
//     [EfiConventionalMemory] = USABLE,
//     [EfiACPIMemoryNVS] = ACPI_NVS};

void testPrint()
{
    globalTest = 1;
    printf("Hello Threading\r\n");
}

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    EFI_MP_SERVICES_PROTOCOL *MpProto = NULL;
    UINTN NumEnabled = 0;
    UINTN ProcNum = 0;
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
    Status = MpProto->GetNumberOfProcessors(MpProto, &ProcNum, &NumEnabled);
    if (Status != EFI_SUCCESS)
    {
        printf("Unable to get the number of processors: %d\r\n", Status);
    }
    // Get Processor Health and Location information
    Status = MpProto->GetProcessorInfo(MpProto, ProcNum, &Tcb);
    if (Status != EFI_SUCCESS)
    {
        printf("Unable to get information for proc. %d: %d\r\n", ProcNum, Status);
    }

    void *Event = NULL;
    void *Procedure = testPrint;
    void *ProcedureArgument = NULL;
    // Create an Event, required to call StartupThisAP in non-blocking mode
    Status = gBS->CreateEvent(0, TPL_NOTIFY, NULL, NULL, &Event);
    if (Status == EFI_SUCCESS)
    {
        printf("Successful Event creation.\r\n");
        // Start a Task on the specified Processor.
        Status = MpProto->StartupThisAP(MpProto, Procedure, ProcNum, Event, 0, ProcedureArgument, NULL);
        if (Status == EFI_SUCCESS)
        {
            printf("Task successfully started.\r\n");
        }
        else
        {
            printf("Failed to start Task on CPU %d\r\n", ProcNum);
            printf("Status %d\r\n", Status);
        }
    }
    else
    {
        printf("Event creation failed: %d\r\n", Status);
    }

    printf("ProcNum: %d\r\n", ProcNum);

    // EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    while (1)
    {
        SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
        SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
        printf("The value is %d, %c", globalTest, Key.UnicodeChar);
    }

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
