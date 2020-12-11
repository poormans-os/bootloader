
#include <Uefi.h>
#include "LoadedImage.h"
#include "SimpleFileSystem.h"
#include "stdio.h"
#include "ourLoadFile.h"

EFI_FILE_PROTOCOL *loadfile(CHAR16 *path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_GUID protocol1 = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID protocol2 = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID protocol3 = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &protocol1, (void **)&LoadedImage);

    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &protocol2, (void **)&DevicePath);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &protocol3, (void **)&Volume);

    EFI_FILE_PROTOCOL *RootFS;
    Volume->OpenVolume(Volume, &RootFS);

    EFI_FILE_PROTOCOL *CurDir = RootFS;

    EFI_FILE_PROTOCOL *FileHandle;
    EFI_STATUS Status = CurDir->Open(CurDir, &FileHandle, path, EFI_FILE_MODE_READ, 0);

    if (Status != EFI_SUCCESS)
        return NULL;
    return FileHandle;
}