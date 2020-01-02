#include "Error.h"
#include "String.h"
#include "EfiPrint.h"
#include "EfiMain.h"
#include "CRT.h"

EFI_STATUS Error::DisplayError(const CHAR16* function, EFI_STATUS status)
{
	CHAR16 statusString[64];
	Print(L"Error:\r\n");
	Print(L"  %S\r\n", function);
	StatusToString(statusString, status);
	Print(L"  Status: %d ( %S )\r\n", status, statusString);
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
			crt::strcpy(buffer, ErrorCodeTable[index].Desc);
			return;
		}
	}

	crt::strcpy(buffer, L"StatusNotFound");
}
