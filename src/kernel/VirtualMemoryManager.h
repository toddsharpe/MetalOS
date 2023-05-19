#pragma once

#include "PhysicalMemoryManager.h"
#include <intrin.h>
#include <cstdint>
#include "VirtualAddressSpace.h"

//Ask address space for block
//Request physical pages
//Map

class VirtualMemoryManager
{
public:
	VirtualMemoryManager(PhysicalMemoryManager& physicalMemory);

	void* Allocate(const void* address, const size_t count, VirtualAddressSpace& addressSpace);
	void* VirtualMap(const void* address, const std::vector<paddr_t>& addresses, VirtualAddressSpace& addressSpace);

private:
	PhysicalMemoryManager& m_physicalMemory;
};
