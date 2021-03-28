#include "fileio.h"

/**
 * @brief This function reads a file and its content if it has any.
 * 
 * @param fileData A pointer to a string that will contain the content of the file.
 * @param ImageHandle The firmware allocated handle for the UEFI image.
 * @return FileType - A mark of which type of file was read:
 *          *None - There was an error while attempting to load a file.
 *          *BF - The file found contains a BrainF*ck code or no file was found.
 *          *SMP - The file found calls for a presentation of the SMP POC of the OS.
 *          *IO - The file found calls for a presentation of the Input-Output mechanism of the OS.
 */
FileType loadfile(char **fileData, IN EFI_HANDLE ImageHandle)
{
    //Initializing protocol variables necessary for loading a file and reading from it.
    EFI_GUID imageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID pathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID fsProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID fileInfoProtocol = EFI_FILE_INFO_ID;

    //Initializing handlers and variables necessary for loading a file and reading from it.
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
    EFI_DEVICE_PATH_PROTOCOL *devicePath = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *volume = NULL;
    EFI_FILE_PROTOCOL *rootFS = NULL;
    EFI_FILE_PROTOCOL *fileHandle = NULL;
    EFI_FILE_INFO *fileInfo = NULL;
    UINTN buffSize = FILE_INFO_BUFFER_SIZE;

    char *str = NULL;     //Variable the will hold the file's content.
    FileType type = None; //Variable of the return value.

    //Loading UEFI protocol handlers.
    if (EFI_SUCCESS != gBS->HandleProtocol(ImageHandle, &imageProtocol, (void **)&loadedImage))
    {
        printf("LoadFile Error 1\r\n");
        return None;
    }
    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &pathProtocol, (void **)&devicePath))
    {
        printf("LoadFile Error 2\r\n");
        return None;
    }
    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &fsProtocol, (void **)&volume))
    {
        printf("LoadFile Error 3\r\n");
        return None;
    }
    if (EFI_SUCCESS != volume->OpenVolume(volume, &rootFS))
    {
        printf("LoadFile Error 4\r\n");
        return None;
    }

    //Attempting to open 3 types of files.
    if (EFI_SUCCESS == rootFS->Open(rootFS, &fileHandle, L"main.bf", EFI_FILE_MODE_READ, 0))
    {
        printf("Running BF\r\n");
        type = BF; //This type of file might have content in it so don't return from the function yet.
    }
    else if (EFI_SUCCESS == rootFS->Open(rootFS, &fileHandle, L"SMP.test", EFI_FILE_MODE_READ, 0))
    {
        printf("Running smp test\r\n");
        return SMP; //This type of file doesn't have relevant content.
    }
    else if (EFI_SUCCESS == rootFS->Open(rootFS, &fileHandle, L"IO.test", EFI_FILE_MODE_READ, 0))
    {
        printf("Running io test\r\n");
        return IO; //This type of file doesn't have relevant content.
    }
    else
    {
        return BF; //If there is no file, get BrainF*ck code through user input.
    }

    //Getting information about a file into fileInfo buffer.
    if (EFI_SUCCESS != fileHandle->GetInfo(fileHandle, &fileInfoProtocol, &buffSize, fileInfo))
    {
        printf("LoadFile Error 5\r\n");
        return None;
    }

    //Allocating memory to the file content buffer according to the size of the file's content.
    if (EFI_SUCCESS != kmalloc(fileInfo->FileSize + 1, (void **)&str))
    {
        printf("LoadFile Error 6\r\n");
        return None;
    }

    //Reading from the file's content to the buffer.
    if (EFI_SUCCESS != fileHandle->Read(fileHandle, (void *)&fileInfo->FileSize, (void *)str))
    {
        printf("LoadFile Error 7\r\n");
        return None;
    }
    str[fileInfo->FileSize] = '\0'; //End of buffer.
    *fileData = str;                //Setting the file data string to the file's content buffer.

    return type; //returning the type of the file.
}