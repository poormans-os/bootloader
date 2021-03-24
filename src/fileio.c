#include "fileio.h"

char *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle)
{
    EFI_GUID imageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID pathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID fsProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID fileInfoProtocol = EFI_FILE_INFO_ID;
    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;

    EFI_DEVICE_PATH_PROTOCOL *devicePath = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *volume = NULL;
    EFI_FILE_PROTOCOL *rootFS = NULL;
    EFI_FILE_PROTOCOL *fileHandle = NULL;
    UINTN buffSize = FILE_INFO_BUFFER_SIZE;
    EFI_FILE_INFO *fileInfo = NULL;

    EFI_STATUS Status = EFI_SUCCESS;

    char *str = NULL;
    if (EFI_SUCCESS != gBS->HandleProtocol(ImageHandle, &imageProtocol, (void **)&loadedImage))
    {
        printf("1\r\n");
        return NULL;
    }

    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &pathProtocol, (void **)&devicePath))
    {
        printf("2\r\n");
        return NULL;
    }

    if (EFI_SUCCESS != gBS->HandleProtocol(loadedImage->DeviceHandle, &fsProtocol, (void **)&volume))
    {
        printf("3\r\n");
        return NULL;
    }

    if (EFI_SUCCESS != volume->OpenVolume(volume, &rootFS))
    {
        printf("4\r\n");
        return NULL;
    }

    if (EFI_SUCCESS != rootFS->Open(rootFS, &fileHandle, path, EFI_FILE_MODE_READ, 0))
    {
        printf("5\r\n");
        return NULL;
    }
    if (EFI_SUCCESS != fileHandle->GetInfo(fileHandle, &fileInfoProtocol, &buffSize, fileInfo))
    {
        printf("6 buffsize: %d\r\n", buffSize);
        return NULL;
    }

    Status = kmalloc(fileInfo->FileSize + 1, (void **)&str); //TODO Fix!!!
    if (Status != EFI_SUCCESS)
    {
        printf("7 Status %d\r\n", Status);
        return NULL;
    }

    Status = fileHandle->Read(fileHandle, (void *)&fileInfo->FileSize, (void *)str);
    if (Status != EFI_SUCCESS)
    {
        printf("8 Status %d\r\n", Status);
        return NULL;
    }
    str[fileInfo->FileSize] = '\0';

    for (size_t i = 0; i < fileInfo->FileSize; i++)
    {
        putchar(str[i]);
    }
    printf("\r\n");

    printf("str[0] %c\r\n", str[0]);
    printf("size %d\r\n", fileInfo->FileSize);
    return str;
}