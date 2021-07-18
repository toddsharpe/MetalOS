#include "VirtualAddressSpace.h"
#include "Main.h"

VirtualAddressSpace::VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool global) :
	m_start(start),
	m_end(end),
	m_global(global),
	m_watermark(start),
	m_nodes()
{
	if (m_start == 0)
		m_watermark = 0x140000000;
}

bool VirtualAddressSpace::IsFree(const uintptr_t address, const size_t count)
{
	Assert((address & PAGE_MASK) == 0);

	//Check if address is inside out address space
	if ((address < m_start) || (address >= m_end))
		return false;

	const uintptr_t highAddress = address + (count << PAGE_SHIFT);
	for (const auto& kvp : m_nodes)
	{
		const uintptr_t end = kvp.first + ((uintptr_t)kvp.second.PageCount << PAGE_SHIFT);
		
		//Check start address
		if ((address >= kvp.first) && (address < end))
			return false;

		//Check ending address
		if ((highAddress >= kvp.first) && (highAddress < end))
			return false;
	}

	return true;
}

bool VirtualAddressSpace::Reserve(uintptr_t& address, const size_t count, MemoryProtection protection)
{
	Assert(count != 0);
	kernel.Printf("Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, m_start, m_end, count);

	if (address != 0)
	{
		//If address is specified, don't round
		if (!IsFree(address, count))
			return false;
	}
	else
	{
		kernel.Printf("  Watermark: 0x%016x Gran: 0x%016x\n", m_watermark, AllocationGranularity);

		while (!IsFree(m_watermark, count))
		{
			Assert(m_watermark < m_end);
			m_watermark += AllocationGranularity;
		}
		
		address = m_watermark;
		m_watermark += ByteAlign((count << PAGE_SHIFT), AllocationGranularity);;
		kernel.Printf("  NewWatermark: 0x%016x\n", m_watermark);
	}

	Node n = { count, protection };
	m_nodes.insert({ address, n });
	kernel.Printf("  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

//Check to see if pointer is in a valid region
bool VirtualAddressSpace::IsValidPointer(const void* p)
{
	const uintptr_t page = (uintptr_t)p & ~PAGE_MASK;
	return !IsFree(page, 1);
}
