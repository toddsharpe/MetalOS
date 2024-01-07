#pragma once

#include "MetalOS.System.h"
#include <stdint.h>

//Contiguous pool of physical pages which can be addressed from virtual base address
//First page is array of bools (NOT bitmap).
//TODO(tsharpe): Change to bitmap or map all physical pages in virtual space and eliminate this class
class PageTablesPool
{
public:
	PageTablesPool(void* const virtualBase, const paddr_t physicalBase, const size_t count);

	void Initialize();
	bool AllocatePage(paddr_t& address);
	bool DeallocatePage(const paddr_t address);

	void* GetVirtualAddress(const paddr_t address) const;

private:
	uintptr_t m_virtualBase;
	paddr_t m_physicalBase;
	size_t m_count;
	bool* m_index;
};