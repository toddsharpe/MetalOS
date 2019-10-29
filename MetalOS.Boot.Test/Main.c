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

EFI_STATUS
(EFIAPI RuntimeGetTime) (
	OUT EFI_TIME* Time,
	OUT EFI_TIME_CAPABILITIES* Capabilities OPTIONAL
	)
{
	/*
	SYSTEMTIME time;
	GetSystemTime(&time);

	Time->Year = time.wYear;
	Time->Month = time.wMonth;
	Time->Day = time.wDay;
	Time->Hour = time.wHour;
	Time->Minute = time.wMinute;
	Time->Second = time.wSecond;
	*/
	Time->Year = 2019;
	Time->Month = 9;
	Time->Day = 13;
	Time->Hour = 14;
	Time->Minute = 11;
	Time->Second = 59;
}

EFI_STATUS
(EFIAPI EfiAllocatePool) (
	IN EFI_MEMORY_TYPE              PoolType,
	IN UINTN                        Size,
	OUT VOID** Buffer
	)
{
	*Buffer = malloc(Size);
	return EFI_SUCCESS;
}

EFI_STATUS
(EFIAPI EfiAllocatePages) (
	IN EFI_ALLOCATE_TYPE            Type,
	IN EFI_MEMORY_TYPE              MemoryType,
	IN UINTN                        NoPages,
	OUT EFI_PHYSICAL_ADDRESS* Memory
	)
{
	//Just grab the memory off the heap
	*Memory = malloc(NoPages << EFI_PAGE_SIZE);
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

	EFI_RUNTIME_SERVICES runtimeServices = { 0 };
	table.RuntimeServices = &runtimeServices;
	runtimeServices.GetTime = RuntimeGetTime;

	EFI_BOOT_SERVICES bootServices = { 0 };
	table.BootServices = &bootServices;
	bootServices.AllocatePool = &EfiAllocatePool;
	bootServices.AllocatePages = &EfiAllocatePages;

	EfiMain(0, &table);
}
