#pragma once

#include "msvc.h"
#include <cstdint>
#include <WindowsTypes.h>
#include "VirtualAddressSpace.h"
#include "UserProcess.h"

class Loader
{
public:
	static bool ReadHeaders(const char* path, uintptr_t& imageBase, DWORD& imageSize);
	static Handle LoadLibrary(UserProcess& process, const char* path);
	static Handle LoadKernelLibrary(const char* path);
	static uintptr_t GetProcAddress(Handle hModule, const char* lpProcName);
	static void KernelExports(void* address, void* kernelAddress);
};

