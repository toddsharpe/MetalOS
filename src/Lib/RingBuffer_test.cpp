#include "Lib/RingBuffer.h"
#include "Assert.h"

void RingBuffer_test()
{
	// Basic push/pop round trip
	{
		RingBuffer<8> rb;
		Assert(rb.IsEmpty());
		Assert(rb.Size() == 0);

		const uint8_t data[] = {1, 2, 3, 4};
		Assert(rb.Push(data, 4));
		Assert(rb.Size() == 4);
		Assert(!rb.IsEmpty());

		uint8_t out[4] = {};
		size_t read = 0;
		Assert(rb.Pop(out, 4, read));
		Assert(read == 4);
		Assert(out[0] == 1 && out[1] == 2 && out[2] == 3 && out[3] == 4);
		Assert(rb.IsEmpty());
		Assert(rb.Size() == 0);
	}

	// Full buffer: push rejects when no space remains
	{
		RingBuffer<4> rb;
		const uint8_t data[] = {0xA, 0xB, 0xC, 0xD};
		Assert(rb.Push(data, 4));
		Assert(rb.Size() == 4);
		const uint8_t extra[] = {0xFF};
		Assert(!rb.Push(extra, 1));
		Assert(rb.Size() == 4);
	}

	// Push more than capacity: rejected even on empty buffer
	{
		RingBuffer<4> rb;
		const uint8_t data[] = {1, 2, 3, 4, 5};
		Assert(!rb.Push(data, 5));
		Assert(rb.IsEmpty());
	}

	// Empty buffer: pop rejects
	{
		RingBuffer<4> rb;
		uint8_t out[4] = {};
		size_t read = 0;
		Assert(!rb.Pop(out, 4, read));
	}

	// Partial pop: pop fewer bytes than available
	{
		RingBuffer<8> rb;
		const uint8_t data[] = {1, 2, 3, 4, 5};
		Assert(rb.Push(data, 5));

		uint8_t out[8] = {};
		size_t read = 0;
		Assert(rb.Pop(out, 2, read));
		Assert(read == 2);
		Assert(out[0] == 1 && out[1] == 2);
		Assert(rb.Size() == 3);

		Assert(rb.Pop(out, 8, read));
		Assert(read == 3);
		Assert(out[0] == 3 && out[1] == 4 && out[2] == 5);
		Assert(rb.IsEmpty());
	}

	// Push exactly N bytes into empty buffer
	{
		RingBuffer<8> rb;
		const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
		Assert(rb.Push(data, 8));
		Assert(rb.Size() == 8);
		Assert(!rb.Push(data, 1));  // must reject when full

		uint8_t out[8] = {};
		size_t read = 0;
		Assert(rb.Pop(out, 8, read));
		Assert(read == 8);
		for (int i = 0; i < 8; i++)
			Assert(out[i] == (uint8_t)i);
		Assert(rb.IsEmpty());
	}

	// Wrap-around push: write pointer crosses end of storage
	{
		RingBuffer<4> rb;
		const uint8_t a[] = {1, 2, 3};
		Assert(rb.Push(a, 3));

		uint8_t out[4] = {};
		size_t read = 0;
		Assert(rb.Pop(out, 3, read));
		Assert(read == 3);
		// write_idx = 3, read_idx = 3; one byte fits at [3], two wrap to [0],[1]
		const uint8_t b[] = {4, 5, 6};
		Assert(rb.Push(b, 3));
		Assert(rb.Size() == 3);

		Assert(rb.Pop(out, 3, read));
		Assert(read == 3);
		Assert(out[0] == 4 && out[1] == 5 && out[2] == 6);
		Assert(rb.IsEmpty());
	}

	// Wrap-around pop: read pointer crosses end of storage
	{
		RingBuffer<4> rb;
		// Advance write and read to position 2 so pop straddles the end
		const uint8_t init[] = {0, 0};
		Assert(rb.Push(init, 2));
		uint8_t discard[2] = {};
		size_t read = 0;
		Assert(rb.Pop(discard, 2, read));
		// write_idx = 2, read_idx = 2; push 3 bytes: [2],[3],[0]
		const uint8_t data[] = {7, 8, 9};
		Assert(rb.Push(data, 3));

		uint8_t out[3] = {};
		Assert(rb.Pop(out, 3, read));
		Assert(read == 3);
		Assert(out[0] == 7 && out[1] == 8 && out[2] == 9);
		Assert(rb.IsEmpty());
	}

	// Interleaved push/pop preserves FIFO order across wrap
	{
		RingBuffer<8> rb;
		uint8_t out[8] = {};
		size_t read = 0;

		const uint8_t a[] = {1, 2, 3, 4, 5};
		Assert(rb.Push(a, 5));
		Assert(rb.Pop(out, 3, read));
		Assert(read == 3 && out[0] == 1 && out[1] == 2 && out[2] == 3);

		const uint8_t b[] = {6, 7, 8, 9, 10, 11};
		Assert(rb.Push(b, 6));  // 2 remaining + 6 = 8, exactly fits
		Assert(rb.Size() == 8);

		Assert(rb.Pop(out, 8, read));
		Assert(read == 8);
		const uint8_t expected[] = {4, 5, 6, 7, 8, 9, 10, 11};
		for (int i = 0; i < 8; i++)
			Assert(out[i] == expected[i]);
		Assert(rb.IsEmpty());
	}
}
