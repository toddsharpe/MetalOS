#pragma once

#define __midl
#include "Error.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define WIDE_HELPER(x) L##x
#define WSTR(x) WIDE_HELPER(x)

#define WFILE WSTR(__FILE__)
#define LLINE WSTR(STR(__LINE__))

#define ReturnIfNotSuccess(x) status = x; if (EFI_ERROR(status)) { Error::DisplayError(L#x, WFILE, LLINE, status); return status; }
#define ReturnIfNotSuccessNoDisplay(x) status = x; if (EFI_ERROR(status)) { return status; }

//extern EFI_SYSTEM_TABLE* ST;
//extern EFI_RUNTIME_SERVICES* RT;
//extern EFI_BOOT_SERVICES* BS;
extern EFI_MEMORY_TYPE AllocationType;

