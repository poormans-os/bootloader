#include <Uefi.h>
#include "stdio.h"
#include "scheduler.h"

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;
static EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

typedef struct
{
    int index;
    char data;
} testArg;

char tests[5] = {'0', '0', '0', '0', '0'};

void testPrint(testArg *arg)
{
    gBS->Stall(2000000);
    tests[arg->index] = arg->data;
}

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    SystemTable = ST;
    gBS = SystemTable->BootServices;

    schedulerMtx = 0;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

    // EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    if (initScheduler() != EFI_SUCCESS)
        printf("Error\r\n");

    for (size_t i = 0; i < 5; i++)
    {
        testArg *argTest = NULL;
        kmalloc(sizeof(testArg), (void **)&argTest);
        argTest->index = i;
        argTest->data = '1' + i;
        addProcToQueue(testPrint, argTest);
    }

    while (1)
    {
        SystemTable->BootServices->WaitForEvent(1, &SystemTable->ConIn->WaitForKey, &KeyEvent);
        SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
        SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
        printf("The value is %c", Key.UnicodeChar);
    }

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
