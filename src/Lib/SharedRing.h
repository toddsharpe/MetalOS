#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

// Single-producer / single-consumer ring of fixed-size records laid over a
// shared-memory region. Usable from both kernel and usermode (only depends on
// <cstdint>/<cstddef>/<cstring>). Layout in the region:
//
//   [SharedRingHeader][record 0][record 1] ... [record Capacity-1]
//
// Head is owned by the producer, Tail by the consumer; both are free-running
// indices modulo Capacity. One slot is left empty so full and empty are
// distinguishable without a separate count. The creator calls Init() once on
// the freshly mapped region; each side then attaches with the constructor.
struct SharedRingHeader
{
	volatile uint32_t Head;
	volatile uint32_t Tail;
	uint32_t Capacity;   // number of record slots (usable == Capacity - 1)
	uint32_t RecordSize; // bytes per record (sanity check across the boundary)
};

template <typename T>
class SharedRing
{
public:
	//Attach to a region already initialized by Init()
	explicit SharedRing(void* const region) :
		m_header(reinterpret_cast<SharedRingHeader*>(region)),
		m_records(reinterpret_cast<uint8_t*>(region) + sizeof(SharedRingHeader))
	{
	}

	//Initialize a freshly mapped region (creator only, before either side uses it)
	static void Init(void* const region, const uint32_t capacity)
	{
		SharedRingHeader* const h = reinterpret_cast<SharedRingHeader*>(region);
		h->Head = 0;
		h->Tail = 0;
		h->Capacity = capacity;
		h->RecordSize = sizeof(T);
	}

	//Bytes of shared region needed to hold 'capacity' slots
	static constexpr size_t RegionSize(const uint32_t capacity)
	{
		return sizeof(SharedRingHeader) + static_cast<size_t>(capacity) * sizeof(T);
	}

	bool Enqueue(const T& value)
	{
		const uint32_t head = m_header->Head;
		const uint32_t next = (head + 1) % m_header->Capacity;
		if (next == m_header->Tail)
			return false; //full
		memcpy(m_records + static_cast<size_t>(head) * sizeof(T), &value, sizeof(T));
		m_header->Head = next;
		return true;
	}

	bool Dequeue(T& value)
	{
		const uint32_t tail = m_header->Tail;
		if (tail == m_header->Head)
			return false; //empty
		memcpy(&value, m_records + static_cast<size_t>(tail) * sizeof(T), sizeof(T));
		m_header->Tail = (tail + 1) % m_header->Capacity;
		return true;
	}

	bool IsEmpty() const { return m_header->Tail == m_header->Head; }

private:
	SharedRingHeader* m_header;
	uint8_t* m_records;
};
