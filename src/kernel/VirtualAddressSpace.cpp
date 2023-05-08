#include "VirtualAddressSpace.h"

#include "Assert.h"

VirtualAddressSpace::VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal) :
	Start(start),
	End(end),
	IsGlobal(isGlobal),
	m_watermark(start),
	m_entries()
{
	//Reserve some space
	m_entries.reserve(16);
	
	//Set base for user allocations
	if (Start == 0)
		m_watermark = 0x140000000;
}

bool VirtualAddressSpace::IsFree(const uintptr_t address, const size_t count) const
{
	Assert((address & PageMask) == 0);

	//Check if address is inside out address space
	if ((address < Start) || (address >= End))
		return false;

	const uintptr_t highAddress = address + (count << PageShift);
	for (const Entry& entry : m_entries)
	{
		const uintptr_t end = entry.Address + (entry.PageCount << PageShift);
		
		//Check start address
		if ((address >= entry.Address) && (address < end))
			return false;

		//Check ending address
		if ((highAddress >= entry.Address) && (highAddress < end))
			return false;
	}

	return true;
}

bool VirtualAddressSpace::Reserve(uintptr_t& address, const size_t count)
{
	Assert(count != 0);
	Printf("Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, Start, End, count);

	if (address != 0)
	{
		//If address is specified, don't round
		if (!IsFree(address, count))
			return false;
	}
	else
	{
		Printf("  Watermark: 0x%016x Gran: 0x%016x\n", m_watermark, AllocationGranularity);

		while (!IsFree(m_watermark, count))
		{
			AssertOp(m_watermark, <, End);
			m_watermark += AllocationGranularity;
		}
		
		address = m_watermark;
		m_watermark += ByteAlign((count << PageShift), AllocationGranularity);;
		Printf("  NewWatermark: 0x%016x\n", m_watermark);
	}

	m_entries.push_back({ address, count });
	Printf("  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

//Check to see if pointer is in a valid region
bool VirtualAddressSpace::IsValidPointer(const void* p) const
{
	const uintptr_t page = (uintptr_t)p & ~PageMask;
	return !IsFree(page, 1);
}
