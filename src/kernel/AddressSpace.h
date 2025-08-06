#pragma once

#include "kernel/Arch.h"
#include "Lib/List.h"
#include "Lib/Arena.h"
#include "MetalOS.Space.h"

class AddressSpace
{
public:
	AddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal);

	void Initialize();

	bool IsValidPointer(const void* const address);
	bool Reserve(Arena& arena, uintptr_t& address, const size_t count);

	void Display() const;

	const uintptr_t Start;
	const uintptr_t End;
	const bool IsGlobal;

	bool Debug;

private:

	struct Reservation
	{
		ListEntry Link;
		uintptr_t Address;
		size_t PageCount;
	};

	bool IsFree(const uintptr_t address, const size_t count) const;

	//Reservations
	uintptr_t m_watermark;
	ListHead m_reservations;
};

class KAddressSpace : public AddressSpace
{
public:
	KAddressSpace();
};

class UAddressSpace : public AddressSpace
{
public:
	UAddressSpace();
};
