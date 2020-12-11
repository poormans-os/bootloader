#pragma once
#include "SimpleFileSystem.h"
#include <Uefi.h>
#include "LoadedImage.h"

EFI_FILE_PROTOCOL* loadfile(CHAR16* path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);