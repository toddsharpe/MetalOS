#pragma once


#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include <kernel/MetalOS.Internal.h>
#include <shared/MetalOS.Types.h>

class VirtualAddressSpace
{
public:
	static const uint32_t AllocationGranularity = (PAGE_SIZE << 2);//16KB

	VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool global);
	
	void Init();

	bool IsFree(const uintptr_t address, const size_t count);
	bool Reserve(uintptr_t& address, const size_t count, const MemoryProtection protection);

	bool IsValidPointer(const void* p);

	bool IsGlobal() const
	{
		return m_global;
	}

	uintptr_t GetStart() const
	{
		return m_start;
	}

	size_t GetLength() const
	{
		return m_end - m_start;
	}

	struct Node
	{
		size_t PageCount;
		MemoryProtection Protection;
	};

	uintptr_t m_start;
	uintptr_t m_end;
	bool m_global;

	uintptr_t m_watermark;
	std::map<uintptr_t, Node> m_nodes;
};

class UserAddressSpace : public VirtualAddressSpace
{
public:
	UserAddressSpace() : VirtualAddressSpace(UserAddress::UserStart, UserAddress::UserStop, false) { }
};

