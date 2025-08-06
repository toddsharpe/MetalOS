#pragma once

#include "kernel/AddressSpace.h"
#include "kernel/Api.h"

AddressSpace::AddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal) :
	Start(start),
	End(end),
	IsGlobal(isGlobal),
	Debug(),
	m_watermark(),
	m_reservations()
{

}

void AddressSpace::Initialize()
{
	ListInitializeHead(m_reservations);
}

bool AddressSpace::IsValidPointer(const void* const address)
{
	const uintptr_t page = (uintptr_t)address & ~PageMask;
	return !IsFree(page, 1);
}

bool AddressSpace::Reserve(Arena& arena, uintptr_t& address, const size_t count)
{
	Assert(count != 0);
	CPrintf(Debug, "Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, Start, End, count);

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
			AssertOp(m_watermark, <, End);
			m_watermark += AllocationGranularity;
		}
		
		address = m_watermark;
		m_watermark += ByteAlign((count << PageShift), AllocationGranularity);;
		CPrintf(Debug, "  NewWatermark: 0x%016x\n", m_watermark);
	}

	Reservation* res = arena.Allocate<Reservation>();
	Assert(res);
	res->Address = address;
	res->PageCount = count;
	ListInsertTail(m_reservations, res->Link);

	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

void AddressSpace::Display() const
{
	if (IsGlobal)
		Printf("KAddressSpace\n");
	else
		Printf("UAddressSpace\n");

	Printf("  Reservations: %d\n", m_reservations.Count);
	ListForEach<Reservation>(m_reservations, [](const ListEntry& entry, const Reservation& item)
	{
		const uintptr_t high = item.Address + (item.PageCount << PageShift);
		Printf("    [0x%016X-0x%016X] (Count: 0x%x)\n", item.Address, high, item.PageCount);
	});
}

bool AddressSpace::IsFree(const uintptr_t address, const size_t count) const
{
	Assert((address & PageMask) == 0);

	//Check if address is inside out address space
	if ((address < Start) || (address >= End))
		return false;

	struct Lookup
	{
		uintptr_t Low;
		uintptr_t High;
	};

	Lookup lookup = {};
	lookup.Low = address;
	lookup.High = address + (count << PageShift);

	const bool found = ListAny<Reservation, Lookup>(m_reservations, [](const ListEntry&, const Reservation& res, Lookup& lookup)
	{
		const uintptr_t end = res.Address + (res.PageCount << PageShift);

		//Check start address
		if ((lookup.Low >= res.Address) && (lookup.Low < end))
			return true;

		//Check ending address
		if ((lookup.High >= res.Address) && (lookup.High < end))
			return true;

		return false;
	}, lookup);

	return !found;
}

KAddressSpace::KAddressSpace() : AddressSpace(KernelStart, KernelEnd, true)
{

}

UAddressSpace::UAddressSpace() : AddressSpace(UserStart, UserEnd, false)
{

}
