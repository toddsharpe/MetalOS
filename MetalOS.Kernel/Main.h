#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include "LoaderParams.h"

extern "C" void main_thunk(LOADER_PARAMS* loader);
void main(LOADER_PARAMS* loader);
_declspec(noreturn) void KernelBugcheck(const char* file, const char* line, const char* assert);

