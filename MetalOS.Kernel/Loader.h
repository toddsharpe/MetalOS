#pragma once
#include <cstdint>
#include <WindowsTypes.h>
#include "VirtualAddressSpace.h"

class Loader
{
public:
	static bool ReadHeaders(const char* path, uintptr_t& imageBase, DWORD& imageSize);
	static bool LoadImage(const char* path, void* address);
};

