#include "VirtualAddressSpace.h"
#include "MetalOS.System.h"
#include "Assert.h"

namespace
{
	constexpr size_t AllocationGranularity = (PageSize << 2);//16KB
}

bool VirtualAddressSpace::Debug = false;

VirtualAddressSpace::VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal) :
	IsGlobal(isGlobal),
	m_reservations(),
	m_start(start),
	m_end(end),
	m_watermark(start),
	m_initialized()
{
	//Set base for user allocations
	if (start == 0)
		m_watermark = 0x140000000;
}

void VirtualAddressSpace::Initialize()
{
	Assert(!m_initialized);
	ListInitializeHead(&m_reservations);
	m_initialized = true;
}

bool VirtualAddressSpace::IsFree(const uintptr_t address, const size_t count) const
{
	Assert(m_initialized);
	Assert((address & PageMask) == 0);

	//Check if address is inside out address space
	if ((address < m_start) || (address >= m_end))
		return false;

	const uintptr_t highAddress = address + (count << PageShift);

	ListEntry* entry = m_reservations.Flink;
	while (entry != &m_reservations)
	{
		const Reservation* res = LIST_CONTAINING_RECORD(entry, Reservation, Link);

		const uintptr_t end = res->Address + (res->PageCount << PageShift);

		//Check start address
		if ((address >= res->Address) && (address < end))
			return false;

		//Check ending address
		if ((highAddress >= res->Address) && (highAddress < end))
			return false;

		entry = entry->Flink;
	}

	return true;
}

bool VirtualAddressSpace::Reserve(uintptr_t& address, const size_t count)
{
	Assert(m_initialized);
	Assert(count != 0);
	CPrintf(Debug, "Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, m_start, m_end, count);

	if (address != 0)
	{
		//If address is specified, don't round
		if (!IsFree(address, count))
			return false;
	}
	else
	{
		CPrintf(Debug, "  Watermark: 0x%016x Gran: 0x%016x\n", m_watermark, AllocationGranularity);

		while (!IsFree(m_watermark, count))
		{
			AssertOp(m_watermark, <, m_end);
			m_watermark += AllocationGranularity;
		}
		
		address = m_watermark;
		m_watermark += ByteAlign((count << PageShift), AllocationGranularity);;
		CPrintf(Debug, "  NewWatermark: 0x%016x\n", m_watermark);
	}

	Reservation* res = new Reservation();
	Assert(res);
	res->Address = address;
	res->PageCount = count;
	ListInsertTail(&m_reservations, &res->Link);

	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

//Check to see if pointer is in a valid region
bool VirtualAddressSpace::IsValidPointer(const void* p) const
{
	const uintptr_t page = (uintptr_t)p & ~PageMask;
	return !IsFree(page, 1);
}
