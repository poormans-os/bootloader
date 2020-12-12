
#include <Uefi.h>
#include "LoadedImage.h"
#include "SimpleFileSystem.h"
#include "stdio.h"
#include "ourLoadFile.h"

EFI_FILE_PROTOCOL *loadfile(CHAR16 *path, EFI_HANDLE ImageHandle)
{
    EFI_GUID protocol1 = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID protocol2 = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID protocol3 = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    gBS->HandleProtocol(ImageHandle, &protocol1, (void **)&LoadedImage);

    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    gBS->HandleProtocol(LoadedImage->DeviceHandle, &protocol2, (void **)&DevicePath);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    gBS->HandleProtocol(LoadedImage->DeviceHandle, &protocol3, (void **)&Volume);

    EFI_FILE_PROTOCOL *RootFS;
    Volume->OpenVolume(Volume, &RootFS);

    EFI_FILE_PROTOCOL *CurDir = RootFS;

    EFI_FILE_PROTOCOL *FileHandle;
    EFI_STATUS Status = CurDir->Open(CurDir, &FileHandle, path, EFI_FILE_MODE_READ, 0);

    if (Status != EFI_SUCCESS)
        return NULL;
    return FileHandle;
}

PSF1_FONT *LoadPSF1Font(CHAR16 *Path, EFI_HANDLE ImageHandle)
{
    EFI_FILE_PROTOCOL *font = loadfile(Path, ImageHandle);
    if (font == NULL)
        return NULL;

    PSF1_HEADER *fontHeader;
    gBS->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void **)&fontHeader);
    UINTN size = sizeof(PSF1_HEADER);
    font->Read(font, &size, fontHeader);
    if (fontHeader->magic[0] != PSF1_MAGIC0 || fontHeader->magic[1] != PSF1_MAGIC1)
        return NULL;

    UINTN glyphBufferSize = fontHeader->charsize * 256;
    if (fontHeader->mode == 1)
        glyphBufferSize = fontHeader->charsize * 512;

    void *glyphBuffer;
    font->SetPosition(font, sizeof(PSF1_HEADER));
    gBS->AllocatePool(EfiLoaderData, glyphBufferSize, (void **)&glyphBuffer);
    font->Read(font, &glyphBufferSize, glyphBuffer);

    PSF1_FONT *finishedFont;
    gBS->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void **)&finishedFont);
    finishedFont->psf1_header = fontHeader;
    finishedFont->glyphBuffer = glyphBuffer;
    return finishedFont;
}

EFI_STATUS EnterBestGraphicMode()
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
            continue;
        else if (info->VerticalResolution > bestHeight && info->HorizontalResolution > bestWidth)
        {
            bestWidth = info->HorizontalResolution;
            bestHeight = info->VerticalResolution;
            bestOption = i;
        }
    }

    return gop->SetMode(gop, bestOption);
}

EFI_STATUS initGOP(FRAMEBUFFER *FrameBuffer)
{
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = gBS->LocateProtocol(&gopGuid, NULL, (void **)&gop);
    if (status != EFI_SUCCESS)
        return status;
    FrameBuffer->BaseAddress = (void *)gop->Mode->FrameBufferBase;
    FrameBuffer->BufferSize = gop->Mode->FrameBufferSize;
    FrameBuffer->Width = gop->Mode->Info->HorizontalResolution;
    FrameBuffer->Height = gop->Mode->Info->VerticalResolution;
    FrameBuffer->PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;
    return status;
}

EFI_STATUS EFIAPI ElfLoadImage(const void *ElfImage, void **EntryPoint)
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
            gBS->CopyMem(MemSegment, FileSegment, ProgramHdrPtr->p_filesz);

            // Fill memory with zero for .bss section and ...
            ExtraZeroes = (UINT8 *)MemSegment + ProgramHdrPtr->p_filesz;
            ExtraZeroesCount = ProgramHdrPtr->p_memsz - ProgramHdrPtr->p_filesz;
            if (ExtraZeroesCount > 0)
                gBS->SetMem(ExtraZeroes, 0x00, ExtraZeroesCount);
        }

        // Get next program header
        ProgramHdr += ElfHdr->e_phentsize;
    }

    *EntryPoint = (VOID *)ElfHdr->e_entry;
    return (EFI_SUCCESS);
}

EFI_STATUS loadKernel(CHAR16 *path, void **buffer, EFI_HANDLE ImageHandle)
{
    UINT64 fsize = 0x01000000;
    EFI_FILE_PROTOCOL *file = loadfile(L"pmos.bin", ImageHandle);

    if (file == NULL)
        return -1;

    gBS->AllocatePool(EfiLoaderData, fsize, buffer);
    file->Read(file, &fsize, *buffer);
    file->Close(file);

    if (*buffer == NULL)
        return -1;

    return EFI_SUCCESS;
}
