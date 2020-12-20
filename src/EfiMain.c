#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"
#include <MemoryAllocationLib.h>
#include <MemLibInternals.h>
#include <SetMemWrapper.c>

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;

UINTN
EFIAPI
myAsmReadCr3 (
    VOID
  )
{
    unsigned long cr3;
  __asm__ volatile(
      "mov %%cr3, %%rax"
      : "=m" (cr3)
      : /* no input */
      : "%rax"
      );
      return cr3;
}

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    UINTN mapSize = 0, mapKey, descriptorSize;
    EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    UINT32 descriptorVersion = 1;

    FRAMEBUFFER *FrameBuffer = NULL;
    // void *kernelBuffer = NULL;
    void *entryPoint = NULL;
    PSF1_FONT *font = LoadPSF1Font(L"zap-light16.psf", ImageHandle);

    if (EnterBestGraphicMode() != EFI_SUCCESS)
        printf("GRAPHICS ERROR\r\n");

    if (initGOP(FrameBuffer) != EFI_SUCCESS)
        printf("GOP ERROR\r\n");

    // if (loadKernel(L"pmos.bin", ImageHandle, &kernelBuffer) != EFI_SUCCESS)
    //     printf("KERNEL ERROR\r\n");
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    EFI_GUID protocol1 = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID protocol2 = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    gBS->HandleProtocol(ImageHandle, &protocol1, (void **)&LoadedImage);
    gBS->HandleProtocol(LoadedImage->DeviceHandle, &protocol2, (void **)&Volume);
    ELF_INFO info = {0};

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    if (LoadElf64(Volume, L"pmos.bin", &info) != EFI_SUCCESS)
        printf("KERNEL ERROR\r\n");
///////////////////////////
    printf("Preparing higher half\r\n");

    __asm__ volatile("push %rax");

    __asm__ volatile("mov %cr0, %rax");
    __asm__ volatile("or $0x1, %rax");
    __asm__ volatile("or $0x40000000, %rax");
    __asm__ volatile("mov %rax, %cr0");

    __asm__ volatile("mov %cr4, %rax");
    __asm__ volatile("or $0x10, %rax");
    __asm__ volatile("mov %rax, %cr4");

    __asm__ volatile("pop %rax");    

    // get the memory map
    UINT64* Pml4 = (UINT64*)myAsmReadCr3();

    // take the first pml4 and copy it to 0xffff800000000000
    Pml4[256] = Pml4[0];

    // allocate pml3 for 0xffffffff80000000
    UINT64* Pml3High = AllocateReservedPages(1);
    SetMem(Pml3High, EFI_PAGE_SIZE, 0);
    printf("Allocated page %x\r\n", Pml3High);
    Pml4[511] = ((UINT64)Pml3High) | 0x3u;

    // map first 2 pages to 0xffffffff80000000
    UINT64* Pml3Low = (UINT64*)(Pml4[0] & 0x7ffffffffffff000u);
    Pml3High[510] = Pml3Low[0];
    Pml3High[511] = Pml3Low[1];

    printf("Getting memory map\r\n");
////////////////////////////////////////////////
    entryPoint = (void *)info.Entry;
    printf("Entry: 0x%x\r\n", info.Entry);
    printf("PhysicalBase: 0x%x\r\n", info.PhysicalBase);
    printf("SectionEntrySize: 0x%x\r\n", info.SectionEntrySize);
    printf("SectionHeaders: 0x%x\r\n", info.SectionHeaders);
    printf("SectionHeadersSize: 0x%x\r\n", info.SectionHeadersSize);
    printf("StringSectionIndex: 0x%x\r\n", info.StringSectionIndex);
    printf("VirtualOffset: 0x%x\r\n", info.VirtualOffset);

    // if (ElfLoadImage(kernelBuffer, &entryPoint) != EFI_SUCCESS)
    //     printf("ENTRY POINT ERROR\r\n");

    if (font == NULL)
        printf("PSF1 ERROR\r\n");

    while (EFI_SUCCESS != (Status = gBS->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion)))
    {
        if (Status == EFI_BUFFER_TOO_SMALL)
        {
            printf("Setting up memory map buffer.\r\n");
            mapSize += 2 * descriptorSize;
            gBS->AllocatePool(EfiLoaderData, mapSize, (void **)&memoryMap);
        }
        else
            printf("Error getting memory map: %d.\r\n", Status);
    }
    if (EFI_ERROR(Status))
    {
        printf("Get Map Error!\r\n");
        return Status;
    }
    else
    {
        printf("Memory map size: %d.\r\n", mapSize);
        printf("Memory descriptor size: %d.\r\n", descriptorSize);
    }
    //unsigned char *addr = (unsigned char *)FrameBuffer->BaseAddress;
    //unsigned char bg_color = 0xff;

    // while (pixels--)
    // {
    //     // if (fb->format == PIXEL_BGR)
    //     // {
    //     //     *addr++ = (bg_color >> 16) & 255;
    //     //     *addr++ = (bg_color >> 8) & 255;
    //     //     *addr++ = bg_color & 255;
    //     // }
    //     // else // RGB
    //     // {
    //     // *addr++ = 0xffff0000;
    //     *addr++ = bg_color & 255;
    //     *addr++ = (bg_color >> 8) & 255;
    //     *addr++ = (bg_color >> 16) & 255;
    //     // }
    // }

    //SystemTable->ConOut->Reset(SystemTable->ConOut, 1);

    // while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
    //     ;

    void (*entryPointFun)(FRAMEBUFFER *) = entryPoint;
    printf("RUNNING\r\n");
    printf("ENTRY AT %x\r\n", entryPoint);
    entryPointFun(FrameBuffer);
    gBS->ExitBootServices(ImageHandle, mapKey);
    printf("STILL RUNNING\r\n");

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, 0);
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
