#pragma once

#include <efi.h>
#include <efilib.h>

#include "Common.h"

UINT32 efi_strlen(CHAR16* str);
void efi_strrev(CHAR16* str);
UINT32 efi_strcpy(CHAR16* source, CHAR16* dest);