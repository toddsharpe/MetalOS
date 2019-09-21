#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS DisplayError(CHAR16* function, EFI_STATUS status);

VOID
StatusToString(
	OUT CHAR16* Buffer,
	IN EFI_STATUS       Status
);