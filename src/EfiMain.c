#include <Uefi.h>

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    Status = SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\r\n");
    if (EFI_ERROR(Status))
    {
        return Status;
    }
 
    Status = SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
    if (EFI_ERROR(Status))
    {
        return Status;
    }
 
    while ((Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY);
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
    return EFI_SUCCESS;
}
