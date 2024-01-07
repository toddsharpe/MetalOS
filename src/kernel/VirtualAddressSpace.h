#pragma once

#include "MetalOS.List.h"
#include "MetalOS.Internal.h"
#include <cstdint>
#include <cstddef>

class VirtualAddressSpace
{
public:
	static bool Debug;

	VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal);
	
	void Initialize();
	bool Reserve(uintptr_t& address, const size_t count);
	bool IsValidPointer(const void* const p) const;

	const bool IsGlobal;

private:
	struct Reservation
	{
		uintptr_t Address;
		size_t PageCount;
		ListEntry Link;
	};

	bool IsFree(const uintptr_t address, const size_t count) const;

	ListEntry m_reservations;
	const uintptr_t m_start;
	const uintptr_t m_end;
	uintptr_t m_watermark;
	bool m_initialized;
};

class UserAddressSpace : public VirtualAddressSpace
{
public:
	UserAddressSpace() : VirtualAddressSpace(UserAddress::UserStart, UserAddress::UserStop, false)
	{

	}
};

/*
class KernelAddressSpace : public VirtualAddressSpace
{
public:
	KernelAddressSpace() : VirtualAddressSpace(KernelAddress::KernelStart, KernelAddress::KernelEnd, KernelAddress::KernelStart, true)
	{

	}
};
*/
