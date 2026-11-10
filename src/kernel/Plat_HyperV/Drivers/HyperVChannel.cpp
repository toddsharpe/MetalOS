#include "HyperVChannel.h"
#include <cstddef>
#include "kernel/Arch.h"
#include "kernel/Plat_HyperV/Types.h"
#include "kernel/Plat_HyperV/HyperV.h"
#include "Lib/Span.h"
#include "kernel/Plat_HyperV/Drivers/HyperVRingBuffer.h"
#include "Assert.h"
#include "kernel/Pmem.h"

HyperVChannel::HyperVChannel(const uint32_t sendSize, const uint32_t receiveSize, const ActionContext callback) :
	m_callback(callback),
	m_outbound(sendSize, *this),//upstream
	m_inbound(receiveSize, *this),//downstream
	m_gpadlHandle(),
	m_channel(),
	m_address(),
	m_pageCount()
{
	
}

void HyperVChannel::Initialize(VmBusDriver& vmbus, const HyperV::vmbus_channel_offer_channel* offerChannel)
{
	m_channel = offerChannel;

	Assert((m_outbound.Size % Arch::PageSize) == 0);
	Assert((m_inbound.Size % Arch::PageSize) == 0);

	//Number of pages
	const size_t sendCount = Arch::SizeToPages(m_outbound.Size);
	const size_t receiveCount = Arch::SizeToPages(m_inbound.Size);
	const size_t pageCount = sendCount + receiveCount;

	//Get physical contiguous region
	const paddr_t address = KePhysicalAlloc(pageCount);
	m_address = address;
	m_pageCount = pageCount;

	//Initialize rings (outbound first)
	m_outbound.Initialize(address);
	m_inbound.Initialize(address + m_outbound.Size);
	
	//Establish GPADL for ring buffers
	m_gpadlHandle = vmbus.CreateGpadl(m_channel->child_relid, address, pageCount);

	//Register the channel callback BEFORE opening: the host can signal the channel the instant it
	//is open, and if no handler is registered yet that notification is dropped (and the host may
	//rescind the channel). The rings are already initialized above, so the callback is safe to run.
	vmbus.SetCallback(m_channel->child_relid, m_callback);

	//Open channel
	HyperV::vmbus_channel_open_channel openChannel;
	memset(&openChannel, 0, sizeof(HyperV::vmbus_channel_open_channel));
	openChannel.header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_OPENCHANNEL;
	openChannel.openid = m_channel->child_relid;
	openChannel.child_relid = m_channel->child_relid;
	openChannel.ringbuffer_gpadlhandle = m_gpadlHandle;
	openChannel.downstream_ringbuffer_pageoffset = (uint32_t)sendCount;
	HyperV::VmBusResponse response;
	vmbus.OpenChannel(openChannel, response);
}

void HyperVChannel::Close(VmBusDriver& vmbus)
{
	//Stop the callback first so nothing reads the rings while they go away
	vmbus.RemoveCallback(m_channel->child_relid);
	if (!vmbus.CloseChannel(m_channel->child_relid))
		Printf("HyperVChannel: CLOSECHANNEL for child_relid=%u wasn't posted\n", m_channel->child_relid);

	//The host owns the rings until it tears the GPADL down, so on a timeout they're left allocated for good
	if (!vmbus.TeardownGpadl(m_channel->child_relid, m_gpadlHandle, 2000))
	{
		Printf("HyperVChannel: GPADL %u for child_relid=%u wasn't torn down; leaking its %u pages\n", m_gpadlHandle, m_channel->child_relid, (uint32_t)m_pageCount);
		return;
	}

	m_outbound.Unmap();
	m_inbound.Unmap();
	KePhysicalFree(m_address, m_pageCount);
	m_gpadlHandle = 0;
}

//Format is vmpacket_descriptor followed by buffer, alignment if needed, and then old indexes
bool HyperVChannel::SendPacket(const void* buffer, const size_t length, const uint64_t requestId, const HyperV::vmbus_packet_type type, const uint16_t flags)
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

	static constexpr size_t headerSize = 1;
	static constexpr size_t fullSize = 3;

	const cbuffer_t buffers[] = {
		Bytes(&desc, sizeof(HyperV::vmpacket_descriptor)),
		Bytes(buffer, length),
		Bytes(&aligned_data, packetlen_aligned - packetlen)
	};
	const size_t writeCount = ((length != 0) ? fullSize : headerSize);

	return m_outbound.Write({ buffers, writeCount });
}

//Format is vmpacket_descriptor, the range count and one gpa_range with its page numbers, then buffer, alignment, and the old indexes
bool HyperVChannel::SendPacketGpaDirect(const void* buffer, const size_t length, const uint64_t requestId, const paddr_t address, const uint32_t byteCount)
{
	const uint32_t offset = static_cast<uint32_t>(address & (Arch::PageSize - 1));
	const size_t pages = Arch::SizeToPages(offset + byteCount);
	Assert(pages != 0 && pages <= GpaDirectMaxPages);

#pragma pack(push, 1)
	struct
	{
		HyperV::vmpacket_descriptor desc;
		uint32_t reserved;
		uint32_t rangecount;
		uint32_t byte_count;
		uint32_t byte_offset;
		uint64_t pfn_array[GpaDirectMaxPages];
	} header = {};
#pragma pack(pop)

	const uint32_t headerSize = static_cast<uint32_t>(offsetof(decltype(header), pfn_array) + pages * sizeof(uint64_t));
	const uint32_t packetlen = headerSize + static_cast<uint32_t>(length);
	const uint32_t packetlen_aligned = ByteAlign(packetlen, sizeof(uint64_t));

	header.desc.type = HyperV::VM_PKT_DATA_USING_GPA_DIRECT;
	header.desc.flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	header.desc.offset8 = static_cast<uint16_t>(headerSize >> 3);
	header.desc.len8 = static_cast<uint16_t>(packetlen_aligned >> 3);
	header.desc.trans_id = requestId;
	header.rangecount = 1;
	header.byte_count = byteCount;
	header.byte_offset = offset;
	for (size_t i = 0; i < pages; i++)
		header.pfn_array[i] = (address >> Arch::PageShift) + i;

	uint64_t aligned_data = 0;
	const cbuffer_t buffers[] = {
		Bytes(&header, headerSize),
		Bytes(buffer, length),
		Bytes(&aligned_data, packetlen_aligned - packetlen)
	};

	return m_outbound.Write({ buffers, 3 });
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
