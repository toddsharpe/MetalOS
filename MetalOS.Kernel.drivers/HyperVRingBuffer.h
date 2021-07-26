#pragma once

#include "MetalOS.Kernel.h"

class HyperVChannel;
class HyperVRingBuffer//TODO: inbound ring buffer, outbound ring buffer
{
public:
	HyperVRingBuffer(const paddr_t address, const size_t count, HyperVChannel& channel);

	void Write(const struct kvec* kv_list, uint32_t kv_count);

	void Increment(const uint32_t length);
	void* Read(const uint32_t length);

	void CommitRead();

	void Display();

private:
#define VMBUS_PKT_TRAILER	8
	uint32_t Copy(const uint32_t location, const void* buffer, const uint32_t length);

	volatile hv_ring_buffer* m_header;
	uint32_t m_size;
	uint32_t m_dataSize;

	//Dont write the header read index until completely done reading (since HV may then start overwriting it) so this is temp
	uint32_t m_iterator;

	HyperVChannel& m_channel;
};

