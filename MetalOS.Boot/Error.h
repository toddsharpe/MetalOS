#pragma once

#include <efi.h>
#include <efilib.h>

VOID
StatusToString(
	OUT CHAR16* Buffer,
	IN EFI_STATUS       Status
);