#include "Kernel/Kernel.h"
#include "Assert.h"

void* Kernel::DriverMapPages(paddr_t address, size_t count)
{
	Assert(count > 0);

	const uintptr_t virtualAddress = KernelIoStart + address;
	
	PageTables tables;
	tables.OpenCurrent();
	Assert(tables.MapPages(virtualAddress, address, count, true));
	return (void*)virtualAddress;
}
