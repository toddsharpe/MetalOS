#pragma once

#include <efi.h>
#include "LoaderParams.h"

extern "C" void main(LOADER_PARAMS* loader);
_declspec(noreturn) void KernelBugcheck(const char* assert);
