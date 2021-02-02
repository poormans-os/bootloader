#include <Uefi.h>
#include "stdio.h"

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

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

    // EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    while (1)
    {
        SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
        SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
        printf("%c", Key.UnicodeChar);
    }

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
