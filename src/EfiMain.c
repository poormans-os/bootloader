#include <Uefi.h>
#include "stdio.h"
#include "scheduler.h"
#include "fileio.h"
#include "bf.h"

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;
static EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

// SMP Test
char smpTests[] = {'0', '0', '0', '0', '0', '\0'};

typedef struct
{
    int index;
    char data;
} smpArg;

void smpPrint(smpArg *arg)
{
    gBS->Stall(2000000);
    smpTests[arg->index] = arg->data;
}

void ioTest(void)
{
    char buffer[512] = {0};

    printf("Simple Echo Service\r\n");
    do
    {
        for (size_t i = 0; i < 512; i++)
            buffer[i] = 0;

        fgets(buffer, 512);
        printf("\r\nEcho: %s\r\n", buffer);
    } while (1);
}

EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    scanfPID = 0;

    SystemTable = ST;
    gBS = SystemTable->BootServices;

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
    gBS->SetWatchdogTimer(0, 0, 0, NULL);
    printf("RUNNING\r\n");

    if (initScheduler() != EFI_SUCCESS)
        printf("Error\r\n");

    // ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.//Hello World!
    // >>+>+>><<<<<++++++++++++[->>>[-]<[->>>+<<<]>>>[-<<+<+>>>]<<<<[->>>>+<<<<]>>>>[-<+>]<<<[->>>+<<<]>>>[-<<<+<+>>>>]<<<[-]>>><[->+<]>[-<+<<+>>>]<<<<<]>>>>. Fibonacci 89 (Y)

    char *fileData = NULL;
    FileType data = loadfile(&fileData, ImageHandle);

    if (None == data)
        printf("LoadFile ERROR\r\n");

    switch (data)
    {
    case BF:
        addProcToQueue(bf__run, fileData);
        break;
    case SMP: // SMP POC
        for (size_t i = 0; i < 5; i++)
        {
            smpArg *argTest = NULL;
            kmalloc(sizeof(smpArg), (void **)&argTest);
            argTest->index = i;
            argTest->data = '1' + i;
            addProcToQueue(smpPrint, argTest);
        }
        while (1)
        {
            printf("smpTests: %s\r\n", smpTests);
            gBS->Stall(1000000);
        }

        break;
    case IO:
        addProcToQueue(ioTest, NULL);
        break;
    default:
        //failed
        break;
    }

    while (1)
        kernelScanf();

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
