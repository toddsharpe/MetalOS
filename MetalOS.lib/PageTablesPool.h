#pragma once
#include <cstdint>

#include "Kernel.h"
#include "PageTables.h"

//First page will be our index
//Gives us 1024 * 8 bits to signify pages
#define PageTablesPoolMax (PAGE_SIZE * 8)

//Class assumes region is Zeroe'd out first
class PageTables;
class PageTablesPool
{
public:
	PageTablesPool(uint64_t physicalAddress, uint32_t pageCount);
	void SetVirtualAddress(uint64_t virtualAddress);

	void Activate(PageTables& tables);

	bool AllocatePage(uint64_t* addressOut);
	bool DeallocatePage(uint64_t address);

	uint32_t AllocatedPageCount();

private:
	uint64_t m_virtualAddress;
	uint64_t m_physicalAddress;
	uint32_t m_pageCount;
	bool* m_index;
};

