#include <Uefi.h>
#include "stdio.h"

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    // EFI_STATUS result = -1;

    UINTN mapSize = 0, mapKey, descriptorSize;
    EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    UINT32 descriptorVersion = 1;

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    printf(SystemTable, "Hello world %d!\r\n", 5);
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    Status = SystemTable->BootServices->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion);
    if (EFI_ERROR(Status))
    {
        printf(SystemTable, "Get Map Error!\r\n");
        return Status;
    }

    Status = SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    if (EFI_ERROR(Status))
    {
        return Status;
    }

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
