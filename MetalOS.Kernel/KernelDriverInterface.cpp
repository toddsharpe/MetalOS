#include "Kernel.h"
#include "Main.h"

void* Kernel::DriverMapIoSpace(paddr_t PhysicalAddress, size_t NumberOfBytes)
{
	size_t count = SIZE_TO_PAGES(NumberOfBytes);
	Assert(count == 1);//TODO: implement

	PageState state;
	Assert(m_pfnDb->AllocatePage(PhysicalAddress, state));

	uintptr_t virtualAddress = KernelDriverIOAddress + PhysicalAddress;
	Assert(m_pageTables->MapKernelPages(virtualAddress, PhysicalAddress, count));

	if (state == PageState::Free)
		memset((void*)virtualAddress, 0, PAGE_SIZE);

	return (void*)virtualAddress;
}

void* Kernel::DriverMapPages(paddr_t address, size_t count)
{
	Assert(count > 0);

	uintptr_t virtualAddress = KernelDriverIOAddress + address;
	Assert(m_pageTables->MapKernelPages(virtualAddress, address, count));
	return (void*)virtualAddress;
}
