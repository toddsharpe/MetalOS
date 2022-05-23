#include "HyperVChannel.h"

#include <Assert.h>
#include <HyperV.h>

HyperVChannel::HyperVChannel(size_t sendSize, size_t receiveSize, CallContext callback) :
	m_sendCount(SIZE_TO_PAGES(sendSize)),
	m_receiveCount(SIZE_TO_PAGES(receiveSize)),
	m_address(kernel.AllocatePhysical(m_sendCount + m_receiveCount)),
	m_callback(callback),
	m_outbound(m_address, m_sendCount, *this),//upstream
	m_inbound(m_address + (m_sendCount << PAGE_SHIFT), m_receiveCount, *this),//downstream
	m_gpadlHandle(),
	m_channel(),
	m_vmbus()
{
	Assert((sendSize % PAGE_SIZE) == 0);
	Assert((receiveSize % PAGE_SIZE) == 0);
	Assert((m_address & PAGE_MASK) == 0);
	kernel.Printf("Send: 0x%x Receive: 0x%x\n", sendSize, receiveSize);

	Device* bus = kernel.KeGetDevice("\\_SB_\\VMOD\\VMBS");
	Assert(bus);
	m_vmbus = static_cast<VmBusDriver*>(bus->GetDriver());
}

void HyperVChannel::Initialize(vmbus_channel_offer_channel* offerChannel, const ReadOnlyBuffer* buffer)
{
	if (buffer != nullptr)
		Assert(buffer->Length <= MAX_USER_DEFINED_BYTES);
	
	m_channel = offerChannel;
	
	//Establish GPADL for ring buffers
	const size_t pageCount = m_sendCount + m_receiveCount;
	const size_t pfnsize = MAX_SIZE_CHANNEL_MESSAGE - sizeof(vmbus_channel_gpadl_header) - (sizeof(gpa_range) - ANYSIZE_ARRAY * sizeof(uint64_t));
	const size_t pfncount = pfnsize / sizeof(uint64_t);

	//If this assertion fails, gpadl body message is needed
	Assert(!(pageCount > pfncount));

	//Everything fits in the header
	const size_t msgsize = sizeof(vmbus_channel_gpadl_header) + sizeof(gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t);
	vmbus_channel_gpadl_header* msg = (vmbus_channel_gpadl_header*)new uint8_t[msgsize];
	memset(msg, 0, msgsize);
	const uint16_t bodySize = (uint16_t)(sizeof(gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t));
	msg->rangecount = 1;
	msg->range_buflen = bodySize;
	msg->range[0].byte_offset = 0;
	msg->range[0].byte_count = (uint32_t)(pageCount << PAGE_SHIFT);
	for (size_t i = 0; i < pageCount; i++)
		msg->range[0].pfn_array[i] = (m_address >> PAGE_SHIFT) + i;

	msg->header.msgtype = vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER;
	msg->child_relid = m_channel->child_relid;
	kernel.Printf("Rel_ID: %d\n", m_channel->child_relid);

	VmBusResponse response;
	HV_HYPERCALL_RESULT_VALUE result = m_vmbus->PostMessage((uint32_t)msgsize, msg, response);
	m_gpadlHandle = response.gpadl_created.gpadl;

	kernel.Printf("GPADL created\n");

	//Open channel
	vmbus_channel_open_channel openChannel;
	memset(&openChannel, 0, sizeof(vmbus_channel_open_channel));
	openChannel.header.msgtype = vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL;
	openChannel.openid = m_channel->child_relid;
	openChannel.child_relid = m_channel->child_relid;
	openChannel.ringbuffer_gpadlhandle = m_gpadlHandle;
	openChannel.downstream_ringbuffer_pageoffset = (uint32_t)m_sendCount;
	if (buffer != nullptr)
		memcpy(openChannel.userdata, buffer->Data, buffer->Length);
	result = m_vmbus->PostMessage(sizeof(vmbus_channel_open_channel), &openChannel, response);

	m_vmbus->SetCallback(m_channel->child_relid, m_callback);
}

//Format is vmpacket_descriptor followed by buffer, alignment if needed, and then old indexes
void HyperVChannel::SendPacket(const void* buffer, const size_t length, const uint64_t requestId, const vmbus_packet_type type, const uint32_t flags)
{
	uint32_t packetlen = sizeof(vmpacket_descriptor) + length;
	const uint32_t packetlen_aligned = AlignSize(packetlen, sizeof(uint64_t));

	vmpacket_descriptor desc = { 0 };
	desc.type = type;
	desc.flags = flags;
	desc.offset8 = sizeof(vmpacket_descriptor) >> 3;
	desc.len8 = (uint16_t)(packetlen_aligned >> 3);
	desc.trans_id = requestId;

	u64 aligned_data = 0;

	const size_t headerSize = 1;
	const size_t fullSize = 3;

	ReadOnlyBuffer buffers[fullSize];
	buffers[0].Data = &desc;
	buffers[0].Length = sizeof(struct vmpacket_descriptor);
	buffers[1].Data = buffer;
	buffers[1].Length = length;
	buffers[2].Data = &aligned_data;
	buffers[2].Length = (packetlen_aligned - packetlen);

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

	HV_CONNECTION_ID id = { 0 };
	id.Id = m_channel->connection_id;
	HV_HYPERCALL_RESULT_VALUE result = HyperV::HvSignalEvent(id, 0);
	Assert(HV_SUCCESS(result.Status));
}

void HyperVChannel::StopRead()
{
	m_inbound.CommitRead();
}

void HyperVChannel::Display()
{
	Printf("Inbound:\n");
	m_inbound.Display();
	Printf("Outbound:\n");
	m_outbound.Display();
}
