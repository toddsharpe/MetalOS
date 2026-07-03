#pragma once

#include <Efi.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define WIDE_HELPER(x) L##x
#define WSTR(x) WIDE_HELPER(x)

#define WFILE WSTR(__FILE__)
#define LLINE WSTR(STR(__LINE__))

#define ReturnIfNotSuccess(x) \
do \
{ \
	const EFI_STATUS _status = x; \
	if (EFI_ERROR(_status)) \
	{ \
		EfiError::DisplayError(L#x, WFILE, LLINE, _status); return _status; \
	} \
} while (0)
#define ReturnIfNotSuccessNoDisplay(x) status = x; if (EFI_ERROR(status)) { return status; }

namespace EfiError
{
	EFI_STATUS DisplayError(const CHAR16* function, const CHAR16* file, const CHAR16* line, const EFI_STATUS status);
}
