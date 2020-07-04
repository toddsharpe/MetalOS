#pragma once

#include "PhysicalMemoryManager.h"
#include <MetalOS.Internal.h>
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

	void* Allocate(uintptr_t address, const size_t count, const MemoryProtection protection, VirtualAddressSpace& addressSpace);
	void* VirtualMap(uintptr_t address, const std::vector<paddr_t>& addresses, const MemoryProtection& protection, VirtualAddressSpace& addressSpace);

	paddr_t ResolveAddress(void* address);

private:
	PhysicalMemoryManager& m_physicalMemory;
};

