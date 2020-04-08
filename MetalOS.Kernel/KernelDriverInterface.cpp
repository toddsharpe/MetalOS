#include "Kernel.h"
#include "Main.h"

void* Kernel::DriverMapIoSpace(paddr_t PhysicalAddress, size_t NumberOfBytes)
{
	uintptr_t virtualAddress = KernerDriverIOAddress + PhysicalAddress;
	size_t count = SIZE_TO_PAGES(NumberOfBytes);
	Assert(m_pageTables->MapKernelPages(virtualAddress, PhysicalAddress, count));
	return (void*)virtualAddress;
}
