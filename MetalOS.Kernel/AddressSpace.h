#pragma once

#include "Kernel.h"
#include "msvc.h"
#include <map>

class VirtualAddressSpace
{
public:
	VirtualAddressSpace();

private:
	std::map<uintptr_t, size_t> m_allocations;
};

