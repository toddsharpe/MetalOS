#pragma once

#include "MetalOS.Kernel.h"
#include "msvc.h"
#include <array>
#include <vector>
#include "MemoryMap.h"
#include "LoaderParams.h"

//return original page state and then marks active
//Page state should be internal, then just return a bool if its zeroed
class PhysicalMemoryManager
{
public:
	PhysicalMemoryManager(MemoryMap& memoryMap);

	bool AllocatePage(paddr_t& address, PhysicalPageState& state);
	void DeallocatePage(paddr_t address);

	size_t GetSize() const
	{
		return sizeof(PFN_ENTRY) * m_length;
	}

private:
	static_assert(sizeof(PFN_ENTRY) == 32, "If this number changes, change boot allotment. TODO etc");

	static const PhysicalPageState GetPageState(const EFI_MEMORY_DESCRIPTOR* desc);
	paddr_t GetPFN(PPFN_ENTRY entry);

	//Lists for quick access at runtime
	PPFN_ENTRY m_zeroed;
	PPFN_ENTRY m_free;

	size_t m_length;
	MemoryMap& m_memoryMap;
	PFN_ENTRY* m_db;
};

