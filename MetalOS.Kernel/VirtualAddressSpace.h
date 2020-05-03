#pragma once

#include "msvc.h"
#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include <MetalOS.Internal.h>


class VirtualAddressSpace
{
public:
	VirtualAddressSpace(const uintptr_t start, const uintptr_t end, const bool global);
	
	bool IsFree(const uintptr_t address, const size_t count);
	bool Reserve(uintptr_t& address, const size_t count, MemoryProtection protection);

	bool IsGlobal()
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

private:
	static const uint32_t AllocationGranularity = (PAGE_SIZE << 2);//16KB
	
	struct Node
	{
		uint32_t PageCount;
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
	UserAddressSpace() : VirtualAddressSpace(0ULL, KernelStart, false) { }
};

