#include "VirtualMemoryManager.h"

#include "Assert.h"
#include "PageTables.h"

VirtualMemoryManager::VirtualMemoryManager(PhysicalMemoryManager& physicalMemory) :
	m_physicalMemory(physicalMemory)
{

}

void* VirtualMemoryManager::Allocate(const void* address, const size_t count, VirtualAddressSpace& addressSpace)
{
	Printf("Allocate: 0x%016x, Count: 0x%x Global: %d\n", address, count, addressSpace.IsGlobal);
	
	uintptr_t addr = (uintptr_t)address;
	if (addr != 0)
		Assert((addr & PageMask) == 0);
	
	//Reserve region, returning nullptr if it isn't free
	if (!addressSpace.Reserve(addr, count))
		return nullptr;
	void* const baseAddress = (void*)addr;

	//Allocate and map pages. Physical address list could be non-contiguous, so map one at a time
	PageTables pt;
	pt.OpenCurrent();
	for (size_t i = 0; i < count; i++)
	{
		paddr_t pAddr = 0;
		Assert(m_physicalMemory.AllocatePage(pAddr));

		Assert(pt.MapPages(addr + (i << PageShift), pAddr, 1, addressSpace.IsGlobal));
	}
	
	//Zero region
	memset(baseAddress, 0, count * PageSize);
	return baseAddress;
}

void* VirtualMemoryManager::VirtualMap(const void* address, const std::vector<paddr_t>& addresses, VirtualAddressSpace& addressSpace)
{
	Printf("VirtualMap: 0x%016x, Count: 0x%x\n", address, addresses.size());
	Assert(addresses.size() != 0);

	uintptr_t addr = (uintptr_t)address;
	if (addr != 0)
		Assert((addr & PageMask) == 0);

	//Reserve region, returning false if it isn't free
	if (!addressSpace.Reserve(addr, addresses.size()))
		return nullptr;
	void* const baseAddress = (void*)addr;

	PageTables pt;
	pt.OpenCurrent();
	for (size_t i = 0; i < addresses.size(); i++)
	{
		//Physical address list could be non-contiguous, so map one at a time
		Assert(pt.MapPages(addr + (i << PageShift), addresses[i], 1, addressSpace.IsGlobal));
	}

	//Zero region
	memset(baseAddress, 0, addresses.size() * PageSize);
	return (void*)addr;
}
