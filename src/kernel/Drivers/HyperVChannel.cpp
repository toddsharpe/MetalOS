#include "HyperVChannel.h"
#include "Arch.h"
#include "kernel/HyperV/Types.h"
#include "kernel/KDevice.h"
#include "Kernel/HyperV/HyperV.h"
#include "Lib/Buffer.h"
#include "kernel/Drivers/HyperVRingBuffer.h"
#include "Assert.h"

HyperVChannel::HyperVChannel(const uint32_t sendSize, const uint32_t receiveSize, const ActionContext callback) :
	m_callback(callback),
	m_outbound(sendSize, *this),//upstream
	m_inbound(receiveSize, *this),//downstream
	m_gpadlHandle(),
	m_channel()
{
	
}

void HyperVChannel::Initialize(HyperV::vmbus_channel_offer_channel* offerChannel, const CBuffer* buffer)
{
	m_channel = offerChannel;
	
	if (buffer != nullptr)
		Assert(buffer->Size <= MAX_USER_DEFINED_BYTES);

	Assert((m_outbound.Size % Arch::PageSize) == 0);
	Assert((m_inbound.Size % Arch::PageSize) == 0);

	//Get reference to vmbus
	KDevice* bus = KeGetDevice("\\_SB_.VMOD.VMBS");
	Assert(bus);
	VmBusDriver* const vmbus = static_cast<VmBusDriver*>(bus->Driver);
	
	//Number of pages
	const size_t sendCount = Arch::SizeToPages(m_outbound.Size);
	const size_t receiveCount = Arch::SizeToPages(m_inbound.Size);
	const size_t pageCount = sendCount + receiveCount;

	//Get physical contiguous region
	const paddr_t address = KePhysicalAlloc(pageCount);

	//Initialize rings (outbound first)
	m_outbound.Initialize(address);
	m_inbound.Initialize(address + m_outbound.Size);
	
	//Establish GPADL for ring buffers
	const size_t pfnsize = HyperV::MAX_SIZE_CHANNEL_MESSAGE - sizeof(HyperV::vmbus_channel_gpadl_header) - (sizeof(HyperV::gpa_range) - ANYSIZE_ARRAY * sizeof(uint64_t));
	const size_t pfncount = pfnsize / sizeof(uint64_t);

	//If this assertion fails, gpadl body message is needed
	Assert(!(pageCount > pfncount));

	StaticBuffer<256> arena;

	//Everything fits in the header
	const size_t msgsize = sizeof(HyperV::vmbus_channel_gpadl_header) + sizeof(HyperV::gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t);
	HyperV::vmbus_channel_gpadl_header* msg = (HyperV::vmbus_channel_gpadl_header*)arena.Data;
	memset(msg, 0, msgsize);
	const uint16_t bodySize = (uint16_t)(sizeof(HyperV::gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t));
	msg->rangecount = 1;
	msg->range_buflen = bodySize;
	msg->range[0].byte_offset = 0;
	msg->range[0].byte_count = (uint32_t)(pageCount << Arch::PageShift);
	for (size_t i = 0; i < pageCount; i++)
		msg->range[0].pfn_array[i] = (address >> Arch::PageShift) + i;

	msg->header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER;
	msg->child_relid = m_channel->child_relid;

	HyperV::VmBusResponse response;
	HyperV::HV_HYPERCALL_RESULT_VALUE result = vmbus->PostMessage((uint32_t)msgsize, msg, response);
	m_gpadlHandle = response.gpadl_created.gpadl;

	//Register the channel callback BEFORE opening: the host can signal the channel the instant it
	//is open, and if no handler is registered yet that notification is dropped (and the host may
	//rescind the channel). The rings are already initialized above, so the callback is safe to run.
	vmbus->SetCallback(m_channel->child_relid, m_callback);

	//Open channel
	HyperV::vmbus_channel_open_channel openChannel;
	memset(&openChannel, 0, sizeof(HyperV::vmbus_channel_open_channel));
	openChannel.header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL;
	openChannel.openid = m_channel->child_relid;
	openChannel.child_relid = m_channel->child_relid;
	openChannel.ringbuffer_gpadlhandle = m_gpadlHandle;
	openChannel.downstream_ringbuffer_pageoffset = (uint32_t)sendCount;
	if (buffer != nullptr)
		memcpy(openChannel.userdata, buffer->Data, buffer->Size);
	result = vmbus->PostMessage(sizeof(HyperV::vmbus_channel_open_channel), &openChannel, response);
}

//Format is vmpacket_descriptor followed by buffer, alignment if needed, and then old indexes
void HyperVChannel::SendPacket(const void* buffer, const size_t length, const uint64_t requestId, const HyperV::vmbus_packet_type type, const uint32_t flags)
{
	uint32_t packetlen = sizeof(HyperV::vmpacket_descriptor) + (uint32_t)length;
	const uint32_t packetlen_aligned = ByteAlign(packetlen, sizeof(uint64_t));

	HyperV::vmpacket_descriptor desc = { 0 };
	desc.type = type;
	desc.flags = flags;
	desc.offset8 = sizeof(HyperV::vmpacket_descriptor) >> 3;
	desc.len8 = (uint16_t)(packetlen_aligned >> 3);
	desc.trans_id = requestId;

	uint64_t aligned_data = 0;

	constexpr size_t headerSize = 1;
	constexpr size_t fullSize = 3;

	const CBuffer buffers[] = {
		{&desc, sizeof(HyperV::vmpacket_descriptor)},
		{buffer, length},
		{&aligned_data, packetlen_aligned - packetlen}
	};
	int writeCount = ((length != 0) ? fullSize : headerSize);

	m_outbound.Write(buffers, writeCount);
}

void* HyperVChannel::ReadPacket(const uint32_t length)
{
	return m_inbound.Read(length);
}

void HyperVChannel::NextPacket(const uint32_t length)
{
	//Finally the new indexes (although we don't use them, see VMBUS_PKT_TRAILER
	m_inbound.Increment(length + VMBUS_PKT_TRAILER);
}

void HyperVChannel::SetEvent()
{
	//Assert(!m_channel->monitor_allocated);
	Assert(m_channel->is_dedicated_interrupt);

	HyperV::HV_CONNECTION_ID id = { 0 };
	id.Id = m_channel->connection_id;
	HyperV::HV_HYPERCALL_RESULT_VALUE result = HyperV::Platform::HvSignalEvent(id, 0);
	Assert(HV_SUCCESS(result.Status));
}

bool HyperVChannel::StopRead()
{
	return m_inbound.CommitRead();
}

void HyperVChannel::Display()
{
	Printf("Inbound:\n");
	m_inbound.Display();
	Printf("Outbound:\n");
	m_outbound.Display();
}
