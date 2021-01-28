#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"
#include <intrin.h>
#include <idt.h>

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;

static UINT32 EfiTypeToStivaleType[] = {
    [EfiReservedMemoryType] = RESERVED,
    [EfiRuntimeServicesCode] = RESERVED,
    [EfiRuntimeServicesData] = RESERVED,
    [EfiMemoryMappedIO] = RESERVED,
    [EfiMemoryMappedIOPortSpace] = RESERVED,
    [EfiPalCode] = RESERVED,
    [EfiUnusableMemory] = BAD_MEMORY,
    [EfiACPIReclaimMemory] = ACPI_RECLAIM,
    [EfiLoaderCode] = KERNEL_MODULES,
    [EfiLoaderData] = KERNEL_MODULES,
    [EfiBootServicesCode] = BOOTLODAER_RECLAIM,
    [EfiBootServicesData] = BOOTLODAER_RECLAIM,
    [EfiConventionalMemory] = USABLE,
    [EfiACPIMemoryNVS] = ACPI_NVS};

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    // UINTN mapSize = 0, mapKey, descriptorSize;
    // EFI_MEMORY_DESCRIPTOR *memoryMap = NULL;
    // UINT32 descriptorVersion = 1;

    FRAMEBUFFER *FrameBuffer = NULL;
    // void *kernelBuffer = NULL;
    // void *entryPoint = NULL;
    PSF1_FONT *font = LoadPSF1Font(L"zap-light16.psf", ImageHandle);

    if (EnterBestGraphicMode() != EFI_SUCCESS)
        printf("GRAPHICS ERROR\r\n");

    if (initGOP(FrameBuffer) != EFI_SUCCESS)
        printf("GOP ERROR\r\n");

    idt_init();
    while (1)
    {
        /* code */
    }

    // if (loadKernel(L"pmos.bin", ImageHandle, &kernelBuffer) != EFI_SUCCESS)
    //     printf("KERNEL ERROR\r\n");
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    EFI_GUID protocol1 = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID protocol2 = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    gBS->HandleProtocol(ImageHandle, &protocol1, (void **)&LoadedImage);
    gBS->HandleProtocol(LoadedImage->DeviceHandle, &protocol2, (void **)&Volume);
    ELF_INFO info = {0};

    // SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    if (LoadElf64(Volume, L"pmos.bin", &info) != EFI_SUCCESS)
        printf("KERNEL ERROR\r\n");

    printf("Preparing higher half\r\n");

    // __asm__ volatile("push %rax");

    // __asm__ volatile("mov %cr0, %rax");
    // __asm__ volatile("or $0x1, %rax");
    // __asm__ volatile("orq $0x80000000, %rax");
    // __asm__ volatile("mov %rax, %cr0");

    // __asm__ volatile("mov %cr4, %rax");
    // __asm__ volatile("or $0x10, %rax");
    // __asm__ volatile("mov %rax, %cr4");

    // __asm__ volatile("pop %rax");

    // __asm__ volatile("cli");
    unsigned long long tempCr0 = ourAsmReadCr0();
    tempCr0 = tempCr0 & ~(1ULL << 16);
    ourAsmWriteCr0(tempCr0);

    // get the memory map
    // UINT64 *Pml4 = (UINT64 *)__readcr3();
    UINT64 *Pml4 = (UINT64 *)ourAsmReadCr3();
    printf("0x%x\r\n", Pml4);

    // take the first pml4 and copy it to 0xffff800000000000
    Pml4[256] = Pml4[0];

    // allocate pml3 for 0xffffffff80000000
    UINT64 *Pml3High = ourAllocateReservedPages(1); //AllocateReservedPages(1);
    memset(Pml3High, 0, EFI_PAGE_SIZE);
    printf("Allocated page %x\r\n", Pml3High);
    Pml4[511] = ((UINT64)Pml3High) | 0x3u;

    // map first 2 pages to 0xffffffff80000000
    UINT64 *Pml3Low = (UINT64 *)(Pml4[0] & 0x7ffffffffffff000u);
    Pml3High[510] = Pml3Low[0];
    Pml3High[511] = Pml3Low[1];

    printf("Getting memory map\r\n");
    UINT8 TmpMemoryMap[1];
    UINTN MemoryMapSize = sizeof(TmpMemoryMap);
    UINTN MapKey = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;
    INFO_STRUCT *Struct = ourAllocateReservedPool(sizeof(INFO_STRUCT));
    memset(Struct, 0, sizeof(INFO_STRUCT));

    if (gBS->GetMemoryMap(&MemoryMapSize, (EFI_MEMORY_DESCRIPTOR *)TmpMemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion) == EFI_BUFFER_TOO_SMALL)
    {
        printf("size of the buffer needed to contain the map: %d\r\n", MemoryMapSize);
    }

    // allocate space for the efi mmap and take into
    // account that there will be changes
    MemoryMapSize += EFI_PAGE_SIZE;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = ourAllocatePool(MemoryMapSize);

    // allocate all the space we will need (hopefully)
    MMAP_ENTRY *StartFrom = ourAllocateReservedPool((MemoryMapSize / DescriptorSize) * sizeof(MMAP_ENTRY));
    // printf("1\r\n");

    printf("Entry: 0x%x\r\n", info.Entry);
    printf("PhysicalBase: 0x%x\r\n", info.PhysicalBase);
    printf("SectionEntrySize: 0x%x\r\n", info.SectionEntrySize);
    printf("SectionHeaders: 0x%x\r\n", info.SectionHeaders);
    printf("SectionHeadersSize: 0x%x\r\n", info.SectionHeadersSize);
    printf("StringSectionIndex: 0x%x\r\n", info.StringSectionIndex);
    printf("VirtualOffset: 0x%x\r\n", info.VirtualOffset);

    void (*entryPointFun)(FRAMEBUFFER *) = (void *)info.Entry + 0xffffffff80000000; //(void *)info.PhysicalBase + info.VirtualOffset;
    printf("jumping to: 0x%x\r\n", entryPointFun);

    if (font == NULL)
        printf("PSF1 ERROR\r\n");

    // call it
    if (EFI_SUCCESS != (Status = gBS->GetMemoryMap(&MemoryMapSize, MemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion)))
    {
        printf("line 124 ERROR: %d\r\n", Status);
    }

    // Exit the memory services
    if (EFI_SUCCESS != (Status = gBS->ExitBootServices(ImageHandle, MapKey)))
    {
        printf("MapKey is incorrect: 0x%x, %d\r\n", MapKey, Status);
    }
    UINTN EntryCount = (MemoryMapSize / DescriptorSize);

    // setup the normal memory map
    Struct->MemoryMapAddr = (UINT64)StartFrom;
    Struct->MemoryMapEntries = 0;
    int LastType = -1;
    UINTN LastEnd = 0xFFFFFFFFFFFF;

    for (int i = 0; i < EntryCount; i++)
    {
        EFI_MEMORY_DESCRIPTOR *Desc = (EFI_MEMORY_DESCRIPTOR *)((UINTN)MemoryMap + DescriptorSize * i);
        int Type = EfiTypeToStivaleType[Desc->Type];

        if (LastType == Type && LastEnd == Desc->PhysicalStart)
        {
            StartFrom[-1].Length += EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
            LastEnd = Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
        }
        else
        {
            StartFrom->Type = Type;
            StartFrom->Length = EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
            StartFrom->Base = Desc->PhysicalStart;
            StartFrom->Unused = 0;
            LastType = Type;
            LastEnd = Desc->PhysicalStart + EFI_PAGES_TO_SIZE(Desc->NumberOfPages);
            StartFrom++;
            Struct->MemoryMapEntries++;
        }
    }

    entryPointFun(FrameBuffer);
    // if (ElfLoadImage(kernelBuffer, &entryPoint) != EFI_SUCCESS)
    //     printf("ENTRY POINT ERROR\r\n");

    // while (EFI_SUCCESS != (Status = gBS->GetMemoryMap(&mapSize, memoryMap, &mapKey, &descriptorSize, &descriptorVersion)))
    // {
    //     if (Status == EFI_BUFFER_TOO_SMALL)
    //     {
    //         printf("Setting up memory map buffer.\r\n");
    //         mapSize += 2 * descriptorSize;
    //         gBS->AllocatePool(EfiLoaderData, mapSize, (void **)&memoryMap);
    //     }
    //     else
    //         printf("Error getting memory map: %d.\r\n", Status);
    // }
    // if (EFI_ERROR(Status))
    // {
    //     printf("Get Map Error!\r\n");
    //     return Status;
    // }
    // else
    // {
    //     printf("Memory map size: %d.\r\n", mapSize);
    //     printf("Memory descriptor size: %d.\r\n", descriptorSize);
    // }

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

    // // gBS->ExitBootServices(ImageHandle, mapKey);
    printf("STILL RUNNING\r\n");

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, 0);
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
