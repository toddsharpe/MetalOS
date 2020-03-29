#pragma once

#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include <cstdint>
#include "msvc.h"
#include <string>

//https://stackoverflow.com/questions/5705650/stackwalk64-on-windows-get-symbol-name
class RuntimeSupport
{
public:
	static PIMAGE_SECTION_HEADER GetPESection(const std::string& name, uint64_t* ImageBase);
	static PRUNTIME_FUNCTION LookupFunctionEntry(uint64_t ControlPC, uint64_t* ImageBase, void* HistoryTable);
	static bool StackWalk64();
};

