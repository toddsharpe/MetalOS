#include "EfiMain.h"
#include "Common.h"
#include "Print.h"
#include "String.h"
#include "Device.h"

#define EFI_DEBUG 1

EFI_SYSTEM_TABLE *ST;
EFI_RUNTIME_SERVICES* RT;
EFI_BOOT_SERVICES* BS;

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;

	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	ReturnIfNotSuccess(Print(L"MetalOS.BootLoader\n\r"));
	ReturnIfNotSuccess(Print(L"Firmware Vendor: %S, Revision: %d\n\r", ST->FirmwareVendor, ST->FirmwareRevision));

	EFI_TIME time;
	ReturnIfNotSuccess(RT->GetTime(&time, NULL));
	Print(L"Date: %d-%d-%d %d:%d:%d\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	LOADER_PARAMS params = { 0 };

	ReturnIfNotSuccess(DiscoverGraphics(&params));

	Keywait("Waiting...\n");

	return status;
}

EFI_STATUS Keywait(CHAR16* String)
{
	EFI_STATUS status;
	EFI_INPUT_KEY Key;

	Print(String);
	ReturnIfNotSuccess(Print(L"Press any key to continue..."));
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	// Poll for key
	while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);

	// Clear keystroke buffer (this is just a pause)
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	Print(L"\r\n");
	return status;
}
