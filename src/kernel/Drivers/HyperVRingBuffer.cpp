#include "kernel/Drivers/HyperVRingBuffer.h"
#include "kernel/Drivers/HyperVChannel.h"

HyperVRingBuffer::HyperVRingBuffer(const uint32_t size, HyperVChannel& channel) :
	Size(size),
	DataSize(size - Arch::PageSize),
	m_channel(channel),
	m_header(),
	m_iterator()
{
	
}

void HyperVRingBuffer::Initialize(const paddr_t address)
{
	//Number of pages
	const size_t count = Arch::SizeToPages(Size);

	//TODO(tsharpe): Temp arena/growing vector
	StaticVector<paddr_t, 32> addresses;
	addresses.Add(address);//hv page

	//Push physical addresses twice (for wraparound)
	for (size_t i = 0; i < count - 1; i++)
	{
		addresses.Add(address + ((i + 1) << Arch::PageShift));
	}
	for (size_t i = 0; i < count - 1; i++)
	{
		addresses.Add(address + ((i + 1) << Arch::PageShift));
	}
	Assert(addresses.Count() == 2 * count - 1);

	void* const base = KeVirtualMap(addresses.begin(), addresses.Count() << Arch::PageShift);
	memset(base, 0, Size);

	m_header = (volatile hv_ring_buffer * )base;
	m_header->feature_bits.value = 1;
}

bool HyperVRingBuffer::Write(const CBuffer* buffers, const size_t count)
{
	//Packet plus the trailing 8-byte previous-indices value
	size_t total = sizeof(uint64_t);
	for (size_t i = 0; i < count; i++)
		total += buffers[i].Size;

	//Refuse a packet that doesn't fit with a byte to spare: never overwrite unread data or make a full ring look empty
	const uint32_t startIndex = m_header->write_index;
	const uint32_t readIndex = m_header->read_index;
	const uint32_t available = (startIndex >= readIndex) ? (DataSize - (startIndex - readIndex)) : (readIndex - startIndex);
	if (available <= total)
		return false;

	//Copy into ring buffer
	uint32_t index = startIndex;
	for (size_t i = 0; i < count; i++)
	{
		index = this->Copy(index, buffers[i].Data, (uint32_t)buffers[i].Size);
	}

	//Write new indexes
	uint64_t indexes = (uint64_t)startIndex;
	index = this->Copy(index, &indexes, sizeof(uint64_t));
	__faststorefence();

	//Update write location
	m_header->write_index = index;

	__faststorefence();

	//Signal
	if (m_header->interrupt_mask)
		return true;

	__faststorefence();

	//If ring buffer was empty, signal device
	if (startIndex == m_header->read_index)
	{
		m_channel.SetEvent();
	}
	return true;
}

void HyperVRingBuffer::Increment(const uint32_t length)
{
	m_iterator = (m_iterator + length) % DataSize;
}

void* HyperVRingBuffer::Read(const uint32_t length)
{
	if (m_iterator == m_header->write_index)
		return nullptr;
	
	void* top = (void*)&m_header->buffer[m_iterator];
	return top;
}

bool HyperVRingBuffer::CommitRead()
{
	__faststorefence();
	m_header->read_index = m_iterator;
	__faststorefence();

	// Host may have written after our last Read() check but before read_index was committed.
	// If so, the host saw the ring as non-empty and suppressed its SINT.
	// Return true so the caller loops and drains the new data without waiting for an interrupt.
	const bool more = (m_header->write_index != m_iterator);

	if (!m_header->pending_send_sz)
		return more;

	m_channel.SetEvent();
	return more;
}

//Copies and returns next location
uint32_t HyperVRingBuffer::Copy(const uint32_t location, const void* buffer, const uint32_t length)
{
	memcpy((void*)&m_header->buffer[location], buffer, length);
	return (location + length) % DataSize;
}

void HyperVRingBuffer::Display()
{
	Printf("Read: 0x%08x\n", m_header->read_index);
	Printf("Iterator: 0x%08x\n", m_iterator);
	Printf("Write: 0x%08x\n", m_header->write_index);
	Printf("Mask: 0x%08x\n", m_header->interrupt_mask);
	PrintBytes((const char*)&m_header->buffer[0], 256);
}
