#pragma once

#include "msvc.h"
#include <map>

class AddressSpace
{
public:
	AddressSpace();

private:
	std::map<uintptr_t, size_t> m_allocations;
};

