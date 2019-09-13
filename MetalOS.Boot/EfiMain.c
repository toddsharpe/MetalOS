#include "EfiMain.h"
#include "Common.h"
#include "Print.h"
#include "String.h"

#define EFI_DEBUG 1

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;
	EFI_INPUT_KEY Key;

	/*
	UINT32 len = stringlen(L"12345");

	CHAR16 string[] = L"ABC,DEF,GHI";
	
	CHAR16 *ptr = stringtok(string, L',');
	while (ptr != NULL)
	{
		ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, ptr));
		ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n\r"));
		ptr = stringtok(NULL, L',');
	}*/

	/*
	CHAR16 buffer[14];
	ByteToString(0x12, buffer);
	WordToString(0x1234, buffer + 2);
	DWordToString(0xDEADBEEF, buffer + 6);
	*/
	ReturnIfNotSuccess(PrintSystemTable(SystemTable));
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"MetalOS.BootLoader\n\r"));

	CHAR16 buffer[3];

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
	ReturnIfNotSuccess(Print(SystemTable->ConOut, L"FirmwareRevision: %u\n", SystemTable->FirmwareRevision));
	ReturnIfNotSuccess(SystemTable->ConOut->OutputString(SystemTable->ConOut, L"\n\r"));


}