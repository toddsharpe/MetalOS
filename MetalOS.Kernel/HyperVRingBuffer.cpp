#include "HyperVRingBuffer.h"

#include "Main.h"
#include "VmBusDriver.h"
#include "HyperVChannel.h"

HyperVRingBuffer::HyperVRingBuffer(const paddr_t address, const size_t count, HyperVChannel& channel) :
	m_channel(channel),
	m_size(count << PAGE_SHIFT),
	m_iterator(),
	m_dataSize((count - 1) << PAGE_SHIFT)
{
	std::vector<paddr_t> addresses;
	addresses.push_back(address);//hv page

	//Push physical addresses twice (for wraparound)
	for (size_t i = 0; i < count - 1; i++)
	{
		addresses.push_back(address + ((i + 1) << PAGE_SHIFT));
	}
	for (size_t i = 0; i < count - 1; i++)
	{
		addresses.push_back(address + ((i + 1) << PAGE_SHIFT));
	}
	Assert(addresses.size() == 2 * count - 1);

	void* base = kernel.VirtualMap(nullptr, addresses, MemoryProtection(true, true, false));
	memset(base, 0, PAGE_SIZE * count);

	Print("Virtual 0x%016x, Physical: 0x%016x, Size: 0x%x\n", base, address, m_size);
	m_header = (volatile hv_ring_buffer * )base;
	m_header->feature_bits.value = 1;
}

void HyperVRingBuffer::Write(const struct kvec* kv_list, uint32_t kv_count)
{
	Print("Write 0x%x\n", kv_count);
	
	//TODO: buffer full etc

	//Copy into ring buffer
	const uint32_t startIndex = m_header->write_index;
	uint32_t index = startIndex;
	for (size_t i = 0; i < kv_count; i++)
	{
		index = this->Copy(index, kv_list[i].iov_base, kv_list[i].iov_len);
	}

	//Write new indexes
	Print("Write: 0x%x Read: 0x%x\n", m_header->write_index, m_header->read_index);
	uint64_t indexes = ((uint64_t)m_header->write_index << 32) + m_header->read_index;
	index = this->Copy(index, &indexes, sizeof(uint64_t));
	__faststorefence();

	//Update write location
	m_header->write_index = index;

	__faststorefence();

	//Signal
	if (m_header->interrupt_mask)
		return;

	__faststorefence();

	//If ring buffer was empty, signal device
	if (startIndex == m_header->read_index)
	{
		m_channel.SetEvent();
	}
}

//Copies and returns next location
uint32_t HyperVRingBuffer::Copy(const uint32_t location, const void* buffer, const uint32_t length)
{
	Print("  Copy: 0x%x, 0x%016x, 0x%x\n", location, buffer, length);
	kernel.PrintBytes((const char*)buffer, length);
	memcpy((void*)&m_header->buffer[location], buffer, length);

	return (location + length) % m_dataSize;
}

void HyperVRingBuffer::Display()
{
	kernel.Printf("Read: 0x%08x\n", m_header->read_index);
	kernel.Printf("Iterator: 0x%08x\n", m_iterator);
	kernel.Printf("Write: 0x%08x\n", m_header->write_index);
	kernel.Printf("Mask: 0x%08x\n", m_header->interrupt_mask);
	kernel.PrintBytes((const char*)&m_header->buffer[0], 256);
}

void* HyperVRingBuffer::Read(const uint32_t length)
{
	if (m_iterator == m_header->write_index)
		return nullptr;
	
	void* top = (void*)&m_header->buffer[m_iterator];
	return top;
}

void HyperVRingBuffer::Increment(const uint32_t length)
{
	m_iterator = (m_iterator + length) % m_dataSize;
}

void HyperVRingBuffer::CommitRead()
{
	__faststorefence();
	m_header->read_index = m_iterator;
	
	//Ring buffer is not blocked, don't signal
	if (!m_header->pending_send_sz)
		return;

	__faststorefence();

	//Do math to figure out if blocked

	m_channel.SetEvent();
}

