#include "fileio.h"

char *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle)
{
    EFI_GUID imageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID pathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID fsProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID fileInfoProtocol = EFI_FILE_INFO_ID;
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
    EFI_STATUS Status = EFI_SUCCESS;
    if (EFI_SUCCESS != gBS->HandleProtocol(ImageHandle, &imageProtocol, (void **)&loadedImage))
    {
        printf("1\r\n");
        return NULL;
    }

    EFI_DEVICE_PATH_PROTOCOL *devicePath = NULL;
    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &pathProtocol, (void **)&devicePath))
        printf("2\r\n");

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *volume = NULL;
    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &fsProtocol, (void **)&volume))
        printf("3\r\n");

    EFI_FILE_PROTOCOL *rootFS = NULL;
    if (EFI_SUCCESS != volume->OpenVolume(volume, &rootFS))
        printf("4\r\n");

    EFI_FILE_PROTOCOL *fileHandle = NULL;
    if (EFI_SUCCESS != rootFS->Open(rootFS, &fileHandle, path, EFI_FILE_MODE_READ, 0))
        printf("5\r\n");

    UINTN buffSize = FILE_INFO_BUFFER_SIZE;
    EFI_FILE_INFO *fileInfo = NULL;
    if (EFI_SUCCESS != fileHandle->GetInfo(fileHandle, &fileInfoProtocol, &buffSize, fileInfo))
        printf("6 buffsize: %d\r\n", buffSize);

    char *str = NULL;
    Status = kmalloc(fileInfo->FileSize + 1, (void **)&str); //TODO Fix!!!
    if (Status != EFI_SUCCESS)
        printf("7 Status %d\r\n", Status);
    str[fileInfo->FileSize] = '\0';

    Status = fileHandle->Read(fileHandle, (void *)&fileInfo->FileSize, (void *)str);
    if (Status != EFI_SUCCESS)
        printf("8 status %d\r\n", Status);

    printf("str: %s\r\n", str);
    return str;

    // int fileInfoBufferSize = 0;
    // EFI_FILE_INFO *fileInfoBuffer = NULL;
    // Status = fileHandle->GetInfo(fileHandle, &fileInfoProtocol, (void *)&fileInfoBufferSize, fileInfoBuffer);
    // if (Status != EFI_SUCCESS)
    // {
    //     printf("fileInfoBufferSize %d\r\n", fileInfoBufferSize);
    //     printf("Open File Failed %d\r\n", Status);

    //     Status = fileHandle->GetInfo(fileHandle, &fileInfoProtocol, (void *)&fileInfoBufferSize, fileInfoBuffer);
    //     if (Status != EFI_SUCCESS)
    //     {
    //         printf("ERROROROROROOR\r\n");
    //         return NULL;
    //     }
    // }

    // int size = fileInfoBuffer->FileSize;

    // printf("Running %d %d\r\n", fileInfoBufferSize, size);

    // void *str = NULL;
    // if (EFI_SUCCESS != kmalloc(size, (void **)&str))
    // {
    //     printf("kmalloc error\r\n");
    //     return NULL;
    // }

    // printf("Before Read %x\r\n", str);
    // fileHandle->Read(fileHandle, (void *)&size, (void *)str);
    // printf("After %x\r\n", str);

    // if (Status != EFI_SUCCESS)
    //     return NULL;
    // return fileHandle;
}