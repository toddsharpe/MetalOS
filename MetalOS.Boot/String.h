#pragma once

#include <efi.h>
#include <efilib.h>

#include "Common.h"

UINT32 efi_strlen(CHAR16* str);
void efi_revstr(CHAR16* str);
