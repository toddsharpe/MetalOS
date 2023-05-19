#pragma once


#include "MetalOS.Kernel.h"
#include <kernel/MetalOS.Internal.h>
#include <shared/MetalOS.Types.h>
#include <shared/MetalOS.System.h>

#include <vector>

class VirtualAddressSpace
{
public:
	static const size_t AllocationGranularity = (PageSize << 2);//16KB

	VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool isGlobal);
	
	bool IsFree(const uintptr_t address, const size_t count) const;
	bool Reserve(uintptr_t& address, const size_t count);

	bool IsValidPointer(const void* p) const;

	const uintptr_t Start;
	const uintptr_t End;
	const bool IsGlobal;

private:
	struct Entry
	{
		uintptr_t Address;
		size_t PageCount;
	};

	uintptr_t m_watermark;
	std::vector<Entry> m_entries;
};

class UserAddressSpace : public VirtualAddressSpace
{
public:
	UserAddressSpace() : VirtualAddressSpace(UserAddress::UserStart, UserAddress::UserStop, false) { }
};

