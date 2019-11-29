#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include "LoaderParams.h"

extern "C" void main(LOADER_PARAMS* loader);
void InitializeGlobals();
_declspec(noreturn) void KernelBugcheck(const char* assert);
