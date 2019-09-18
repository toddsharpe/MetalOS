#pragma once

#include <efi.h>
#include <efilib.h>

void GetDirectory(CHAR16* source, CHAR16* destination);
void PathCombine(CHAR16* source, CHAR16* destination, CHAR16* file);