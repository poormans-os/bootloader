#include <Uefi.h>
#include "stdio.h"
#include "scheduler.h"
#include "fileio.h"

#include "bf.h"

EFI_SYSTEM_TABLE *SystemTable;                                                                                              //OS's System Table handler.
EFI_BOOT_SERVICES *gBS;                                                                                                     //OS's Boot Services handler.
static EFI_GUID gEfiMpServiceProtocolGuid = {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}}; //ID of multi-processor protocol.

// SMP Test
char smpTests[] = {'0', '0', '0', '0', '0', '\0'};

typedef struct
{
    int index;
    char data;
} smpArg;

/**
 * @brief This function changes values in a char array according to the given arguments struct.
 * 
 * @param arg A struct containing index in the array and data to put in that index.
 */
void smpPrint(smpArg *arg)
{
    gBS->Stall(2000000);
    smpTests[arg->index] = arg->data;
}

// Input - Output Test
/**
 * @brief This function contains an infinite that reads a string from the user using fgets function, and prints
 *      it using printf.
 */
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

/**
 * @brief This function is the kernal's entry point. It Initializes all of the resources and services that are
 *      necessary for the OS to work.
 * 
 * @param ImageHandle The firmware allocated handle for the UEFI image.
 * @param ST A pointer to the EFI System Table.
 * 
 * @return EFI_SUCCESS The operation completed successfully.
 */
EFI_STATUS
EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *ST)
{
    char *fileData = NULL; //Variable for data read from a file.

    scanfPID = 0;                    //Initialize variable to make kernelScanf wait before looking for input.
    SystemTable = ST;                //Initialize global System Table handler.
    gBS = SystemTable->BootServices; //Initialize global Boot Services handler.

    SystemTable->ConOut->Reset(SystemTable->ConOut, 1); //Reset the text output device hardware and optionaly run diagnostics.
    gBS->SetWatchdogTimer(0, 0, 0, NULL);               //Sets the system's watchdog timer.
    printf("RUNNING\r\n");

    if (initScheduler() != EFI_SUCCESS) //Initialize the OS's scheduler and check for its success.
        printf("Error\r\n");

    FileType data = loadfile(&fileData, ImageHandle); //Read content of a file into fileData.

    switch (data)
    {
    case BF:                               //File contains brainF*ck code.
        addProcToQueue(bf__run, fileData); //Add function that runs BF code to the scheduler's queue.
        break;

    case SMP:                          // SMP POC - File calls for a presentation of the SMP POC of the OS.
        for (size_t i = 0; i < 5; i++) //Add five tasks to the schedulers queue that will change the values of a chars array.
        {
            smpArg *argTest = NULL;
            kmalloc(sizeof(smpArg), (void **)&argTest);
            argTest->index = i;
            argTest->data = '1' + i;
            addProcToQueue(smpPrint, argTest);
        }
        while (1) //Infinite loop that prints the current value of the chars array.
        {
            printf("smpTests: %s\r\n", smpTests);
            gBS->Stall(1000000);
        }
        break;

    case IO:                          //File calls for a presentation of the Input and Output of the OS.
        addProcToQueue(ioTest, NULL); //Add Input-Output function to the scheduler queue.
        break;

    case None: //There was an error in the progress of locating and reading from a file.
        printf("LoadFile ERROR\r\n");
        break;

    default:
        //failed.
        break;
    }

    while (1)          //prevent from OS to shut down.
        kernelScanf(); //Call function that looks for input when getchar is called.

    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL); //Reset the entire OS before end of run.
    return EFI_SUCCESS;
}
