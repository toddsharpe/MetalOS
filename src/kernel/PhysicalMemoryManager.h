#pragma once

#include "Lib/Bitset.h"
#include "Lib/Arena.h"
#include "kernel/MemoryMap.h"
#include "MetalOS.Loader.h"

//Allocates physical pages from page frame database
class PhysicalMemoryManager
{
public:
	PhysicalMemoryManager(PageFrame* const frames, const size_t count);

	void Initialize(const MemoryMap& memoryMap, Arena& arena);

	bool Allocate(paddr_t& address);
	bool Allocate(paddr_t& address, const size_t pageCount);
	void Deallocate(const paddr_t address);

private:
	static constexpr size_t BuddySize = 4;
	constexpr size_t GetIndex(const PageFrame* entry) const;

	constexpr size_t GetBuddyIndex(const paddr_t address);

	PageFrame* const m_frames;
	const size_t m_count;
	ListHead m_freeList;

	//Bitvector for contiguous allocations, where False is Free.
	DynamicBitset m_buddyMap;
	size_t m_nextBuddy;
};
