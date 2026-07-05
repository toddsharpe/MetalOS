#pragma once

#include "Lib/StaticVector.h"
#include <cstdlib>

class AddressSpace
{
public:
	constexpr AddressSpace(const uintptr_t start, const uintptr_t end, const uintptr_t offset = 0) :
		Start(start),
		End(end),
		Debug(false),
		m_watermark(start + offset),
		m_reservations()
	{

	}

	bool IsValidPointer(const void* const address);
	uintptr_t Reserve(const size_t count);
	bool Reserve(const uintptr_t address, const size_t count);
	bool Free(const uintptr_t address, const size_t count);
	size_t GetCount(const uintptr_t address) const;

	void Display() const;

	const uintptr_t Start;
	const uintptr_t End;

	bool Debug;

private:
	static constexpr uint64_t AllocationGranularity = 0x1'0000; //64K

	struct Reservation
	{
		uintptr_t Address;
		size_t PageCount;
	};

	bool IsFree(const uintptr_t address, const size_t count) const;

	//Reservations
	uintptr_t m_watermark;
	//LinkedList<Reservation> m_reservations;
	StaticVector<Reservation, 16> m_reservations;
};
