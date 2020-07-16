#pragma once
#include <cstdint>

class Heap
{
public:
	void Initialize(size_t size);

	void* Allocate(const size_t size);
	void* Reallocate(void* address, size_t size);
	void Deallocate(void* address);

private:
	static const size_t MinBlockSize = 8;
	static constexpr size_t Alignment = 16;

	static constexpr size_t HeapAlign(const size_t x)
	{
		const size_t alignmentMask = Alignment - 1;
		return ((x + alignmentMask) & ~(alignmentMask));
	}

	//TODO: double linked list implementation without size
	struct HeapBlock
	{
		HeapBlock* Next;
		union
		{
			size_t Size;
			size_t Free : 1;
		};
		
		uint8_t Data[];

		//Mask out flags
		size_t GetSize()
		{
			return Size & ~0x1;
		}
	};
	static_assert(sizeof(HeapBlock) == 16, "Heap block has changed");

	size_t m_size;
	void* m_address;

	HeapBlock* m_head;
};

