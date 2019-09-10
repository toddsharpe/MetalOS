#include "EfiMain.h"
#include "Common.h"

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;
	EFI_INPUT_KEY Key;

	ReturnIfNotSuccess(PrintSystemTable(SystemTable));
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"MetalOS.BootLoader\n\r"));

	/* Now wait for a keystroke before continuing, otherwise your
	   message will flash off the screen before you see it.

	   First, we need to empty the console input buffer to flush
	   out any keystrokes entered before this point */
	status = SystemTable->ConIn->Reset(SystemTable->ConIn, FALSE);
	if (EFI_ERROR(status))
		return status;

	/* Now wait until a key becomes available.  This is a simple
	   polling implementation.  You could try and use the WaitForKey
	   event instead if you like */
	while ((status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY);

	return status;
}

EFI_STATUS PrintSystemTable(EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"PrintSystemTable\n\r"))
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, SystemTable->FirmwareVendor));
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n\r"));


}