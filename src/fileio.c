#include "fileio.h"

char *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle)
{

    EFI_GUID imageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID pathProtocol = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID fsProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    //EFI_GUID fileInfoProtocol = (EFI_GUID){0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};
    // EFI_GUID fsProtocol = EFI_FILE_INFO;

    EFI_LOADED_IMAGE_PROTOCOL *loadedImage = NULL;
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

    int size = 1024;
    char *str = NULL;
    if (EFI_SUCCESS != kmalloc(1024, (void **)str)) //TODO Fix!!!
        printf("6\r\n");
    fileHandle->Read(fileHandle, (void *)&size, (void *)str);
    printf("%x: %s", str, str);
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