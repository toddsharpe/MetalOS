#pragma once

#include "kernel/AddressSpace.h"

bool AddressSpace::IsValidPointer(const void* const address)
{
	const uintptr_t page = (uintptr_t)address & ~Arch::PageMask;
	if (page < Start || page >= End)
		return false;
	return !IsFree(page, 1);
}

uintptr_t AddressSpace::Reserve(const size_t count)
{
	Assert(count);
	CPrintf(Debug, "Reserve: Any in [0x%016x, 0x%016x] Size: 0x%x\n", Start, End, count);
	CPrintf(Debug, "  Watermark: 0x%016x Gran: 0x%016x\n", m_watermark, AllocationGranularity);

	//Find first free
	const size_t bytes = (count << Arch::PageShift);
	uintptr_t test = m_watermark;
	const uintptr_t stop = End - bytes;
	while (test <= stop && !IsFree(test, count))
	{
		test += AllocationGranularity;
	}

	//None found
	if (test == stop)
		return 0;

	//Claim address
	m_watermark = test;
	const uintptr_t address = m_watermark;
	m_reservations.Add(Reservation{address, count});

	//Increment watermark
	m_watermark += ByteAlign(bytes, AllocationGranularity);
	CPrintf(Debug, "  NewWatermark: 0x%016x\n", m_watermark);
	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return address;
}

bool AddressSpace::Reserve(const uintptr_t address, const size_t count)
{
	CPrintf(Debug, "Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, Start, End, count);
	Assert(count != 0);

	//Check requested address
	if (!IsFree(address, count))
		return false;

	//Claim
	m_reservations.Add(Reservation{address, count});

	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

size_t AddressSpace::GetCount(const uintptr_t address) const
{
	for (size_t i = 0; i < m_reservations.Count(); i++)
	{
		if (m_reservations[i].Address == address)
			return m_reservations[i].PageCount;
	}
	return 0;
}

bool AddressSpace::Free(const uintptr_t address, const size_t count)
{
	CPrintf(Debug, "Free: 0x%016x Count: 0x%x\n", address, count);

	for (size_t i = 0; i < m_reservations.Count(); i++)
	{
		if (m_reservations[i].Address == address)
		{
			Assert(m_reservations[i].PageCount == count);
			return m_reservations.RemoveAt(i);
		}
	}

	return false;
}

void AddressSpace::Display() const
{
	Printf("  Reservations: %d\n", m_reservations.Count());
	for (const Reservation& item : m_reservations)
	{
		const uintptr_t high = item.Address + (item.PageCount << Arch::PageShift);
		Printf("    [0x%016X-0x%016X] (Count: 0x%x)\n", item.Address, high, item.PageCount);
	}
}

bool AddressSpace::IsFree(const uintptr_t address, const size_t count) const
{
	Assert((address & Arch::PageMask) == 0);

	if ((address < Start) || (address >= End))
		return false;

	struct Lookup
	{
		uintptr_t Low;
		uintptr_t High;
	};

	const Lookup lookup = { address, address + (count << Arch::PageShift) };

	if (lookup.High > End)
		return false;

	for (const Reservation& res : m_reservations)
	{
		const uintptr_t end = res.Address + (res.PageCount << Arch::PageShift);

		if ((lookup.Low >= res.Address) && (lookup.Low < end))
			return false;

		if ((lookup.High > res.Address) && (lookup.High <= end))
			return false;

		if ((res.Address >= lookup.Low) && (res.Address < lookup.High))
			return false;
	}

	return true;
}
