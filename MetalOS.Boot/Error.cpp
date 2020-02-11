#include "Error.h"

#include <crt_wchar.h>
#include "EfiMain.h"

EFI_STATUS Error::DisplayError(const CHAR16* function, EFI_STATUS status)
{
	CHAR16 statusString[64];
	Print(L"Error:\r\n");
	Print(L"  %s\r\n", function);
	StatusToString(statusString, status);
	Print(L"  Status: %d ( %s )\r\n", status, statusString);
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
