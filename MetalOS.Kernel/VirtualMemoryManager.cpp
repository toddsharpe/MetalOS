#include "VirtualMemoryManager.h"
#include "Main.h"

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager& physicalMemory) :
	m_physicalMemory(physicalMemory)
{

}

//TODO: round down address to nearest page or granularity boundary?
void* VirtualMemoryManager::Allocate(uintptr_t address, const size_t count, const MemoryProtection protection, VirtualAddressSpace& addressSpace)
{
	Print("Allocate: 0x%016x, Count: 0x%x Global: %d\n", address, count, addressSpace.IsGlobal());
	
	if (address != 0)
		Assert((address & PAGE_MASK) == 0);
	
	//Reserve region, returning false if it isn't free
	if (!addressSpace.Reserve(address, count, protection))
		return nullptr;

	PageTables pt(__readcr3());

	//Retrieve and map physical pages
	//TODO: this code maps each page since it might have to zero it first
	//Optimize this
	for (size_t i = 0; i < count; i++)
	{
		PageState state = PageState::Zeroed;
		paddr_t addr = 0;
		Assert(m_physicalMemory.AllocatePage(addr, state));

		//Map page
		const uintptr_t virtualAddress = address + (i << PAGE_SHIFT);
		if (addressSpace.IsGlobal())
			pt.MapKernelPages(virtualAddress, addr, 1);
		else
			pt.MapUserPages(virtualAddress, addr, 1);

		//Clear if needed
		if (state == PageState::Free)
			memset((void*)virtualAddress, 0, PAGE_SIZE);
	}

	return (void*)address;
}

void* VirtualMemoryManager::VirtualMap(uintptr_t virtualAddress, const std::vector<paddr_t>& addresses, const MemoryProtection& protection, VirtualAddressSpace& addressSpace)
{
	Print("VirtualMap: 0x%016x, Count: 0x%x\n", addresses.front(), addresses.size());
	Assert(addresses.size() != 0);

	if (virtualAddress != 0)
		Assert((virtualAddress & PAGE_MASK) == 0);

	//Reserve region, returning false if it isn't free
	if (!addressSpace.Reserve(virtualAddress, addresses.size(), protection))
		return nullptr;

	PageTables pt(__readcr3());

	for (size_t i = 0; i < addresses.size(); i++)
	{
		if (addressSpace.IsGlobal())
			pt.MapKernelPages(virtualAddress + (i << PAGE_SHIFT), addresses[i], 1);
		else
			pt.MapUserPages(virtualAddress + (i << PAGE_SHIFT), addresses[i], 1);
	}

	return (void*)virtualAddress;
}

paddr_t VirtualMemoryManager::ResolveAddress(void* address)
{
	PageTables pt(__readcr3());
	return pt.ResolveAddress((uintptr_t)address);
}
