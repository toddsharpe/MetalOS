#pragma once

#include <stdint.h>
#include <MetalOS.Internal.h>

//First page will be our index
//Gives us 1024 * 8 bits to signify pages
#define PageTablesPoolMax (PAGE_SIZE * 8)

class PageTables;
class PageTablesPool
{
public:
	PageTablesPool(uint64_t physicalAddress, uint32_t pageCount);
	PageTablesPool(uint64_t virtualAddress, uint64_t physicalAddress, uint32_t pageCount);

	bool AllocatePage(uint64_t* addressOut);
	bool DeallocatePage(uint64_t address);

	uint32_t AllocatedPageCount();

	uint64_t GetVirtualAddress(uint64_t physicalAddress);

private:
	uint64_t m_virtualAddress;
	uint64_t m_physicalAddress;
	uint32_t m_pageCount;
	bool* m_index;
};

