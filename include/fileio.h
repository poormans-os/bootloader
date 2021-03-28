#pragma once

#include <Uefi.h>
#include <LoadedImage.h>
#include <SimpleFileSystem.h>
#include <fileinfo.h>

#include "utils.h"
#include "stdio.h"

#define FILE_INFO_BUFFER_SIZE 96

typedef enum
{
    None = 0,
    BF,
    SMP,
    IO,
} FileType;

extern EFI_BOOT_SERVICES *gBS;

FileType loadfile(char **fileData, IN EFI_HANDLE ImageHandle);