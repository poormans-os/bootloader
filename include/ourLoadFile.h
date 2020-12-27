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

typedef struct
{
    UINT64 Base;
    UINT64 Length;
    UINT32 Type;
    UINT32 Unused;
} MMAP_ENTRY;

typedef struct
{
    UINT64 Cmdline;
    UINT64 MemoryMapAddr;
    UINT64 MemoryMapEntries;
    UINT64 FramebufferAddr;
    UINT16 FramebufferPitch;
    UINT16 FramebufferWidth;
    UINT16 FramebufferHeight;
    UINT16 FramebufferBpp;
    UINT64 Rsdp;
    UINT64 ModuleCount;
    UINT64 Modules;
    UINT64 Epoch;
    UINT64 Flags;
#define STIVALE_STRUCT_BIOS BIT0
} INFO_STRUCT;

#define USABLE 1
#define RESERVED 2
#define ACPI_RECLAIM 3
#define ACPI_NVS 4
#define BAD_MEMORY 5
#define KERNEL_MODULES 10
#define BOOTLODAER_RECLAIM 0x1000

extern EFI_SYSTEM_TABLE *SystemTable;
extern EFI_BOOT_SERVICES *gBS;

EFI_FILE_PROTOCOL EFIAPI *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle);
void *EFIAPI ourAllocatePool(size_t size);
PSF1_FONT EFIAPI *LoadPSF1Font(IN CHAR16 *Path, IN EFI_HANDLE ImageHandle);
EFI_STATUS EFIAPI EnterBestGraphicMode();
EFI_STATUS EFIAPI initGOP(IN FRAMEBUFFER *FrameBuffer);
EFI_STATUS EFIAPI ElfLoadImage(IN CONST void *ElfImage, OUT void **EntryPoint);
EFI_STATUS EFIAPI loadKernel(IN CHAR16 *path, IN EFI_HANDLE ImageHandle, OUT void **buffer);
EFI_STATUS EFIAPI LoadElf64(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs, CHAR16 *file, ELF_INFO *info);
void *EFIAPI ourAllocateReservedPages(IN UINTN Pages);
// UINTN EFIAPI ourAsmReadCr3();
unsigned long long ourAsmReadCr3(void);
unsigned long long ourAsmReadCr0(void);
void ourAsmWriteCr0(unsigned long long value);
VOID *EFIAPI ourAllocateReservedPool(IN UINTN AllocationSize);