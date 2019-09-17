#pragma once

#include <efi.h>
#include <efilib.h>

void efi_memcpy(void* source, void* dest, UINT32 count);
void efi_memset(void* dest, UINT8 value, UINT32 count);
int efi_memcmp(void* source, void* dest, UINT32 count);