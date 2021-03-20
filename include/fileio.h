#pragma once

#include <Uefi.h>
#include <LoadedImage.h>
#include <SimpleFileSystem.h>
#include <fileinfo.h>

#include "utils.h"
#include "stdio.h"

extern EFI_BOOT_SERVICES *gBS;

char *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle);