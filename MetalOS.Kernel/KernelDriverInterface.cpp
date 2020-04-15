#include "Kernel.h"
#include "Main.h"

void* Kernel::DriverMapIoSpace(paddr_t PhysicalAddress, size_t NumberOfBytes)
{
	size_t count = SIZE_TO_PAGES(NumberOfBytes);
	Assert(count == 1);//TODO: implement

	PhysicalPageState state;
	Assert(m_pfnDb->AllocatePage(PhysicalAddress, state));

	uintptr_t virtualAddress = KernelDriverIOAddress + PhysicalAddress;
	Assert(m_pageTables->MapKernelPages(virtualAddress, PhysicalAddress, count));

	if (state == PhysicalPageState::Free)
		memset((void*)virtualAddress, 0, PAGE_SIZE);

	return (void*)virtualAddress;
}
