#pragma once

#include <stdint.h>
#include <kernel/MetalOS.Internal.h>

//First page will be our index
//Gives us 1024 * 8 bits to signify pages
#define PageTablesPoolMax (PAGE_SIZE * 8)

class PageTablesPool
{
public:
	PageTablesPool(void* const baseAddress, const paddr_t physicalAddress, const size_t pageCount);

	bool AllocatePage(uint64_t* addressOut);
	bool DeallocatePage(uint64_t address);

	uint32_t AllocatedPageCount();

	uint64_t GetVirtualAddress(uint64_t physicalAddress);

private:
	void* m_baseAddress;
	paddr_t m_physicalAddress;
	size_t m_pageCount;
	bool* m_index;
};