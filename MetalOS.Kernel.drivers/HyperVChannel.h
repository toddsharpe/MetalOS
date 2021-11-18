#pragma once

#include <linux\hyperv.h>
#include "HyperVRingBuffer.h"
#include "VmBusDriver.h"

//Channel allocates one contigious section of memory
//Builds outbound and inbound ring buffers
//Each ring buffer has 1 hv struct to start, followed by pages that are wrap around mapped once
//Both buffers touch physical space - gpadl assumes upstream is at 0 and has a downstream offset
//Virtual: [0-hv_page|1|2|3|4| <non-contiguous>  5-hv_page|6|7|8|9|
//Physical:        [0|1|2|1|2|                           3|4|5|4|5|

class HyperVChannel
{
public:
	HyperVChannel(size_t sendSize, size_t receiveSize, CallContext callback);

	void Initialize(vmbus_channel_offer_channel* offerChannel, const ReadOnlyBuffer* buffer = nullptr);

	void SendPacket(const void* buffer, const size_t length, const uint64_t requestId, const vmbus_packet_type type, const uint32_t flags);

	//start read
	void* ReadPacket(const uint32_t length);
	void NextPacket(const uint32_t length);
	void StopRead();

	void SetEvent();

	void Display();


private:
	size_t m_sendCount;
	size_t m_receiveCount;
	paddr_t m_address;
	CallContext m_callback;

	HyperVRingBuffer m_inbound;
	HyperVRingBuffer m_outbound;

	uint32_t m_gpadlHandle;

	vmbus_channel_offer_channel* m_channel;

	VmBusDriver* m_vmbus;
};

