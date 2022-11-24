#pragma once


#include <cstdint>
#include "VirtualAddressSpace.h"
#include "UserProcess.h"

class Loader
{
public:
	static Handle LoadLibrary(UserProcess& process, const char* path);
	static void* LoadKernelLibrary(const std::string& path);
	static void KernelExports(void* address, const Handle importLibrary, const std::string& libraryName);
};

