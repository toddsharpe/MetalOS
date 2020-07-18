#include "Error.h"

#include <efilib.h>
#include <crt_wchar.h>
#include <crt_stdlib.h>
#include <stdlib.h>
#include "EfiMain.h"

#define MAXBUFFER 128
void Bugcheck(const char* file, const char* line, const char* assert)
{
	CHAR16 wideFile[MAXBUFFER] = { 0 };
	mbstowcs(wideFile, file, MAXBUFFER);

	CHAR16 wideLine[MAXBUFFER] = { 0 };
	mbstowcs(wideLine, line, MAXBUFFER);

	CHAR16 wideAssert[MAXBUFFER] = { 0 };
	mbstowcs(wideAssert, assert, MAXBUFFER);

	Print(L"Error:\r\n");
	Print(L"  %s\r\n", wideAssert);
	Print(L"  %s\r\n", wideFile);
	Print(L"  %s\r\n", wideLine);
	BS->Stall(1000 * 1000 * 60 * 60);
}

EFI_STATUS Error::DisplayError(const CHAR16* function, const CHAR16* file, const CHAR16* line, EFI_STATUS status)
{
	CHAR16 statusString[64];
	Print(L"Error:\r\n");
	Print(L"  %s\r\n", function);
	StatusToString(statusString, status);
	Print(L"  Status: %d ( %s )\r\n", status, statusString);
	Print(L"  %s\r\n", file);
	Print(L"  %s\r\n", line);
	BS->Stall(1000 * 1000 * 60 * 60);
	return EFI_ABORTED;
}

void Error::StatusToString(OUT CHAR16* buffer, IN EFI_STATUS status)
{
	UINTN index;

	for (index = 0; ErrorCodeTable[index].Desc; index++)
	{
		if (ErrorCodeTable[index].Code == status)
		{
			wcscpy(buffer, ErrorCodeTable[index].Desc);
			return;
		}
	}

	wcscpy(buffer, L"StatusNotFound");
}
