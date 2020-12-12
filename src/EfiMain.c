#include <Uefi.h>
#include "stdio.h"
#include "ourLoadFile.h"

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    printf("RUNNING\r\n");

    FRAMEBUFFER *FrameBuffer = NULL;
    void *kernelBuffer = NULL;
    void *entryPoint = NULL;
    PSF1_FONT *font = LoadPSF1Font(L"zap-light16.psf", ImageHandle);

    if (loadKernel(L"pmos.bin", &kernelBuffer, ImageHandle) != EFI_SUCCESS)
        printf("KERNEL ERROR\r\n");
    printf("pool addr: %x\r\n", kernelBuffer);

    if (ElfLoadImage(kernelBuffer, &entryPoint) != EFI_SUCCESS)
        printf("ENTRY POINT ERROR\r\n");

    if (EnterBestGraphicMode() != EFI_SUCCESS)
        printf("GRAPHICS ERROR\r\n");

    if (initGOP(FrameBuffer) != EFI_SUCCESS)
        printf("GOP ERROR\r\n");

    if (font == NULL)
        printf("PSF1 ERROR\r\n");

    void (*entryPointFun)(FRAMEBUFFER *) = entryPoint;

    entryPointFun(FrameBuffer);
    printf("STILL RUNNING\r\n");
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, 0, 0, 0);

    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY)
        ;
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
