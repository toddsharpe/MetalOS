#include "Kernel/Kernel.h"
#include "Assert.h"

void* Kernel::DriverMapPages(paddr_t address, size_t count)
{
	Assert(count > 0);

	uintptr_t virtualAddress = KernelIoStart + address;
	Assert(m_pageTables->MapKernelPages(virtualAddress, address, count));
	return (void*)virtualAddress;
}
