#pragma once

#include "Lib/LinkedList.h"
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

	//Validate every page spanned by [address, address+length) is reserved in this space.
	//Length-bounded so a user buffer near a mapping's end can't run into unmapped memory.
	bool IsValidRange(const void* const address, const size_t length) const;

	//Validate a typed object pointer over its full sizeof extent (not just its first page).
	template <typename T>
	bool IsValid(const T* const object) const { return IsValidRange(object, sizeof(T)); }

	uintptr_t Reserve(const size_t count);
	bool Reserve(const uintptr_t address, const size_t count);
	bool Free(const uintptr_t address, const size_t count);
	size_t GetCount(const uintptr_t address) const;
	size_t ReservedBytes() const;

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

		//Reservations are keyed by their (unique) address, so equality compares Address only.
		bool operator==(const Reservation& other) const { return Address == other.Address; }
	};

	bool IsFree(const uintptr_t address, const size_t count) const;

	//Reservations. Unbounded: the kernel address space in particular accumulates more than a
	//small fixed cap (fixed maps, UEFI ranges, thread stacks, transient loader aliases).
	uintptr_t m_watermark;
	LinkedList<Reservation> m_reservations;
};
