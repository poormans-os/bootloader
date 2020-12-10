#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

const CHAR16 *memory_types[] = 
{
    L"EfiReservedMemoryType",
    L"EfiLoaderCode",
    L"EfiLoaderData",
    L"EfiBootServicesCode",
    L"EfiBootServicesData",
    L"EfiRuntimeServicesCode",
    L"EfiRuntimeServicesData",
    L"EfiConventionalMemory",
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
};

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_FILE_PROTOCOL* test = loadfile(L"test.txt", ImageHandle, SystemTable);
    if(test == NULL)
    {
        printf(SystemTable, "ERROR\r\n");
    }
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    // UINTN mapSize = 0, mapKey, descriptorSize;
    // EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    // UINT32 descriptorVersion = 1;

    // SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    // printf(SystemTable, "Disabling watchdog timer.\r\n");
    // SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
    
    
    // while(EFI_SUCCESS != (Status = SystemTable->BootServices->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion)))
    // {
    //     if(Status == EFI_BUFFER_TOO_SMALL)
    //     {
    //         printf(SystemTable, "Setting up memory map buffer.\r\n");
    //         mapSize += 2 * descriptorSize;
    //         SystemTable->BootServices->AllocatePool(EfiLoaderData, mapSize, (void **)&memoryMap);
    //     } 
    //     else printf(SystemTable, "Error getting memory map: %d.\r\n", Status);
    // }
    
    // if (EFI_ERROR(Status))
    // {
    //     printf(SystemTable, "Get Map Error!\r\n");
    //     return Status;
    // }
    // else
    // {
    //     printf(SystemTable, "Memory map size: %d.\r\n", mapSize);
    //     printf(SystemTable, "Memory descriptor size: %d.\r\n", descriptorSize);
    // }

    // Status = SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    // if (EFI_ERROR(Status))
    // {
    //     return Status;
    // }

    // uint8_t *startOfMemoryMap = (uint8_t *)memoryMap;
    // uint8_t *endOfMemoryMap = startOfMemoryMap + mapSize;

    // uint8_t *offset = startOfMemoryMap;

    // uint32_t counter = 0; 
    // uint64_t totalPages = 0;

    // EFI_MEMORY_DESCRIPTOR *desc = NULL;

    // while(offset < endOfMemoryMap)
    // {
    //     desc = (EFI_MEMORY_DESCRIPTOR *)offset;

    //     printf(SystemTable, "Map %d:\r\n", counter);
    //     printf(SystemTable, "  Type: %x, %s\r\n", desc->Type, memory_types[desc->Type]); 
    //     printf(SystemTable, "  PhysicalStart: %x\r\n", desc->PhysicalStart);
    //     printf(SystemTable, "  VirtualStart: %x\r\n", desc->VirtualStart);
    //     printf(SystemTable, "  NumberOfPages: %x   (4k)\r\n", desc->NumberOfPages);
    //     printf(SystemTable, "  Attribute: %x\r\n", desc->Attribute);

    //     totalPages += desc->NumberOfPages;

    //     offset += descriptorSize;
    //     counter++;
    //     while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
    //     ;
    // }

    // uint64_t memorySize = totalPages * 4096;
    // printf(SystemTable, "Memory detected: %d MB\n", memorySize / 1024 / 1024);

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
