#include "Lib/CircularBuffer.h"
#include "Assert.h"

void CircularBuffer_test()
{
	// Basic FIFO ordering
	{
		CircularBuffer<int, 8> buffer;
		Assert(buffer.IsEmpty());
		Assert(!buffer.IsFull());
		Assert(buffer.Count() == 0);

		buffer.Enqueue(1);
		buffer.Enqueue(2);
		buffer.Enqueue(3);
		buffer.Enqueue(4);

		Assert(buffer.Count() == 4);
		Assert(!buffer.IsEmpty());

		Assert(buffer.Peek() == 1);
		Assert(buffer.Dequeue() == 1);
		Assert(buffer.Dequeue() == 2);
		Assert(buffer.Dequeue() == 3);
		Assert(buffer.Dequeue() == 4);

		Assert(buffer.IsEmpty());
		Assert(buffer.Count() == 0);
	}

	// Fill to capacity and verify IsFull
	{
		CircularBuffer<int, 4> buffer;
		Assert(buffer.Enqueue(1));
		Assert(buffer.Enqueue(2));
		Assert(buffer.Enqueue(3));
		Assert(buffer.Enqueue(4));
		Assert(buffer.IsFull());
		Assert(!buffer.Enqueue(5));  // Must reject when full

		Assert(buffer.Dequeue() == 1);
		Assert(!buffer.IsFull());
		Assert(buffer.Enqueue(5));  // Must accept after dequeue
		Assert(buffer.Dequeue() == 2);
		Assert(buffer.Dequeue() == 3);
		Assert(buffer.Dequeue() == 4);
		Assert(buffer.Dequeue() == 5);
		Assert(buffer.IsEmpty());
	}

	// Wrap-around: interleave enqueue and dequeue across the index boundary
	{
		CircularBuffer<int, 4> buffer;
		buffer.Enqueue(1);
		buffer.Enqueue(2);
		buffer.Dequeue();  // head advances to 1
		buffer.Dequeue();  // head advances to 2
		buffer.Enqueue(3);
		buffer.Enqueue(4);
		buffer.Enqueue(5);  // tail wraps around

		Assert(buffer.Count() == 3);
		Assert(buffer.Dequeue() == 3);
		Assert(buffer.Dequeue() == 4);
		Assert(buffer.Dequeue() == 5);
		Assert(buffer.IsEmpty());
	}

	// Single element: enqueue then dequeue repeatedly
	{
		CircularBuffer<int, 2> buffer;
		for (int i = 0; i < 8; i++)
		{
			Assert(buffer.Enqueue(i));
			Assert(buffer.Peek() == i);
			Assert(buffer.Dequeue() == i);
			Assert(buffer.IsEmpty());
		}
	}
}
