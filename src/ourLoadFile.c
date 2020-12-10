
#include <Uefi.h>
#include "LoadedImage.h"
#include "SimpleFileSystem.h"

EFI_FILE_PROTOCOL* loadfile(CHAR16* path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_GUID protocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &protocol, (void**)&LoadedImage);

    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &protocol, (void**)&DevicePath);

    // Beyond BIO Page 58
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
    SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &protocol, (void**)&Volume);
    
    // Beyond BIO Page 58
    EFI_FILE_PROTOCOL* RootFS;
    Volume->OpenVolume(Volume, &RootFS);
    
    EFI_FILE_PROTOCOL* CurDir = RootFS;

    EFI_FILE_PROTOCOL* FileHandle;
    EFI_STATUS Status = CurDir->Open(CurDir, &FileHandle, path, EFI_FILE_MODE_READ, 0);
	if (Status != EFI_SUCCESS)
    {
		return NULL;
	}
	return FileHandle;
}