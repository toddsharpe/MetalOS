#include "VirtualMemoryManager.h"
#include "Main.h"

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager& physicalMemory, PageTablesPool& pool) :
	m_physicalMemory(physicalMemory),
	m_pool(pool)
{

}

//TODO: round down address to nearest page or granularity boundary?
void* VirtualMemoryManager::Allocate(uintptr_t address, const size_t count, const MemoryProtection protection, VirtualAddressSpace& addressSpace)
{
	Print("Allocate: 0x%016x, Count: 0x%x\n");
	
	if (address != 0)
		Assert((address & PAGE_MASK) == 0);
	
	//Reserve region, returning false if it isn't free
	if (!addressSpace.Reserve(address, count, protection))
		return nullptr;

	PageTables pt(__readcr3());
	pt.SetPool(&m_pool);

	//Retrieve and map physical pages
	//TODO: this code maps each page since it might have to zero it first
	//Optimize this
	for (size_t i = 0; i < count; i++)
	{
		PhysicalPageState state = PhysicalPageState::Zeroed;
		paddr_t addr = 0;
		Assert(m_physicalMemory.AllocatePage(addr, state));

		//Map page
		const uintptr_t virtualAddress = address + (i << PAGE_SHIFT);
		if (addressSpace.IsGlobal())
			pt.MapKernelPages(virtualAddress, addr, 1);
		else
			pt.MapUserPages(virtualAddress, addr, 1);

		//Clear if needed
		if (state == PhysicalPageState::Free)
			memset((void*)virtualAddress, 0, PAGE_SIZE);
	}

	return (void*)address;
}

paddr_t VirtualMemoryManager::ResolveAddress(void* address)
{
	PageTables pt(__readcr3());
	return pt.ResolveAddress((uintptr_t)address);
}
