#pragma once

#include "MemoryMap.h"
#include "LoaderParams.h"
#include "Bitvector.h"
#include "MetalOS.List.h"
#include "MetalOS.System.h"

#include <array>
#include <vector>

//return original page state and then marks active
//Page state should be internal, then just return a bool if its zeroed
class PhysicalMemoryManager
{
public:
	PhysicalMemoryManager(void* const address, const size_t count);

	void Initialize(const MemoryMap& memoryMap);
	bool AllocatePage(paddr_t& address);
	void DeallocatePage(const paddr_t address);

	bool AllocateContiguous(paddr_t& address, const size_t pageCount);

	size_t GetSize() const;

private:
	size_t GetIndex(const PageFrame* entry) const;

	PageFrame* const m_frames;
	const size_t m_count;
	ListEntry m_freeList;

	//Bitvector for contiguous allocations, where False is Free.
	Bitvector m_buddyMap;
	size_t m_nextBuddy;
};
