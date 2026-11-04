#pragma once

#include "kernel/Drivers/HyperVRingBuffer.h"
#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/HyperV/VmBus.h"

//Channel allocates one contigious section of memory
//Builds outbound and inbound ring buffers
//Each ring buffer has 1 hv struct to start, followed by pages that are wrap around mapped once
//Both buffers touch physical space - gpadl assumes upstream is at 0 and has a downstream offset
//Virtual: [0-hv_page|1|2|3|4| <non-contiguous>  5-hv_page|6|7|8|9|
//Physical:        [0|1|2|1|2|                           3|4|5|4|5|

class HyperVChannel
{
public:
	HyperVChannel(const uint32_t sendSize, const uint32_t receiveSize, const ActionContext callback);

	void Initialize(VmBusDriver& vmbus, HyperV::vmbus_channel_offer_channel* offerChannel);
	void Close(VmBusDriver& vmbus); //Undoes Initialize (vmbus_close_internal); the rings are freed only once the host releases their GPADL

	bool SendPacket(const void* buffer, const size_t length, const uint64_t requestId, const HyperV::vmbus_packet_type type, const uint32_t flags); //False if the outbound ring is full

	//Calls onPacket(descriptor, payload, payloadSize) per inbound packet; re-drains if StopRead reports more arrived
	template <typename F>
	void Drain(F onPacket)
	{
		HyperV::vmpacket_descriptor* packet;
		do
		{
			while ((packet = (HyperV::vmpacket_descriptor*)ReadPacket(sizeof(HyperV::vmpacket_descriptor))) != nullptr)
			{
				uint8_t* const payload = (uint8_t*)packet + (packet->offset8 << 3);
				const uint32_t size = (packet->len8 << 3) - (packet->offset8 << 3);
				onPacket(*packet, payload, size);

				NextPacket(packet->len8 << 3);
			}
		} while (StopRead());
	}

	void SetEvent();

private:
	void* ReadPacket(const uint32_t length);
	void NextPacket(const uint32_t length);
	bool StopRead();

	ActionContext m_callback;

	HyperVRingBuffer m_inbound;
	HyperVRingBuffer m_outbound;

	HyperV::vmbus_channel_offer_channel* m_channel;
	uint32_t m_gpadlHandle;
	paddr_t m_address; //both rings' physical pages, outbound first
	size_t m_pageCount;
};

