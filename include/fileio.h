#pragma once

#include <Uefi.h>
#include <LoadedImage.h>
#include <SimpleFileSystem.h>
#include <fileinfo.h>

#include "utils.h"
#include "stdio.h"

#define FILE_INFO_BUFFER_SIZE 96

extern EFI_BOOT_SERVICES *gBS;

char *loadfile(IN CHAR16 *path, IN EFI_HANDLE ImageHandle);