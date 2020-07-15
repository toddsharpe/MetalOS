#pragma once
#include <cstdint>

class Heap
{
public:
	void Initialize(size_t size);

	void* Allocate(const size_t size);
	void Deallocate(void* address);

private:
	static constexpr size_t HeapAlign(const size_t x)
	{
		const size_t alignment = 16;
		const size_t alignmentMask = alignment - 1;
		return ((x + alignmentMask) & ~(alignmentMask));
	}

#pragma pack(push, 1)
	struct HeapBlock
	{
		HeapBlock* Next;
		HeapBlock* Prev;
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
	static_assert(sizeof(HeapBlock) == 16, "Heap block has changed");

	size_t m_size;
	void* m_address;

	uint32_t m_allocated; //Total size of objects allocated, not space being taken up with padding/alignment/overhead
	HeapBlock* m_head;
};

