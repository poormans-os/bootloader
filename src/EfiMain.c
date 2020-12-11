#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"
#include "GraphicsOutput.h"
#include <elf_common.h>
#include <elf64.h>

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

EFI_STATUS EnterBestGraphicMode(EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
    UINTN sizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

    INTN i;
    INT32 bestOption;
    INT32 bestWidth = 0;
    INT32 bestHeight = 0;
    for (i = 0; i < gop->Mode->MaxMode; i++)
    {
        gop->QueryMode(gop, i, &sizeOfInfo, &info);
        if (info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor)
        {
            continue;
        }
        else if (info->VerticalResolution > bestHeight && info->HorizontalResolution > bestWidth)
        {
            bestWidth = info->HorizontalResolution;
            bestHeight = info->VerticalResolution;
            bestOption = i;
        }
    }

    return gop->SetMode(gop, bestOption);
}

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

EFI_STATUS EFIAPI ElfLoadImage(EFI_SYSTEM_TABLE *SystemTable, const void *ElfImage, void **EntryPoint)
{
    Elf64_Ehdr *ElfHdr;
    UINT8 *ProgramHdr;
    Elf64_Phdr *ProgramHdrPtr;
    UINTN Index;
    UINT8 IdentMagic[4] = {0x7f, 0x45, 0x4c, 0x46};

    ElfHdr = (Elf64_Ehdr *)ElfImage;
    ProgramHdr = (UINT8 *)ElfImage + ElfHdr->e_phoff;

    for (Index = 0; Index < 4; Index++)
    {
        if (ElfHdr->e_ident[Index] != IdentMagic[Index])
        {
            return EFI_INVALID_PARAMETER;
        }
    }

    // Load every loadable ELF segment into memory
    for (Index = 0; Index < ElfHdr->e_phnum; Index++)
    {
        ProgramHdrPtr = (Elf64_Phdr *)ProgramHdr;

        // Only consider PT_LOAD type segments
        if (ProgramHdrPtr->p_type == 1)
        {
            VOID *FileSegment;
            VOID *MemSegment;
            VOID *ExtraZeroes;
            UINTN ExtraZeroesCount;

            // Load the segment in memory
            FileSegment = (VOID *)((UINTN)ElfImage + ProgramHdrPtr->p_offset);
            MemSegment = (VOID *)ProgramHdrPtr->p_vaddr;
            SystemTable->BootServices->CopyMem(MemSegment, FileSegment, ProgramHdrPtr->p_filesz);

            // Fill memory with zero for .bss section and ...
            ExtraZeroes = (UINT8 *)MemSegment + ProgramHdrPtr->p_filesz;
            ExtraZeroesCount = ProgramHdrPtr->p_memsz - ProgramHdrPtr->p_filesz;
            if (ExtraZeroesCount > 0)
            {
                SystemTable->BootServices->SetMem(ExtraZeroes, 0x00, ExtraZeroesCount);
            }
        }

        // Get next program header
        ProgramHdr += ElfHdr->e_phentsize;
    }

    *EntryPoint = (VOID *)ElfHdr->e_entry;
    return (EFI_SUCCESS);
}

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    FRAMEBUFFER *FrameBuffer = NULL;
    EFI_FILE_PROTOCOL *kernel = loadfile(L"pmos.bin", ImageHandle, SystemTable);

    void *kernelBuffer;
    UINT64 fsize = 0x00100000;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    printf(SystemTable, "RUNNING\r\n");
    if (kernel == NULL)
        printf(SystemTable, "ERROR\r\n");
    printf(SystemTable, "file addr: %x\r\n", kernel);

    SystemTable->BootServices->AllocatePool(EfiLoaderData, fsize, (void **)&kernelBuffer);
    kernel->Read(kernel, &fsize, kernelBuffer);
    kernel->Close(kernel);

    void *entryPoint = NULL;
    if (ElfLoadImage(SystemTable, kernelBuffer, &entryPoint) != EFI_SUCCESS)
        printf(SystemTable, "ENTRY POINT ERROR\r\n");

    if (EnterBestGraphicMode(SystemTable) != EFI_SUCCESS)
        printf(SystemTable, "GRAPHICS ERROR\r\n");

    if (initGOP(SystemTable, FrameBuffer) != EFI_SUCCESS)
        printf(SystemTable, "GOP ERROR\r\n");

    PSF1_FONT *newFont = LoadPSF1Font(L"zap-light16.psf", ImageHandle, SystemTable);
    if (newFont == NULL)
        printf(SystemTable, "PSF1 ERROR\r\n");

    void (*entryPointFun)(FRAMEBUFFER *) = entryPoint;

    entryPointFun(FrameBuffer);
    printf(SystemTable, "STILL RUNNING\r\n");
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, 0);

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
