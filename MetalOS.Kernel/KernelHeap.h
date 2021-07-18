#pragma once

#include "MetalOS.Kernel.h"
#include "VirtualAddressSpace.h"
#include "VirtualMemoryManager.h"

class KernelHeap
{
public:
	KernelHeap(VirtualMemoryManager& virtualMemory, VirtualAddressSpace& addressSpace);

	void* Allocate(const size_t size, const uintptr_t callerAddress);
	void Deallocate(const void* address);

	void PrintHeap() const;
	void PrintHeapHeaders() const;
	uint32_t GetAllocated() const
	{
		return m_allocated;
	}

	static const uint32_t MinBlockSize = 4;
	static constexpr uint16_t Magic = 0xBEEF;

private:
	void Grow(size_t pages);

	//TODO: remove magic, use bit of address for free (min alloc = 2)
#pragma pack(push, 1)
	struct HeapBlock
	{
		HeapBlock* Next;
		HeapBlock* Prev;
		union
		{
			struct
			{
				uint16_t Free : 1;
				uint16_t Magic;//Debug only, verify block integrity
			};
			uint64_t Flags;
		};
		uintptr_t CallerAddress; //Address of caller that allocated block
		uint8_t Data[];

		//Doesn't include header
		size_t GetLength()
		{
			if (Next == nullptr)
				return 0;
			return (size_t)Next - (size_t)Data;
		}
	};
#pragma pack(pop)
	static_assert(sizeof(HeapBlock) == 32, "Heap block has changed");

	VirtualMemoryManager& m_memoryManager;
	VirtualAddressSpace& m_addressSpace;

	uint64_t m_address;
	uint64_t m_end;
	uint32_t m_allocated; //Total size of objects allocated, not space being taken up with padding/alignment/overhead
	HeapBlock* m_head;
};

