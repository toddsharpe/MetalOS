#include "Error.h"

#include <efilib.h>
#include <crt_wchar.h>
#include <crt_stdlib.h>
#include <stdlib.h>
#include "EfiMain.h"

constexpr UINTN Stall = 1000 * 1000 * 60;//60 minute stall

extern "C" void Bugcheck(const char* file, const char* line, const char* assert)
{
	static constexpr size_t MaxBuffer = 128;
	
	CHAR16 wideFile[MaxBuffer] = { 0 };
	mbstowcs(wideFile, file, MaxBuffer);

	CHAR16 wideLine[MaxBuffer] = { 0 };
	mbstowcs(wideLine, line, MaxBuffer);

	CHAR16 wideAssert[MaxBuffer] = { 0 };
	mbstowcs(wideAssert, assert, MaxBuffer);

	Print(L"Error:\r\n");
	Print(L"  %s\r\n", wideAssert);
	Print(L"  %s\r\n", wideFile);
	Print(L"  %s\r\n", wideLine);
	BS->Stall(Stall);
}

EFI_STATUS Error::DisplayError(const CHAR16* function, const CHAR16* file, const CHAR16* line, EFI_STATUS status)
{
	Print(L"Error:\r\n");
	Print(L"  %s\r\n", function);
	Print(L"  Status: %d ( %s )\r\n", status, StatusToString(status));
	Print(L"  %s\r\n", file);
	Print(L"  %s\r\n", line);
	BS->Stall(Stall);
	return EFI_ABORTED;
}

const CHAR16* Error::StatusToString(const EFI_STATUS status)
{
	UINTN index;

	for (index = 0; ErrorCodeTable[index].Desc; index++)
	{
		if (ErrorCodeTable[index].Code == status)
		{
			return ErrorCodeTable[index].Desc;
		}
	}

	return nullptr;
}
