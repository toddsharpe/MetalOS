#include <stdlib.h>
#include <stdio.h>

#include <efi.h>
#include <efilib.h>

#define EFI_DEBUG 1

EFI_STATUS
(EFIAPI StdOutPrint) (
	IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE* This,
	IN CHAR16* String
	)
{
	wprintf(String);

	return EFI_SUCCESS;
}

EFI_STATUS
(EFIAPI StdInReset) (
	IN struct _SIMPLE_INPUT_INTERFACE* This,
	IN BOOLEAN                          ExtendedVerification
	)
{
	fflush(stdin);
}

EFI_STATUS
(EFIAPI StdInReadKeyStroke) (
	IN struct _SIMPLE_INPUT_INTERFACE* This,
	OUT EFI_INPUT_KEY* Key
	)
{
	Key->UnicodeChar = getwchar();

	return EFI_SUCCESS;
}

extern EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

int main(int argc, char** argv)
{
	SIMPLE_TEXT_OUTPUT_INTERFACE stdOut = { 0 };
	stdOut.OutputString = StdOutPrint;

	SIMPLE_INPUT_INTERFACE stdIn = { 0 };
	stdIn.Reset = StdInReset;
	stdIn.ReadKeyStroke = StdInReadKeyStroke;
	
	EFI_SYSTEM_TABLE table = { 0 };
	table.FirmwareVendor = L"SharpeCoding";
	table.FirmwareRevision = 0x01020304;
	table.ConOut = &stdOut;
	table.ConIn = &stdIn;

	EfiMain(0, &table);
}
