#include <Uefi.h>
#include "stdio.h"

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    UINTN mapSize = 0, mapKey, descriptorSize;
    EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    UINT32 descriptorVersion = 1;

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    printf(SystemTable, "Disabling watchdog timer.\r\n");
    SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    
    
    while(EFI_SUCCESS != (Status = SystemTable->BootServices->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion)))
    {
        if(Status == EFI_BUFFER_TOO_SMALL)
        {
            printf(SystemTable, "Setting up memory map buffer.\r\n");
            mapSize += 2 * descriptorSize;
            SystemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void **)&memoryMap);
        } 
        else printf(SystemTable, "Error getting memory map: %d.\r\n", Status);
    }
    
    if (EFI_ERROR(Status))
    {
        printf(SystemTable, "Get Map Error!\r\n");
        return Status;
    }
    else
    {
        printf(SystemTable, "Memory map size: %d.\r\n", mapSize);
        printf(SystemTable, "Memory descriptor size: %d.\r\n", descriptorSize);
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
