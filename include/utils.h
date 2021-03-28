#pragma once

extern EFI_BOOT_SERVICES *gBS;

#define kmalloc(x, y) gBS->AllocatePool(EfiReservedMemoryType, x, y) //IN* , OUT**
#define free(x) gBS->FreePool(x)