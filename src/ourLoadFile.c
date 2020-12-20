
#include <Uefi.h>
#include "LoadedImage.h"
#include "SimpleFileSystem.h"
#include "stdio.h"
#include "ourLoadFile.h"

EFI_FILE_PROTOCOL EFIAPI *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle)
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

void *EFIAPI ourAllocatePool(size_t size)
{
    void *addr = NULL;
    if (gBS->AllocatePool(EfiLoaderData, size, &addr) != EFI_SUCCESS)
        addr = 0;
    return addr;
}

EFI_STATUS EFIAPI FileRead(EFI_FILE_HANDLE file, void *dest, size_t size, size_t offset)
{
    EFI_STATUS status = file->SetPosition(file, offset);
    if (status != EFI_SUCCESS)
        return status;
    return file->Read(file, &size, dest);
}

void EFIAPI ZeroMem(void *addr, size_t size)
{
    if (size <= 0)
        return;
    if (addr == NULL)
        return;

    memset(addr, 0, size);
}

EFI_STATUS LoadElf64(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs, CHAR16 *file, ELF_INFO *info)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_FILE_PROTOCOL *root = NULL;
    EFI_FILE_PROTOCOL *elfFile = NULL;

    //CHECK(info != NULL);
    info->PhysicalBase = MAX_INT64;

    // open the executable file
    fs->OpenVolume(fs, &root);                               //EFI_CHECK(fs->OpenVolume(fs, &root));
    root->Open(root, &elfFile, file, EFI_FILE_MODE_READ, 0); //EFI_CHECK(root->Open(root, &elfFile, file, EFI_FILE_MODE_READ, 0));

    // read the header
    Elf64_Ehdr ehdr;
    FileRead(elfFile, &ehdr, sizeof(Elf64_Ehdr), 0); //CHECK_AND_RETHROW(FileRead(elfFile, &ehdr, sizeof(Elf64_Ehdr), 0));

    // verify is an elf
    if (!IS_ELF(ehdr))
        return -1; //CHECK(IS_ELF(ehdr));

    // verify the elf type
    // ehdr.e_ident[EI_VERSION] == EV_CURRENT; //CHECK(ehdr.e_ident[EI_VERSION] == EV_CURRENT);
    // ehdr.e_ident[EI_CLASS] == ELFCLASS64;   //CHECK(ehdr.e_ident[EI_CLASS] == ELFCLASS64);
    // ehdr.e_ident[EI_DATA] == ELFDATA2LSB;   //CHECK(ehdr.e_ident[EI_DATA] == ELFDATA2LSB);

    // Load from section headers
    Elf64_Phdr phdr;
    for (int i = 0; i < ehdr.e_phnum; i++)
    {
        FileRead(elfFile, &phdr, sizeof(Elf64_Phdr), ehdr.e_phoff + ehdr.e_phentsize * i); //CHECK_AND_RETHROW(FileRead(elfFile, &phdr, sizeof(Elf64_Phdr), ehdr.e_phoff + ehdr.e_phentsize * i));

        switch (phdr.p_type)
        {
        // normal section
        case PT_LOAD:
            // ignore empty sections
            if (phdr.p_memsz == 0)
                continue;

            // get the type and pages to allocate
            EFI_MEMORY_TYPE MemType = (phdr.p_flags & PF_X) ? EfiLoaderCode : EfiLoaderData;
            UINTN nPages = EFI_SIZE_TO_PAGES(ALIGN_VALUE(phdr.p_memsz, EFI_PAGE_SIZE));

            // allocate the address
            EFI_PHYSICAL_ADDRESS base = info->VirtualOffset ? phdr.p_vaddr - info->VirtualOffset : phdr.p_paddr;
            //TRACE("    BASE = %p, PAGES = %d", base, nPages);
            status = EFI_ERROR(gBS->AllocatePages(AllocateAnyPages, MemType, nPages, &base)); //EFI_CHECK(gBS->AllocatePages(AllocateAddress, MemType, nPages, &base));
            if (status != EFI_SUCCESS)
                return status;

            FileRead(elfFile, (void *)base, phdr.p_filesz, phdr.p_offset); //CHECK_AND_RETHROW(FileRead(elfFile, (void *)base, phdr.p_filesz, phdr.p_offset));
            ZeroMem((void *)(base + phdr.p_filesz), phdr.p_memsz - phdr.p_filesz);

            if (info->PhysicalBase > base)
                info->PhysicalBase = base;

        // ignore entry
        default:
            break;
        }
    }

    // copy the section headers
    info->SectionHeadersSize = ehdr.e_shnum * ehdr.e_shentsize;
    info->SectionHeaders = ourAllocatePool(info->SectionHeadersSize); // TODO: Delete if error
    info->SectionEntrySize = ehdr.e_shentsize;
    info->StringSectionIndex = ehdr.e_shstrndx;
    FileRead(elfFile, info->SectionHeaders, info->SectionHeadersSize, ehdr.e_shoff); //CHECK_AND_RETHROW(FileRead(elfFile, info->SectionHeaders, info->SectionHeadersSize, ehdr.e_shoff));

    // copy the entry
    info->Entry = ehdr.e_entry;
    if (ehdr.e_entry > 0xffffffff80000000)
        info->VirtualOffset = 0xffffffff80000000;

    // if (root != NULL)
    // {
    //     FileHandleClose(root);
    // }

    // if (elfFile != NULL)
    // {
    //     FileHandleClose(elfFile);
    // }

    return status;
}

PSF1_FONT EFIAPI *LoadPSF1Font(IN CHAR16 *Path, IN EFI_HANDLE ImageHandle)
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

EFI_STATUS EFIAPI EnterBestGraphicMode()
{
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
    UINTN sizeOfInfo = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    gBS->LocateProtocol(&gopGuid, NULL, (VOID **)&gop);

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

EFI_STATUS EFIAPI initGOP(IN FRAMEBUFFER *FrameBuffer)
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

EFI_STATUS EFIAPI ElfLoadImage(IN CONST void *ElfImage, OUT void **EntryPoint)
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
            //MemSegment = (VOID *)ProgramHdrPtr->p_vaddr;
            MemSegment = ourAllocatePool(ProgramHdrPtr->p_filesz);
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

EFI_STATUS EFIAPI loadKernel(IN CHAR16 *path, IN EFI_HANDLE ImageHandle, OUT void **buffer)
{
    UINT64 fsize = 0x00100000;
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
