#include <Uefi.h>
#include "stdio.h"
#include "scheduler.h"
#include "fileio.h"

#include "bf.h"

EFI_SYSTEM_TABLE *SystemTable;
EFI_BOOT_SERVICES *gBS;
static EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};

// SMP Test
// typedef struct
// {
//     int index;
//     char data;
// } testArg;

// char tests[5] = {'0', '0', '0', '0', '0'};

// void testPrint(testArg *arg)
// {
//     gBS->Stall(2000000);
//     tests[arg->index] = arg->data;
// }

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

    // Testing SMP
    // for (size_t i = 0; i < 5; i++)
    // {
    //     testArg *argTest = NULL;
    //     kmalloc(sizeof(testArg), (void **)&argTest);
    //     argTest->index = i;
    //     argTest->data = '1' + i;
    //     addProcToQueue(testPrint, argTest);
    // }

    // ++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.//Hello World!
    // >>+>+>><<<<<++++++++++++[->>>[-]<[->>>+<<<]>>>[-<<+<+>>>]<<<<[->>>>+<<<<]>>>>[-<+>]<<<[->>>+<<<]>>>[-<<<+<+>>>>]<<<[-]>>><[->+<]>[-<+<<+>>>]<<<<<]>>>>. Fibonacci 89 (Y)

    char *bf__program = loadfile(L"main.bf", ImageHandle);

    if (NULL == bf__program)
        printf("LoadFile ERROR\r\n");

    printf("program in main: %s\r\n", bf__program);
    addProcToQueue(bf__run, bf__program);

    while (1)
        kernelScanf();

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
