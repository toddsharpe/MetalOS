#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Drivers/HyperVChannel.h"
#include "kernel/Objects/KEvent.h"
#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Net/PacketBuffer.h"

namespace
{

#define NIC_SEND_RING_BUFFER_SIZE    (40 * 1024)
#define NIC_RECV_RING_BUFFER_SIZE    (40 * 1024)
#define NIC_RECV_BUFFER_ID           0xcafe
#define NIC_RECV_BUFFER_PAGES        4096  // 16 MB
#define NIC_SEND_BUFFER_ID           0
#define NIC_SEND_BUFFER_PAGES        4096  // 16 MB

#define NVSP_PROTOCOL_VERSION_2      0x30002
#define NVSP_PROTOCOL_VERSION_5      0x50000
#define NVSP_PROTOCOL_VERSION_6      0x60000
#define NVSP_PROTOCOL_VERSION_61     0x60001
#define NVSP_STATUS_SUCCESS          1
#define NVSP_CHANNEL_TYPE_DATA       0
#define NVSP_INVALID_SECTION_INDEX   0xFFFFFFFF

enum class NvspMessageType : uint32_t
{
	None = 0,
	Init = 1,
	InitComplete = 2,
	SendNdisVersion = 100,
	SendReceiveBuffer = 101,
	SendReceiveBufferComplete = 102,
	RevokeReceiveBuffer = 103,
	SendSendBuffer = 104,
	SendSendBufferComplete = 105,
	RevokeSendBuffer = 106,
	SendRndisPacket = 107,
	SendRndisPacketComplete = 108,
	SendNdisConfig = 200,
};

#pragma pack(push, 1)

struct nvsp_message_header
{
	NvspMessageType msg_type;
};

struct nvsp_init_message
{
	uint32_t min_protocol_ver;
	uint32_t max_protocol_ver;
};

struct nvsp_init_complete_message
{
	uint32_t negotiated_protocol_ver;
	uint32_t max_mdl_chain_len;
	uint32_t status;
};

struct nvsp_send_ndis_version_message
{
	uint32_t ndis_major_ver;
	uint32_t ndis_minor_ver;
};

struct nvsp_send_recv_buf_message
{
	uint32_t gpadl_handle;
	uint16_t id;
};

struct nvsp_recv_buf_section
{
	uint32_t offset;
	uint32_t sub_alloc_size;
	uint32_t num_sub_allocs;
	uint32_t end_offset;
};

struct nvsp_send_recv_buf_complete_message
{
	uint32_t status;
	uint32_t num_sections;
	nvsp_recv_buf_section sections[1]; // host may return more; read via raw pointer
};

struct nvsp_send_send_buf_message
{
	uint32_t gpadl_handle;
	uint16_t id;
};

struct nvsp_send_send_buf_complete_message
{
	uint32_t status;
	uint32_t section_size;  // size of each send section; guest divides buf into sections of this size
};

struct nvsp_send_rndis_packet_message
{
	uint32_t channel_type;
	uint32_t send_buf_section_index;
	uint32_t send_buf_section_size;
};

// Minimal 16-byte NVSP prefix for inline RNDIS sends.
// nvsp_message pads to 28 bytes due to union; using that as a header
// displaces inline RNDIS data by 12 bytes and the host reads garbage.
struct nvsp_rndis_prefix
{
	NvspMessageType msg_type;        // 4
	uint32_t channel_type;           // 4
	uint32_t send_buf_section_index; // 4
	uint32_t send_buf_section_size;  // 4
};
static_assert(sizeof(nvsp_rndis_prefix) == 16, "nvsp_rndis_prefix must be exactly 16 bytes");

struct nvsp_send_rndis_packet_complete_message
{
	uint32_t status;
};

struct nvsp_send_ndis_config_message
{
	uint32_t mtu;
	uint16_t reserved;
	uint16_t capability_flags;
};

struct nvsp_message
{
	nvsp_message_header header;
	union
	{
		nvsp_init_message init;
		nvsp_init_complete_message init_complete;
		nvsp_send_ndis_version_message send_ndis_version;
		nvsp_send_recv_buf_message send_recv_buf;
		nvsp_send_recv_buf_complete_message send_recv_buf_complete;
		nvsp_send_send_buf_message send_send_buf;
		nvsp_send_send_buf_complete_message send_send_buf_complete;
		nvsp_send_rndis_packet_message send_rndis_pkt;
		nvsp_send_rndis_packet_complete_message send_rndis_pkt_complete;
		nvsp_send_ndis_config_message send_ndis_config;
	} msg;
};

// Transfer page packet header (follows vmpacket_descriptor for VM_PKT_DATA_USING_XFER_PAGES)
struct vmtransfer_page_range
{
	uint32_t byte_count;
	uint32_t byte_offset;
};

struct vmtransfer_page_packet_header
{
	uint16_t xfer_pageset_id;
	uint8_t  sender_owns_set;
	uint8_t  reserved;
	uint32_t range_cnt;
	vmtransfer_page_range ranges[1];
};

// RNDIS
#define RNDIS_MSG_INIT    0x00000002U
#define RNDIS_MSG_INIT_C  0x80000002U
#define RNDIS_MSG_QUERY   0x00000004U
#define RNDIS_MSG_QUERY_C 0x80000004U
#define RNDIS_MSG_SET     0x00000005U
#define RNDIS_MSG_SET_C   0x80000005U
#define RNDIS_MSG_PACKET  0x00000001U

#define OID_802_3_PERMANENT_ADDRESS  0x01010101U
#define OID_802_3_CURRENT_ADDRESS    0x01010102U

#define NVSP_CHANNEL_TYPE_CONTROL 1

#define OID_GEN_CURRENT_PACKET_FILTER   0x0001010EU
#define RNDIS_PACKET_TYPE_DIRECTED      0x00000001U
#define RNDIS_PACKET_TYPE_MULTICAST     0x00000002U
#define RNDIS_PACKET_TYPE_ALL_MULTICAST 0x00000004U
#define RNDIS_PACKET_TYPE_BROADCAST     0x00000008U
#define RNDIS_PACKET_TYPE_PROMISCUOUS   0x00000020U

struct rndis_message_header
{
	uint32_t ndis_msg_type;
	uint32_t msg_len;
};

struct rndis_initialize_request
{
	uint32_t req_id;
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t max_transfer_size;
};

struct rndis_set_request
{
	uint32_t req_id;
	uint32_t oid;
	uint32_t info_buflen;
	uint32_t info_buf_offset;
	uint32_t device_vc_handle;
};

struct rndis_initialize_complete
{
	uint32_t req_id;
	uint32_t status;
	uint32_t major_version;
	uint32_t minor_version;
	uint32_t device_flags;
	uint32_t medium;
	uint32_t max_packets_per_msg;
	uint32_t max_transfer_size;
	uint32_t packet_alignment_factor;
	uint32_t af_list_offset;
	uint32_t af_list_size;
};

struct rndis_query_request
{
	uint32_t req_id;
	uint32_t oid;
	uint32_t info_buflen;
	uint32_t info_buf_offset;  // offset from start of this struct to info buffer
	uint32_t device_vc_handle;
};

struct rndis_query_complete
{
	uint32_t req_id;
	uint32_t status;
	uint32_t info_buflen;
	uint32_t info_buf_offset;  // offset from start of this struct to info buffer
};

struct rndis_packet
{
	uint32_t data_offset;      // from start of rndis_packet to Ethernet frame
	uint32_t data_length;
	uint32_t oob_data_offset;
	uint32_t oob_data_length;
	uint32_t num_oob_elements;
	uint32_t per_pkt_info_offset;
	uint32_t per_pkt_info_length;
	uint32_t vc_handle;
	uint32_t reserved;
};

#pragma pack(pop)

} // namespace

class HyperVNic : public Driver, public Net::NetDriver
{
public:
	HyperVNic(KDevice& device);

	Result Initialize() override;
	Result Enumerate() override;

	// NetDriver interface
	void Receive(Net::NetIf& net_if) override;
	bool Send(Net::NetIf& net_if, Net::Packet& packet) override;
	const Net::eth_mac_t& GetMac() const override;
	bool IsLinkUp() const override;

	static void Callback(void* context) { ((HyperVNic*)context)->OnCallback(); }

private:
	void OnCallback();
	Result SendFrame(const void* data, size_t length);
	void SendNvsp(const nvsp_message& msg, bool requestCompletion = false);
	void SendNvspCompletion(const nvsp_message& msg, uint64_t transId);
	void SendRndisPacket(uint32_t channelType, const void* rndisData, uint32_t rndisLen, bool requestCompletion = false);
	uint32_t CreateGpadl(paddr_t address, size_t pageCount);

	HyperVChannel m_channel;
	KEvent m_event;

	Net::eth_mac_t m_macAddress;
	Net::NetIf* m_netIf;
	Net::PacketBuffer<8192> m_rxQueue;

	uint8_t* m_sendBuf;
	uint32_t m_sendSectionSize;

	uint8_t* m_recvBuf;
	uint32_t m_recvBufGpadl;
	uint32_t m_recvSectionCount;
	uint32_t m_recvSectionSize;
	nvsp_recv_buf_section m_recvSections[16];

	nvsp_message m_response;
};
