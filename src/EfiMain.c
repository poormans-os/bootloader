#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"
#include "GraphicsOutput.h"

// typedef unsigned char uint8_t;
// typedef unsigned int uint32_t;
// typedef unsigned long uint64_t;

// const CHAR16 *memory_types[] =
//     {
//         L"EfiReservedMemoryType",
//         L"EfiLoaderCode",
//         L"EfiLoaderData",
//         L"EfiBootServicesCode",
//         L"EfiBootServicesData",
//         L"EfiRuntimeServicesCode",
//         L"EfiRuntimeServicesData",
//         L"EfiConventionalMemory",
//         L"EfiUnusableMemory",
//         L"EfiACPIReclaimMemory",
//         L"EfiACPIMemoryNVS",
//         L"EfiMemoryMappedIO",
//         L"EfiMemoryMappedIOPortSpace",
//         L"EfiPalCode",
// };

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
typedef struct PSF1_HEADER
{
    UINT8 magic[2];
    UINT8 mode;
    UINT8 charsize;
} PSF1_HEADER;

typedef struct PSF1_FONT
{
    PSF1_HEADER *psf1_header;
    void *glyphBuffer;
} PSF1_FONT;

PSF1_FONT *LoadPSF1Font(CHAR16 *Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_FILE_PROTOCOL *font = loadfile(Path, ImageHandle, SystemTable);
    if (font == NULL)
        return NULL;

    PSF1_HEADER *fontHeader;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void **)&fontHeader);
    UINTN size = sizeof(PSF1_HEADER);
    font->Read(font, &size, fontHeader);
    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1)
    {
        return NULL;
    }

    UINTN glyphBufferSize = fontHeader->charsize * 256;
    if (fontHeader->mode == 1)
    {
        glyphBufferSize = fontHeader->charsize * 512;
    }

    void *glyphBuffer;
    font->SetPosition(font, sizeof(PSF1_HEADER));
    SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void **)&glyphBuffer);
    font->Read(font, &glyphBufferSize, glyphBuffer);

    PSF1_FONT *finishedFont;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void **)&finishedFont);
    finishedFont->psf1_header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;
    return finishedFont;
}

typedef struct FRAMEBUFFER
{
    void *BaseAddress;
    UINT64 BufferSize;
    UINT32 Width;
    UINT32 Height;
    UINT32 PixelsPerScanLine;
} FRAMEBUFFER;

EFI_STATUS initGOP(EFI_SYSTEM_TABLE *SystemTable, FRAMEBUFFER *FrameBuffer)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = SystemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void **)&gop);
    if (status != EFI_SUCCESS)
        return status;
    FrameBuffer->BaseAddress = (void *)gop->Mode->FrameBufferBase;
    FrameBuffer->BufferSize = gop->Mode->FrameBufferSize;
    FrameBuffer->Width = gop->Mode->Info->HorizontalResolution;
    FrameBuffer->Height = gop->Mode->Info->VerticalResolution;
    FrameBuffer->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    return status;
}

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    FRAMEBUFFER *FrameBuffer = NULL;
    EFI_FILE_PROTOCOL *test = loadfile(L"test.txt", ImageHandle, SystemTable);

    void *testBuffer;
    UINT64 fsize = 0x00100000;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    printf(SystemTable, "RUNNING\r\n");
    if (test == NULL)
    {
        printf(SystemTable, "ERROR\r\n");
    }
    printf(SystemTable, "file addr: %x\r\n", test);

    SystemTable->BootServices->AllocatePool(EfiLoaderData, fsize, (void **)&testBuffer);
    test->Read(test, &fsize, testBuffer);
    test->Close(test);

    printf(SystemTable, "first 3 letter of \"/test\": %c%c%c", ((char *)testBuffer)[0], ((char *)testBuffer)[1], ((char *)testBuffer)[2]);
    printf(SystemTable, "\r\n");

    if (initGOP(SystemTable, FrameBuffer) != EFI_SUCCESS)
        printf(SystemTable, "GOP ERROR\r\n");

    PSF1_FONT *newFont = LoadPSF1Font(L"zap-light16.psf", ImageHandle, SystemTable);
    if (newFont == NULL)
        printf(SystemTable, "PSF1 ERROR\r\n");

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
