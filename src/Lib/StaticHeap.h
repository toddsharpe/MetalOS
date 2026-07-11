#pragma once

#include <cstddef>
#include <cstdint>
#include "Assert.h"

class Heap
{
public:
	static constexpr size_t Alignment = 16;

	constexpr Heap() :
		m_heap(),
		m_size(),
		m_head(),
		m_inUse()
	{
	}

	Heap(void * const heap, const size_t size) :
		m_heap(heap),
		m_size(size),
		m_head(reinterpret_cast<Block *>(heap)),
		m_inUse()
	{
		m_head->Size = size - sizeof(Block);
		m_head->Prev = nullptr;
		m_head->Next = nullptr;
		m_head->Free = true;
	}

	void Open(void * const heap, const size_t size)
	{
		Assert(heap);
		Assert(size);

		//Open
		m_heap = heap;
		m_size = size;
		m_head = reinterpret_cast<Block *>(heap);
		m_inUse = 0;

		//Build first block
		m_head->Size = size - sizeof(Block);
		m_head->Prev = nullptr;
		m_head->Next = nullptr;
		m_head->Free = true;
	}

	void *Allocate(const size_t size)
	{
		if (!IsValid())
		{
			Assert(false);
			return nullptr;
		}
		
		const size_t aligned = AlignUp(size);

		for (Block *block = m_head; block != nullptr; block = block->Next)
		{
			if (!block->Free || block->Size < aligned)
				continue;

			// Split if the remainder fits another block header plus at least one allocation unit
			if (block->Size >= aligned + sizeof(Block) + Alignment)
			{
				Block *split = reinterpret_cast<Block *>(Data(block) + aligned);
				split->Size = block->Size - aligned - sizeof(Block);
				split->Prev = block;
				split->Next = block->Next;
				split->Free = true;

				if (block->Next)
					block->Next->Prev = split;

				block->Size = aligned;
				block->Next = split;
			}

			block->Free = false;
			m_inUse += block->Size;
			return Data(block);
		}

		Assert(false);
		return nullptr;
	}

	template <typename T>
	T *Allocate()
	{
		void *const allocated = Allocate(sizeof(T));
		if (!allocated)
			return nullptr;

		T *created = new (allocated) T();
		return created;
	}

	template <typename T, typename... A>
	T *Allocate(A &&...args)
	{
		void *const allocated = Allocate(sizeof(T));
		if (!allocated)
			return nullptr;

		T *created = new (allocated) T(args...);
		return created;
	}

	void Deallocate(const void *const ptr)
	{
		if (!ptr)
			return;

		Block *block = reinterpret_cast<Block *>(static_cast<uint8_t *>(const_cast<void *>(ptr)) - sizeof(Block));
		m_inUse -= block->Size;
		block->Free = true;

		// Coalesce forward: absorb consecutive free blocks after this one
		while (block->Next && block->Next->Free)
		{
			block->Size += sizeof(Block) + block->Next->Size;
			block->Next = block->Next->Next;
			if (block->Next)
				block->Next->Prev = block;
		}

		// Coalesce backward: let the free predecessor absorb this block
		if (block->Prev && block->Prev->Free)
		{
			Block *prev = block->Prev;
			prev->Size += sizeof(Block) + block->Size;
			prev->Next = block->Next;
			if (block->Next)
				block->Next->Prev = prev;
		}
	}

	// Grow/shrink an allocation, preserving its contents (allocate-copy-free).
	void *Reallocate(const void *const ptr, const size_t size)
	{
		if (!ptr)
			return Allocate(size);
		if (size == 0)
		{
			Deallocate(ptr);
			return nullptr;
		}

		const Block *const block = reinterpret_cast<const Block *>(static_cast<const uint8_t *>(ptr) - sizeof(Block));
		const size_t oldSize = block->Size;

		void *const moved = Allocate(size);
		if (!moved)
			return nullptr;

		const size_t copy = oldSize < size ? oldSize : size;
		uint8_t *const dst = static_cast<uint8_t *>(moved);
		const uint8_t *const src = static_cast<const uint8_t *>(ptr);
		for (size_t i = 0; i < copy; i++)
			dst[i] = src[i];

		Deallocate(ptr);
		return moved;
	}

	void Display() const
	{
		// Display the heap blocks for debugging
		Printf("Heap at %p, size %zu:\n", m_heap, m_size);
		for (Block *block = m_head; block != nullptr; block = block->Next)
		{
			Printf("  Block at %p: size %zu, %s\n", block, block->Size, block->Free ? "free" : "allocated");
		}
	}

	bool IsValid() const
	{
		return m_heap && m_size;
	}

	size_t InUse() const
	{
		return m_inUse;
	}

private:
	struct alignas(Alignment) Block
	{
		size_t Size;
		Block *Prev;
		Block *Next;
		bool Free;
	};

	static_assert(sizeof(Block) % Alignment == 0, "Block size must be a multiple of Alignment");

	static constexpr size_t AlignUp(const size_t size)
	{
		return (size + Alignment - 1) & ~(Alignment - 1);
	}

	static uint8_t *Data(Block * const block)
	{
		return reinterpret_cast<uint8_t *>(block) + sizeof(Block);
	}

	void *m_heap;
	size_t m_size;
	Block *m_head;
	size_t m_inUse;
};

template <size_t N>
class StaticHeap : public Heap
{
public:
	StaticHeap() :
		Heap(m_heap, N)
	{
	}

	static constexpr size_t Size = N;

private:
	alignas(Alignment) uint8_t m_heap[N];
};
