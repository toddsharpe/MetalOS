#pragma once

#include "Error.h"

#define ReturnIfNotSuccess(x) status = x; if (EFI_ERROR(status)) { Error::DisplayError(L#x, status); return status; }

extern EFI_SYSTEM_TABLE* ST;
extern EFI_RUNTIME_SERVICES* RT;
extern EFI_BOOT_SERVICES* BS;
extern EFI_MEMORY_TYPE AllocationType;

