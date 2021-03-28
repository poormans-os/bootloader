#pragma once

#include <Uefi.h>
#include <LoadedImage.h>
#include <SimpleFileSystem.h>
#include <fileinfo.h>

#include "utils.h"
#include "stdio.h"

#define FILE_INFO_BUFFER_SIZE 96 //Constant size of struct containig information of a file.

typedef enum //Return options of loadfile function.
{
    None = 0, //There was an error while attempting to load a file.
    BF,       //The file found contains a BrainF*ck code or no file was found.
    SMP,      //The file found calls for a presentation of the SMP POC of the OS.
    IO,       //The file found calls for a presentation of the Input-Output mechanism of the OS.
} FileType;

extern EFI_BOOT_SERVICES *gBS; //Os's Boot Services handler.

FileType loadfile(char **fileData, IN EFI_HANDLE ImageHandle);