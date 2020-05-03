#include "VirtualAddressSpace.h"
#include "Main.h"

VirtualAddressSpace::VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool global) :
	m_start(start),
	m_end(end),
	m_global(global),
	m_watermark(end),
	m_nodes()
{

}

bool VirtualAddressSpace::IsFree(const uintptr_t address, const size_t count)
{
	Assert((address & PAGE_MASK) == 0);
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
	if (address != 0)
	{
		//If address is specified, don't round
		if (!IsFree(address, count))
			return false;
	}
	else
	{
		//Round size to multiple of AllocationGranularity
		size_t desired = (count << PAGE_SHIFT);
		const uintptr_t remainder = desired % AllocationGranularity;
		if (remainder != 0)
			desired += (AllocationGranularity - remainder);

		//Choose address using watermark
		//Watermark is address that can be given out
		m_watermark -= desired;
		Assert(m_watermark % AllocationGranularity == 0);

		while (!IsFree(m_watermark, count))
		{
			Assert(m_watermark != 0);
			m_watermark -= AllocationGranularity;
		}

		address = m_watermark;
	}

	Node n = { count, protection };
	m_nodes.insert({ address, n });

	return true;
}
