#pragma once
#include <SimpleFileSystem.h>
#include <elf_common.h>
#include <elf64.h>
#include <Uefi.h>
#include <LoadedImage.h>
#include <GraphicsOutput.h>

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

typedef struct FRAMEBUFFER
{
    void *BaseAddress;
    UINT64 BufferSize;
    UINT32 Width;
    UINT32 Height;
    UINT32 PixelsPerScanLine;
} FRAMEBUFFER;

typedef struct _ELF_INFO
{
    // will subtract this value from the Virtual address
    // if zero then physical address is used
    UINT64 VirtualOffset;
    EFI_PHYSICAL_ADDRESS PhysicalBase;

    // The entry of the image
    UINTN Entry;

    // section entry info
    void *SectionHeaders;
    UINTN SectionHeadersSize;
    UINTN SectionEntrySize;
    UINTN StringSectionIndex;
} ELF_INFO;

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

EFI_FILE_PROTOCOL EFIAPI *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle);
PSF1_FONT EFIAPI *LoadPSF1Font(IN CHAR16 *Path, IN EFI_HANDLE ImageHandle);
EFI_STATUS EFIAPI EnterBestGraphicMode();
EFI_STATUS EFIAPI initGOP(IN FRAMEBUFFER *FrameBuffer);
EFI_STATUS EFIAPI ElfLoadImage(IN CONST void *ElfImage, OUT void **EntryPoint);
EFI_STATUS EFIAPI loadKernel(IN CHAR16 *path, IN EFI_HANDLE ImageHandle, OUT void **buffer);
EFI_STATUS EFIAPI LoadElf64(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs, CHAR16 *file, ELF_INFO *info);
void *EFIAPI outAllocateReservedPages(IN UINTN Pages);
UINTN EFIAPI ourAsmReadCr3();
