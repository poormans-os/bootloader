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

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

EFI_FILE_PROTOCOL *loadfile(CHAR16 *path, EFI_HANDLE ImageHandle);
PSF1_FONT *LoadPSF1Font(CHAR16 *Path, EFI_HANDLE ImageHandle);
EFI_STATUS EnterBestGraphicMode();
EFI_STATUS initGOP(FRAMEBUFFER *FrameBuffer);
EFI_STATUS EFIAPI ElfLoadImage(const void *ElfImage, void **EntryPoint);
EFI_STATUS loadKernel(CHAR16 *path, void **buffer, EFI_HANDLE ImageHandle);