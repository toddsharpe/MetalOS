#include "HyperVNic.h"
#include "kernel/KDevice.h"
#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/HyperV/VmBus.h"
#include "Assert.h"
#include "kernel/Api.h"

HyperVNic::HyperVNic(KDevice& device) :
	NicDriver(device),
	m_channel(NIC_SEND_RING_BUFFER_SIZE, NIC_RECV_RING_BUFFER_SIZE, { &HyperVNic::Callback, this }),
	m_event(false, false),
	m_macAddress{},
	m_sendBuf(nullptr),
	m_sendSectionSize(0),
	m_sendSectionCount(0),
	m_sendSectionsInUse(0),
	m_recvBuf(nullptr),
	m_response()
{
}

Result HyperVNic::Initialize()
{
	m_channel.Initialize(*m_device.VmBus, &m_device.m_channel);
	m_device.Class = KDeviceClass::Nic;

	// Step 1: negotiate NVSP protocol version
	nvsp_message msg = {};
	msg.header.msg_type = NvspMessageType::Init;
	msg.msg.init.min_protocol_ver = NVSP_PROTOCOL_VERSION_2;
	msg.msg.init.max_protocol_ver = NVSP_PROTOCOL_VERSION_2;
	SendNvsp(msg, true);
	KeWait(m_event);

	Assert(m_response.header.msg_type == NvspMessageType::InitComplete);
	Assert(m_response.msg.init_complete.status == NVSP_STATUS_SUCCESS);

	// Step 2: report NDIS version 6.30 (no response)
	msg = {};
	msg.header.msg_type = NvspMessageType::SendNdisVersion;
	msg.msg.send_ndis_version.ndis_major_ver = 6;
	msg.msg.send_ndis_version.ndis_minor_ver = 30;
	SendNvsp(msg);

	// Step 3: allocate and register receive buffer
	const paddr_t recvBufAddr = KePhysicalAlloc(NIC_RECV_BUFFER_PAGES);
	m_recvBuf = (uint8_t*)KeVirtualMap(recvBufAddr, NIC_RECV_BUFFER_PAGES * Arch::PageSize);
	Assert(m_recvBuf);
	memset(m_recvBuf, 0, NIC_RECV_BUFFER_PAGES * Arch::PageSize);

	const uint32_t recvBufGpadl = m_device.VmBus->CreateGpadl(m_device.child_relid, recvBufAddr, NIC_RECV_BUFFER_PAGES);

	msg = {};
	msg.header.msg_type = NvspMessageType::SendReceiveBuffer;
	msg.msg.send_recv_buf.gpadl_handle = recvBufGpadl;
	msg.msg.send_recv_buf.id = NIC_RECV_BUFFER_ID;
	SendNvsp(msg, true);
	KeWait(m_event);

	Assert(m_response.header.msg_type == NvspMessageType::SendReceiveBufferComplete);

	// Step 4: register send buffer — host requires this before activating the RNDIS layer
	{
		const paddr_t sendBufAddr = KePhysicalAlloc(NIC_SEND_BUFFER_PAGES);
		m_sendBuf = (uint8_t*)KeVirtualMap(sendBufAddr, NIC_SEND_BUFFER_PAGES * Arch::PageSize);
		Assert(m_sendBuf);
		memset(m_sendBuf, 0, NIC_SEND_BUFFER_PAGES * Arch::PageSize);

		const uint32_t sendBufGpadl = m_device.VmBus->CreateGpadl(m_device.child_relid, sendBufAddr, NIC_SEND_BUFFER_PAGES);

		msg = {};
		msg.header.msg_type = NvspMessageType::SendSendBuffer;
		msg.msg.send_send_buf.gpadl_handle = sendBufGpadl;
		msg.msg.send_send_buf.id = NIC_SEND_BUFFER_ID;
		SendNvsp(msg, true);
		KeWait(m_event);

		Assert(m_response.header.msg_type == NvspMessageType::SendSendBufferComplete);
		m_sendSectionSize = m_response.msg.send_send_buf_complete.section_size;
		Assert(m_sendSectionSize);

		const size_t sections = (NIC_SEND_BUFFER_PAGES * Arch::PageSize) / m_sendSectionSize;
		m_sendSectionCount = (sections < MaxSendSections) ? (uint32_t)sections : MaxSendSections;
	}

	// Step 5: initialize RNDIS
	{
		const uint32_t rndisLen = (uint32_t)(sizeof(rndis_message_header) + sizeof(rndis_initialize_request));

		StaticBuffer<64> initArena;
		uint8_t* rndis = initArena.Data;
		memset(rndis, 0, rndisLen);

		rndis_message_header* rndisHdr = (rndis_message_header*)rndis;
		rndisHdr->ndis_msg_type = RNDIS_MSG_INIT;
		rndisHdr->msg_len = rndisLen;

		rndis_initialize_request* rndisInit = (rndis_initialize_request*)(rndisHdr + 1);
		rndisInit->req_id = 1;
		rndisInit->major_version = 1;
		rndisInit->minor_version = 0;
		rndisInit->max_transfer_size = 0x4000;

		SendRndisPacket(NVSP_CHANNEL_TYPE_CONTROL, rndis, rndisLen, true);
		KeWait(m_event);
	}

	// Step 6: set packet filter — host delivers no frames until this OID is set
	{
		const uint32_t filterValue = RNDIS_PACKET_TYPE_DIRECTED |
			RNDIS_PACKET_TYPE_ALL_MULTICAST | RNDIS_PACKET_TYPE_BROADCAST;
		const uint32_t rndisLen = (uint32_t)(sizeof(rndis_message_header) + sizeof(rndis_set_request) + sizeof(uint32_t));

		StaticBuffer<64> setArena;
		uint8_t* rndis = setArena.Data;
		memset(rndis, 0, rndisLen);

		rndis_message_header* rndisHdr = (rndis_message_header*)rndis;
		rndisHdr->ndis_msg_type = RNDIS_MSG_SET;
		rndisHdr->msg_len = rndisLen;

		rndis_set_request* setReq = (rndis_set_request*)(rndisHdr + 1);
		setReq->req_id = 2;
		setReq->oid = OID_GEN_CURRENT_PACKET_FILTER;
		setReq->info_buflen = sizeof(uint32_t);
		setReq->info_buf_offset = sizeof(rndis_set_request);
		setReq->device_vc_handle = 0;
		*(uint32_t*)(setReq + 1) = filterValue;

		SendRndisPacket(NVSP_CHANNEL_TYPE_CONTROL, rndis, rndisLen, true);
		KeWait(m_event);
	}

	// Step 7: query MAC address via RNDIS OID.
	// Response arrives as XFER_PAGES, so no COMPLETION_REQUESTED — OnCallback signals m_event on RNDIS_MSG_QUERY_C.
	{
		const uint32_t rndisLen = (uint32_t)(sizeof(rndis_message_header) + sizeof(rndis_query_request));

		StaticBuffer<64> queryArena;
		uint8_t* rndis = queryArena.Data;
		memset(rndis, 0, rndisLen);

		rndis_message_header* rndisHdr = (rndis_message_header*)rndis;
		rndisHdr->ndis_msg_type = RNDIS_MSG_QUERY;
		rndisHdr->msg_len = rndisLen;

		rndis_query_request* queryReq = (rndis_query_request*)(rndisHdr + 1);
		queryReq->req_id = 3;
		queryReq->oid = OID_802_3_PERMANENT_ADDRESS;
		queryReq->info_buflen = 0;
		queryReq->info_buf_offset = sizeof(rndis_query_request);
		queryReq->device_vc_handle = 0;

		SendRndisPacket(NVSP_CHANNEL_TYPE_CONTROL, rndis, rndisLen, false);
		KeWait(m_event);

		//Stored little-endian; print in canonical (wire) order.
		Printf("HyperVNic: MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
			m_macAddress.bytes[5], m_macAddress.bytes[4], m_macAddress.bytes[3],
			m_macAddress.bytes[2], m_macAddress.bytes[1], m_macAddress.bytes[0]);
	}

	//The NIC is now discoverable via the device tree (KDeviceClass::Nic). The kernel
	//net device (Kernel_net.cpp) finds it there to transmit; received frames go up
	//via KeNetRxFrame (called from OnCallback) to the netstack RX ring.

	return Result::Success;
}

Result HyperVNic::Enumerate()
{
	return Result::Success;
}

void HyperVNic::SendNvsp(const nvsp_message& msg, bool requestCompletion)
{
	const uint32_t flags = requestCompletion ? VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED : 0;
	m_channel.SendPacket(&msg, sizeof(nvsp_message), (uint64_t)&msg,
		HyperV::VM_PKT_DATA_INBAND, flags);
}

void HyperVNic::SendNvspCompletion(const nvsp_message& msg, uint64_t transId)
{
	m_channel.SendPacket(&msg, sizeof(nvsp_message), transId,
		HyperV::VM_PKT_COMP, 0);
}

bool HyperVNic::SendRndisPacket(uint32_t channelType, const void* rndisData, uint32_t rndisLen, bool requestCompletion, uint32_t section)
{
	Assert(rndisLen <= m_sendSectionSize);
	Assert(section < m_sendSectionCount);
	memcpy(m_sendBuf + (size_t)section * m_sendSectionSize, rndisData, rndisLen);

	// Full nvsp_message (28 bytes) required — host validates inline packet length and rejects truncated sends.
	nvsp_message msg = {};
	msg.header.msg_type = NvspMessageType::SendRndisPacket;
	msg.msg.send_rndis_pkt.channel_type = channelType;
	msg.msg.send_rndis_pkt.send_buf_section_index = section;
	msg.msg.send_rndis_pkt.send_buf_section_size = rndisLen;

	//trans_id names the section so its SendRndisPacketComplete can release it (control messages use 0)
	const uint64_t transId = (section == 0) ? 0 : (SendSectionTag | section);
	const uint32_t flags = requestCompletion ? VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED : 0;
	return m_channel.SendPacket(&msg, sizeof(nvsp_message), transId, HyperV::VM_PKT_DATA_INBAND, flags);
}

//Returns a free data section (never 0), or 0 if all are still owned by the host
uint32_t HyperVNic::AcquireSendSection()
{
	const cpu_flags_t flags = ArchDisableInterrupts();
	uint32_t section = 0;
	for (uint32_t i = 1; i < m_sendSectionCount; i++)
	{
		const uint64_t bit = 1ULL << i;
		if ((m_sendSectionsInUse & bit) == 0)
		{
			m_sendSectionsInUse |= bit;
			section = i;
			break;
		}
	}
	ArchRestoreFlags(flags);
	return section;
}

//Called when the host completes a data frame, or when the frame never reached the ring
void HyperVNic::ReleaseSendSection(const uint64_t transId)
{
	if ((transId & SendSectionTagMask) != SendSectionTag)
		return;

	const uint64_t section = transId & ~SendSectionTagMask;
	const cpu_flags_t flags = ArchDisableInterrupts();
	if (section != 0 && section < m_sendSectionCount)
		m_sendSectionsInUse &= ~(1ULL << section);
	ArchRestoreFlags(flags);
}

void HyperVNic::OnCallback()
{
	m_channel.Drain([this](const HyperV::vmpacket_descriptor& packet, uint8_t* const payload, const uint32_t)
	{
		switch (packet.type)
		{
		case HyperV::VM_PKT_COMP:
		{
			nvsp_message* nvsp = (nvsp_message*)payload;
			const NvspMessageType type = nvsp->header.msg_type;

			if (type == NvspMessageType::InitComplete ||
				type == NvspMessageType::SendReceiveBufferComplete ||
				type == NvspMessageType::SendSendBufferComplete ||
				type == NvspMessageType::SendRndisPacketComplete)
			{
				//A data frame's send section belongs to the host until this completion
				if (type == NvspMessageType::SendRndisPacketComplete)
					ReleaseSendSection(packet.trans_id);

				memcpy(&m_response, nvsp, sizeof(nvsp_message));
				m_event.Set();
			}
		}
		break;

		case HyperV::VM_PKT_DATA_USING_XFER_PAGES:
		{
			const uint64_t transId = packet.trans_id;
			vmtransfer_page_packet_header* xfer =
				(vmtransfer_page_packet_header*)((uintptr_t)&packet + sizeof(HyperV::vmpacket_descriptor));

			nvsp_message* nvsp = (nvsp_message*)payload;

			if (nvsp->header.msg_type != NvspMessageType::SendRndisPacket)
				break;

			if (nvsp->msg.send_rndis_pkt.channel_type == NVSP_CHANNEL_TYPE_CONTROL)
			{
				const uint32_t offset = xfer->ranges[0].byte_offset;
				rndis_message_header* hdr = (rndis_message_header*)(m_recvBuf + offset);

				if (hdr->ndis_msg_type == RNDIS_MSG_QUERY_C)
				{
					rndis_query_complete* qc = (rndis_query_complete*)(hdr + 1);
					const uint8_t* info = (const uint8_t*)qc + qc->info_buf_offset;
					const uint32_t copyLen = qc->info_buflen < sizeof(m_macAddress) ? qc->info_buflen : sizeof(m_macAddress);
					//RNDIS returns the MAC in canonical (wire) order; store it little-endian to match the kernel convention
					for (uint32_t i = 0; i < copyLen; i++)
						m_macAddress.bytes[copyLen - 1 - i] = info[i];
					m_event.Set();
				}
			}
			else if (nvsp->msg.send_rndis_pkt.channel_type == NVSP_CHANNEL_TYPE_DATA)
			{
				for (uint32_t r = 0; r < xfer->range_cnt; r++)
				{
					const uint32_t offset = xfer->ranges[r].byte_offset;
					rndis_message_header* hdr = (rndis_message_header*)(m_recvBuf + offset);
					if (hdr->ndis_msg_type != RNDIS_MSG_PACKET)
						continue;

					rndis_packet* pkt = (rndis_packet*)(hdr + 1);
					uint8_t* const frame = (uint8_t*)pkt + pkt->data_offset;
					const uint32_t frameLen = pkt->data_length;

					//Hand the raw ethernet frame to the kernel net device (interrupt context); the usermode netstack drains it from the RX ring
					KeNetRxFrame(this, frame, frameLen);
				}
			}

			// Return receive buffer sections to host
			nvsp_message complete = {};
			complete.header.msg_type = NvspMessageType::SendRndisPacketComplete;
			complete.msg.send_rndis_pkt_complete.status = NVSP_STATUS_SUCCESS;
			SendNvspCompletion(complete, transId);
		}
		break;

		default:
			break;
		}
	});
}

Result HyperVNic::SendFrame(const void* data, size_t length)
{
	// Build the RNDIS_PACKET message: [rndis_message_header][rndis_packet][Ethernet frame].
	const uint32_t rndisLen = (uint32_t)(sizeof(rndis_message_header) + sizeof(rndis_packet) + length);

	StaticBuffer<2048> rndisArena;
	uint8_t* buf = rndisArena.Data;
	memset(buf, 0, rndisLen);

	rndis_message_header* rndisHdr = (rndis_message_header*)buf;
	rndisHdr->ndis_msg_type = RNDIS_MSG_PACKET;
	rndisHdr->msg_len = rndisLen;

	rndis_packet* pkt = (rndis_packet*)(rndisHdr + 1);
	pkt->data_offset = sizeof(rndis_packet);
	pkt->data_length = (uint32_t)length;

	memcpy((uint8_t*)(pkt + 1), data, length);

	//Own section per in-flight frame so back-to-back sends can't overwrite one the host hasn't read; drop if none free
	const uint32_t section = AcquireSendSection();
	if (section == 0)
		return Result::Failed;

	// Transmit over the data channel through the send-buffer section, the same path as control RNDIS messages
	if (!SendRndisPacket(NVSP_CHANNEL_TYPE_DATA, buf, rndisLen, true, section))
	{
		//Ring full: no completion will come, so the section is still ours
		ReleaseSendSection(SendSectionTag | section);
		return Result::Failed;
	}
	return Result::Success;
}

void HyperVNic::GetMac(uint8_t out[6]) const
{
	memcpy(out, m_macAddress.bytes, sizeof(m_macAddress.bytes));
}
