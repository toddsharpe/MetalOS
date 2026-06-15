#pragma once

#include "HyperVNic.h"
#include "kernel/KDevice.h"
#include "kernel/Drivers/VmBusDriver.h"
#include "kernel/HyperV/VmBus.h"
#include "Lib/Arena.h"
#include "Assert.h"
#include "kernel/Api.h"

HyperVNic::HyperVNic(KDevice& device) :
	Driver(device),
	m_channel(NIC_SEND_RING_BUFFER_SIZE, NIC_RECV_RING_BUFFER_SIZE, { &HyperVNic::Callback, this }),
	m_event(false, false),
	m_macAddress{},
	m_netIf(nullptr),
	m_rxQueue(),
	m_sendBuf(nullptr),
	m_sendSectionSize(0),
	m_recvBuf(nullptr),
	m_recvBufGpadl(0),
	m_recvSectionCount(0),
	m_recvSectionSize(0),
	m_recvSections(),
	m_response()
{
}

Result HyperVNic::Initialize(Arena& arena)
{
	m_channel.Initialize(&m_device.m_channel);
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
	// negotiated_protocol_ver is reserved/unused for V1/V2 — always 0xffffffff.
	// Track the version we sent instead, same as Linux netvsc.c.
	const uint32_t negotiatedVer = msg.msg.init.max_protocol_ver;

	// Step 2: report NDIS version matching negotiated NVSP version (no response)
	msg = {};
	msg.header.msg_type = NvspMessageType::SendNdisVersion;
	msg.msg.send_ndis_version.ndis_major_ver = 6;
	msg.msg.send_ndis_version.ndis_minor_ver = (negotiatedVer >= NVSP_PROTOCOL_VERSION_61) ? 61 :
	                                            (negotiatedVer >= NVSP_PROTOCOL_VERSION_6)  ? 60 :
	                                            (negotiatedVer >= NVSP_PROTOCOL_VERSION_5)  ? 50 : 30;
	SendNvsp(msg);

	// Step 3: allocate and register receive buffer
	const paddr_t recvBufAddr = KePhysicalAlloc(NIC_RECV_BUFFER_PAGES);
	m_recvBuf = (uint8_t*)MapPages(recvBufAddr, NIC_RECV_BUFFER_PAGES, MapType::Driver);
	Assert(m_recvBuf);
	memset(m_recvBuf, 0, NIC_RECV_BUFFER_PAGES * PageSize);

	m_recvBufGpadl = CreateGpadl(recvBufAddr, NIC_RECV_BUFFER_PAGES);

	msg = {};
	msg.header.msg_type = NvspMessageType::SendReceiveBuffer;
	msg.msg.send_recv_buf.gpadl_handle = m_recvBufGpadl;
	msg.msg.send_recv_buf.id = NIC_RECV_BUFFER_ID;
	SendNvsp(msg, true);
	KeWait(m_event);

	Assert(m_response.header.msg_type == NvspMessageType::SendReceiveBufferComplete);
	m_recvSectionSize = (m_recvSectionCount > 0) ? m_recvSections[0].sub_alloc_size : 0;

	// Step 4: register send buffer — host requires this before activating the RNDIS layer
	{
		const paddr_t sendBufAddr = KePhysicalAlloc(NIC_SEND_BUFFER_PAGES);
		m_sendBuf = (uint8_t*)MapPages(sendBufAddr, NIC_SEND_BUFFER_PAGES, MapType::Driver);
		Assert(m_sendBuf);
		memset(m_sendBuf, 0, NIC_SEND_BUFFER_PAGES * PageSize);

		const uint32_t sendBufGpadl = CreateGpadl(sendBufAddr, NIC_SEND_BUFFER_PAGES);

		msg = {};
		msg.header.msg_type = NvspMessageType::SendSendBuffer;
		msg.msg.send_send_buf.gpadl_handle = sendBufGpadl;
		msg.msg.send_send_buf.id = NIC_SEND_BUFFER_ID;
		SendNvsp(msg, true);
		KeWait(m_event);

		Assert(m_response.header.msg_type == NvspMessageType::SendSendBufferComplete);
		m_sendSectionSize = m_response.msg.send_send_buf_complete.section_size;
	}

	// Step 4b: SendNdisConfig — required for NVSP V5+; not sent for V2
	if (negotiatedVer >= NVSP_PROTOCOL_VERSION_5)
	{
		nvsp_message cfg = {};
		cfg.header.msg_type = NvspMessageType::SendNdisConfig;
		cfg.msg.send_ndis_config.mtu = 1514;
		cfg.msg.send_ndis_config.reserved = 0;
		cfg.msg.send_ndis_config.capability_flags = 0;
		SendNvsp(cfg, false);
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

		Printf("HyperVNic: MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
			m_macAddress.bytes[0], m_macAddress.bytes[1], m_macAddress.bytes[2],
			m_macAddress.bytes[3], m_macAddress.bytes[4], m_macAddress.bytes[5]);
	}

	const Net::NetDriverOps ops = { this, m_macAddress, &HyperVNic::Send_, &HyperVNic::Receive_, &HyperVNic::IsLinkUp_ };
	m_netIf = arena.Allocate<Net::NetIf>(ops, Net::l2_t::Ethernet);
	m_netIf->Init();
	Net::AddNetIf(*m_netIf);

	return Result::Success;
}

Result HyperVNic::Enumerate(Arena& arena)
{
	return Result::Success;
}

void HyperVNic::Receive(Net::NetIf& net_if)
{
	StaticBuffer<Net::buffer_size> buf;
	size_t bytes_read;
	while (m_rxQueue.Pop(buf, bytes_read))
	{
		Net::Packet packet(Buffer(buf.Data, bytes_read), bytes_read);
		Net::get_l2(net_if.l2).Receive(net_if, packet);
	}
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

void HyperVNic::SendRndisPacket(uint32_t channelType, const void* rndisData, uint32_t rndisLen, bool requestCompletion)
{
	Assert(rndisLen <= m_sendSectionSize);
	memcpy(m_sendBuf, rndisData, rndisLen);

	// Full nvsp_message (28 bytes) required — host validates inline packet length and rejects truncated sends.
	nvsp_message msg = {};
	msg.header.msg_type = NvspMessageType::SendRndisPacket;
	msg.msg.send_rndis_pkt.channel_type = channelType;
	msg.msg.send_rndis_pkt.send_buf_section_index = 0;
	msg.msg.send_rndis_pkt.send_buf_section_size = rndisLen;

	const uint32_t flags = requestCompletion ? VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED : 0;
	m_channel.SendPacket(&msg, sizeof(nvsp_message), 0, HyperV::VM_PKT_DATA_INBAND, flags);
}

uint32_t HyperVNic::CreateGpadl(paddr_t address, size_t pageCount)
{
	KDevice* bus = KeGetDevice("\\_SB_.VMOD.VMBS");
	Assert(bus);
	VmBusDriver* vmbus = static_cast<VmBusDriver*>(bus->Driver);

	static constexpr size_t MaxHeaderPfns = 26;
	static constexpr size_t MaxBodyPfns = 28;

	const size_t headerPfns = pageCount < MaxHeaderPfns ? pageCount : MaxHeaderPfns;
	const size_t headerMsgSize = sizeof(HyperV::vmbus_channel_gpadl_header)
		+ sizeof(HyperV::gpa_range)
		+ (headerPfns - ANYSIZE_ARRAY) * sizeof(uint64_t);

	StaticBuffer<256> headerBuf;
	HyperV::vmbus_channel_gpadl_header* header =
		(HyperV::vmbus_channel_gpadl_header*)headerBuf.Data;
	memset(header, 0, headerMsgSize);
	header->header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_HEADER;
	header->child_relid = m_device.child_relid;
	header->rangecount = 1;
	header->range_buflen = (uint16_t)(sizeof(HyperV::gpa_range) + (pageCount - ANYSIZE_ARRAY) * sizeof(uint64_t));
	header->range[0].byte_count = (uint32_t)(pageCount << PageShift);
	header->range[0].byte_offset = 0;
	for (size_t i = 0; i < headerPfns; i++)
		header->range[0].pfn_array[i] = (address >> PageShift) + i;

	KEvent event(false, false);
	HyperV::VmBusResponse response;
	const uint32_t gpadl = vmbus->BeginGpadl((uint32_t)headerMsgSize, header, response, event);

	size_t pfnsSent = headerPfns;
	uint32_t bodyNum = 0;
	while (pfnsSent < pageCount)
	{
		const size_t batchSize = (pageCount - pfnsSent) < MaxBodyPfns ? (pageCount - pfnsSent) : MaxBodyPfns;
		const size_t bodyMsgSize = sizeof(HyperV::vmbus_channel_gpadl_body)
			+ (batchSize - ANYSIZE_ARRAY) * sizeof(uint64_t);

		StaticBuffer<256> bodyBuf;
		HyperV::vmbus_channel_gpadl_body* body =
			(HyperV::vmbus_channel_gpadl_body*)bodyBuf.Data;
		memset(body, 0, bodyMsgSize);
		body->header.msgtype = HyperV::vmbus_channel_message_type::CHANNELMSG_GPADL_BODY;
		body->msgnumber = bodyNum++;
		body->gpadl = gpadl;
		for (size_t i = 0; i < batchSize; i++)
			body->pfn[i] = (address >> PageShift) + pfnsSent + i;

		vmbus->PostGpadlBody((uint32_t)bodyMsgSize, body);
		pfnsSent += batchSize;
	}

	KeWait(event);
	Assert(response.gpadl_created.creation_status == 0);
	return response.gpadl_created.gpadl;
}

void HyperVNic::OnCallback()
{
	HyperV::vmpacket_descriptor* packet;
	const uint32_t length = sizeof(HyperV::vmpacket_descriptor);
	do
	{
		while ((packet = (HyperV::vmpacket_descriptor*)m_channel.ReadPacket(length)) != nullptr)
		{
			switch (packet->type)
			{
			case HyperV::VM_PKT_COMP:
			{
				nvsp_message* nvsp = (nvsp_message*)((uintptr_t)packet + (packet->offset8 << 3));
				const NvspMessageType type = nvsp->header.msg_type;

				if (type == NvspMessageType::InitComplete ||
					type == NvspMessageType::SendReceiveBufferComplete ||
					type == NvspMessageType::SendSendBufferComplete ||
					type == NvspMessageType::SendRndisPacketComplete)
				{
					// SendReceiveBufferComplete has a variable sections[] array;
					// read directly from ring buffer memory before the fixed memcpy.
					if (type == NvspMessageType::SendReceiveBufferComplete)
					{
						nvsp_send_recv_buf_complete_message* complete = &nvsp->msg.send_recv_buf_complete;
						m_recvSectionCount = complete->num_sections;
						if (m_recvSectionCount > ArraySize(m_recvSections))
							m_recvSectionCount = ArraySize(m_recvSections);
						for (uint32_t i = 0; i < m_recvSectionCount; i++)
							m_recvSections[i] = ((nvsp_recv_buf_section*)complete->sections)[i];
					}

					memcpy(&m_response, nvsp, sizeof(nvsp_message));
					m_event.Set();
				}
			}
			break;

			case HyperV::VM_PKT_DATA_USING_XFER_PAGES:
			{
				const uint64_t transId = packet->trans_id;
				vmtransfer_page_packet_header* xfer =
					(vmtransfer_page_packet_header*)((uintptr_t)packet + sizeof(HyperV::vmpacket_descriptor));

				nvsp_message* nvsp = (nvsp_message*)((uintptr_t)packet + (packet->offset8 << 3));

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
						memcpy(m_macAddress.bytes, info, copyLen);
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

						//Send up net stack
						Buffer backing(frame, frameLen);
						Net::Packet packet({ frame, frameLen }, frameLen);
						Net::Ethernet::Receive(*m_netIf, packet);
						//m_rxQueue.Push(frame, frameLen);
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

			m_channel.NextPacket(packet->len8 << 3);
		}
	} while (m_channel.StopRead());
}

Result HyperVNic::SendFrame(const void* data, size_t length)
{
	// Build inline send: [nvsp_rndis_prefix][rndis_message_header][rndis_packet][Ethernet frame]
	// send_buf_section_index=0xFFFFFFFF tells the host to read data inline from the ring buffer.
	const uint32_t rndisLen = (uint32_t)(sizeof(rndis_message_header) + sizeof(rndis_packet) + length);
	const uint32_t totalLen = (uint32_t)(sizeof(nvsp_rndis_prefix) + rndisLen);

	StaticBuffer<2048> txArena;
	uint8_t* buf = txArena.Data;
	memset(buf, 0, totalLen);

	nvsp_rndis_prefix* prefix = (nvsp_rndis_prefix*)buf;
	prefix->msg_type = NvspMessageType::SendRndisPacket;
	prefix->channel_type = NVSP_CHANNEL_TYPE_DATA;
	prefix->send_buf_section_index = NVSP_INVALID_SECTION_INDEX;
	prefix->send_buf_section_size = 0;

	rndis_message_header* rndisHdr = (rndis_message_header*)(prefix + 1);
	rndisHdr->ndis_msg_type = RNDIS_MSG_PACKET;
	rndisHdr->msg_len = rndisLen;

	rndis_packet* pkt = (rndis_packet*)(rndisHdr + 1);
	pkt->data_offset = sizeof(rndis_packet);
	pkt->data_length = (uint32_t)length;

	memcpy((uint8_t*)(pkt + 1), data, length);

	m_channel.SendPacket(buf, totalLen, 0,
		HyperV::VM_PKT_DATA_INBAND, VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	return Result::Success;
}
