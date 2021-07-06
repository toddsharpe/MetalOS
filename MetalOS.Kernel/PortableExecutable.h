#pragma once

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include <cstdint>
#include "msvc.h"
#include <string>
#include "MetalOS.Kernel.h"

class PortableExecutable
{
public:
	static PIMAGE_SECTION_HEADER GetPESection(const std::string& name, const uintptr_t ImageBase);
	static PIMAGE_SECTION_HEADER GetPESection(const uint32_t index, const uintptr_t ImageBase);
	static uintptr_t GetProcAddress(Handle hModule, const char* lpProcName);
};

