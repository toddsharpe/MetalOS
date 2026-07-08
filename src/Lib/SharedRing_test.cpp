#include "Lib/SharedRing.h"
#include "Assert.h"

//SharedRing lays a SPSC ring over a raw region (real use: page-aligned shared memory).
//The tests back it with an aligned stack buffer sized by RegionSize().
void SharedRing_test()
{
	// RegionSize accounts for the header plus 'capacity' records
	{
		Assert(SharedRing<uint32_t>::RegionSize(4) == sizeof(SharedRingHeader) + 4 * sizeof(uint32_t));
		Assert(SharedRing<uint64_t>::RegionSize(8) == sizeof(SharedRingHeader) + 8 * sizeof(uint64_t));
	}

	// Init leaves the ring empty and stamps capacity + record size into the header
	{
		constexpr uint32_t cap = 4;
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);

		const SharedRingHeader* const header = reinterpret_cast<const SharedRingHeader*>(region);
		Assert(header->Capacity == cap);
		Assert(header->RecordSize == sizeof(uint32_t));

		SharedRing<uint32_t> ring(region);
		Assert(ring.IsEmpty());

		uint32_t value = 0;
		Assert(!ring.Dequeue(value)); // empty rejects
	}

	// Basic enqueue/dequeue round trip preserves values in FIFO order
	{
		constexpr uint32_t cap = 8;
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);
		SharedRing<uint32_t> ring(region);

		for (uint32_t i = 1; i <= 5; i++)
			Assert(ring.Enqueue(i));
		Assert(!ring.IsEmpty());

		for (uint32_t i = 1; i <= 5; i++)
		{
			uint32_t value = 0;
			Assert(ring.Dequeue(value));
			Assert(value == i);
		}
		Assert(ring.IsEmpty());
	}

	// One slot is kept empty: capacity N holds N-1 records, then enqueue rejects
	{
		constexpr uint32_t cap = 4; // 3 usable
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);
		SharedRing<uint32_t> ring(region);

		Assert(ring.Enqueue(10));
		Assert(ring.Enqueue(20));
		Assert(ring.Enqueue(30));
		Assert(!ring.Enqueue(40)); // full: the reserved empty slot blocks a 4th
		Assert(!ring.IsEmpty());

		uint32_t value = 0;
		Assert(ring.Dequeue(value) && value == 10); // freeing a slot re-opens enqueue
		Assert(ring.Enqueue(40));
		Assert(ring.Dequeue(value) && value == 20);
		Assert(ring.Dequeue(value) && value == 30);
		Assert(ring.Dequeue(value) && value == 40);
		Assert(ring.IsEmpty());
	}

	// Wrap-around (near-empty): enqueue-then-dequeue many times so the indices wrap
	{
		constexpr uint32_t cap = 4;
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);
		SharedRing<uint32_t> ring(region);

		for (uint32_t i = 0; i < 20; i++) // >> cap, forces head/tail to wrap repeatedly
		{
			Assert(ring.Enqueue(i));
			uint32_t value = 0;
			Assert(ring.Dequeue(value));
			Assert(value == i);
		}
		Assert(ring.IsEmpty());
	}

	// Wrap-around (near-full): keep the ring at max occupancy while the indices wrap
	{
		constexpr uint32_t cap = 4; // 3 usable
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);
		SharedRing<uint32_t> ring(region);

		uint32_t enq = 0;
		uint32_t deq = 0;
		Assert(ring.Enqueue(enq++)); // prime with 2 records
		Assert(ring.Enqueue(enq++));
		for (int i = 0; i < 20; i++)
		{
			Assert(ring.Enqueue(enq++)); // top up to the 3-record max
			uint32_t value = 0;
			Assert(ring.Dequeue(value));
			Assert(value == deq++);
		}
		uint32_t value = 0;
		while (ring.Dequeue(value))
			Assert(value == deq++);
		Assert(deq == enq); // everything enqueued came back out, in order
	}

	// Separate producer/consumer views over one region (the real cross-process use)
	{
		constexpr uint32_t cap = 8;
		alignas(8) uint8_t region[SharedRing<uint32_t>::RegionSize(cap)] = {};
		SharedRing<uint32_t>::Init(region, cap);

		SharedRing<uint32_t> producer(region);
		SharedRing<uint32_t> consumer(region);

		Assert(consumer.IsEmpty());
		Assert(producer.Enqueue(0xABCD));
		Assert(!consumer.IsEmpty()); // producer's write is visible through the consumer view

		uint32_t value = 0;
		Assert(consumer.Dequeue(value));
		Assert(value == 0xABCD);
		Assert(producer.IsEmpty());
	}

	// Struct records round-trip byte-exact (records are memcpy'd whole)
	{
		struct Record
		{
			uint32_t A;
			uint16_t B;
			uint8_t  C[3];
		};

		constexpr uint32_t cap = 4;
		alignas(8) uint8_t region[SharedRing<Record>::RegionSize(cap)] = {};
		SharedRing<Record>::Init(region, cap);
		SharedRing<Record> ring(region);

		const Record in = { 0x11223344, 0x5566, { 7, 8, 9 } };
		Assert(ring.Enqueue(in));

		Record out = {};
		Assert(ring.Dequeue(out));
		Assert(out.A == in.A && out.B == in.B);
		Assert(out.C[0] == 7 && out.C[1] == 8 && out.C[2] == 9);
	}
}
