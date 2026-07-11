#include "Lib/StaticHeap.h"

void StaticHeap_test()
{
	// Basic allocation
	{
		StaticHeap<1024> heap;
		void *a = heap.Allocate(64);
		void *b = heap.Allocate(64);
		Assert(a != b);
	}

	// Returned pointers are Alignment-aligned
	{
		StaticHeap<1024> heap;
		void *a = heap.Allocate(1);
		Assert(a != nullptr);
		Assert((reinterpret_cast<uintptr_t>(a) % Heap::Alignment) == 0);
	}

	// Free and re-allocate the same slot
	{
		StaticHeap<256> heap;
		void *a = heap.Allocate(64);
		Assert(a != nullptr);
		heap.Deallocate(a);
		void *b = heap.Allocate(64);
		Assert(b == a); // first-fit should return the same block
	}

	// Forward coalescing: free two adjacent blocks, then allocate the combined size
	{
		StaticHeap<512> heap;
		void *a = heap.Allocate(64);
		void *b = heap.Allocate(64);
		Assert(a != nullptr && b != nullptr);
		heap.Deallocate(a);
		heap.Deallocate(b); // b is freed after a; forward coalesce merges b into a
		void *c = heap.Allocate(128);
		Assert(c != nullptr);
		Assert(c == a); // coalesced region starts where a was
	}

	// Backward coalescing: free later block first, then earlier block absorbs it
	{
		StaticHeap<512> heap;
		void *a = heap.Allocate(64);
		void *b = heap.Allocate(64);
		Assert(a != nullptr && b != nullptr);
		heap.Deallocate(b); // free b first
		heap.Deallocate(a); // a is freed last; backward coalesce should absorb b
		void *c = heap.Allocate(128);
		Assert(c != nullptr);
		Assert(c == a);
	}

	// Deallocating nullptr is a no-op
	{
		StaticHeap<256> heap;
		heap.Deallocate(nullptr); // must not crash
	}

	// Templated Allocate calls constructor
	{
		struct Val
		{
			int x;
			Val() :
				x(42)
			{
			}
		};

		StaticHeap<256> heap;
		Val *v = heap.Allocate<Val>();
		Assert(v != nullptr);
		Assert(v->x == 42);
	}

	// InUse starts at zero
	{
		StaticHeap<256> heap;
		AssertEqual(heap.InUse(), (size_t)0);
	}

	// InUse tracks aligned allocation size
	{
		StaticHeap<1024> heap;
		heap.Allocate(16);
		AssertEqual(heap.InUse(), (size_t)16);

		heap.Allocate(32);
		AssertEqual(heap.InUse(), (size_t)48);
	}

	// InUse decrements on Deallocate, reaches zero when all freed
	{
		StaticHeap<512> heap;
		void *a = heap.Allocate(64);
		void *b = heap.Allocate(64);
		AssertEqual(heap.InUse(), (size_t)128);

		heap.Deallocate(a);
		AssertEqual(heap.InUse(), (size_t)64);

		heap.Deallocate(b);
		AssertEqual(heap.InUse(), (size_t)0);
	}

	// InUse unaffected by Deallocate(nullptr)
	{
		StaticHeap<256> heap;
		heap.Allocate(32);
		const size_t before = heap.InUse();
		heap.Deallocate(nullptr);
		AssertEqual(heap.InUse(), before);
	}

	// Templated Allocate with arguments
	{
		struct Pair
		{
			int x, y;
			Pair(int a, int b) :
				x(a),
				y(b)
			{
			}
		};

		StaticHeap<256> heap;
		Pair *p = heap.Allocate<Pair>(3, 7);
		Assert(p != nullptr);
		Assert(p->x == 3 && p->y == 7);
	}

	// Reallocate(nullptr, size) behaves like Allocate
	{
		StaticHeap<256> heap;
		void *a = heap.Reallocate(nullptr, 64);
		Assert(a != nullptr);
		AssertEqual(heap.InUse(), (size_t)64);
	}

	// Reallocate(ptr, 0) frees and returns nullptr
	{
		StaticHeap<256> heap;
		void *a = heap.Allocate(64);
		Assert(a != nullptr);
		void *b = heap.Reallocate(a, 0);
		Assert(b == nullptr);
		AssertEqual(heap.InUse(), (size_t)0);
	}

	// Grow preserves the original bytes
	{
		StaticHeap<512> heap;
		uint8_t *a = static_cast<uint8_t *>(heap.Allocate(16));
		for (uint8_t i = 0; i < 16; i++)
			a[i] = i;

		uint8_t *b = static_cast<uint8_t *>(heap.Reallocate(a, 64));
		Assert(b != nullptr);
		for (uint8_t i = 0; i < 16; i++)
			AssertEqual(b[i], i);
	}

	// Shrink preserves the surviving prefix
	{
		StaticHeap<512> heap;
		uint8_t *a = static_cast<uint8_t *>(heap.Allocate(64));
		for (uint8_t i = 0; i < 64; i++)
			a[i] = i;

		uint8_t *b = static_cast<uint8_t *>(heap.Reallocate(a, 16));
		Assert(b != nullptr);
		for (uint8_t i = 0; i < 16; i++)
			AssertEqual(b[i], i);
	}

	// Reallocate always moves (allocate-copy-free) and InUse tracks the new size
	{
		StaticHeap<256> heap;
		void *a = heap.Allocate(32);
		Assert(a != nullptr);
		void *b = heap.Reallocate(a, 48); // new block taken before old is freed, so b != a
		Assert(b != nullptr && b != a);
		AssertEqual(heap.InUse(), (size_t)48);
	}
}
