#pragma once

#include "kernel/Drivers/Driver.h"
#include "kernel/Plat_HyperV/Drivers/HyperVChannel.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/Kernel_net.h"

namespace
{

#define NIC_SEND_RING_BUFFER_SIZE    (40 * 1024)
#define NIC_RECV_RING_BUFFER_SIZE    (40 * 1024)
#define NIC_RECV_BUFFER_ID           0xcafe
#define NIC_RECV_BUFFER_PAGES        4096  // 16 MB
#define NIC_SEND_BUFFER_ID           0
#define NIC_SEND_BUFFER_PAGES        4096  // 16 MB

#define NVSP_PROTOCOL_VERSION_2      0x30002
#define NVSP_STATUS_SUCCESS          1
#define NVSP_CHANNEL_TYPE_DATA       0

enum class NvspMessageType : uint32_t
{
	None = 0,
	Init = 1,
	InitComplete = 2,
	SendNdisVersion = 100,
	SendReceiveBuffer = 101,
	SendReceiveBufferComplete = 102,
	SendSendBuffer = 104,
	SendSendBufferComplete = 105,
	SendRndisPacket = 107,
	SendRndisPacketComplete = 108,
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
	nvsp_recv_buf_section sections[1]; // unread, but it makes this the largest member: sizeof(nvsp_message) must be 28
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

struct nvsp_send_rndis_packet_complete_message
{
	uint32_t status;
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
	} msg;
};
static_assert(sizeof(nvsp_message) == 28, "Host requires the full 28-byte nvsp_message (sized by send_recv_buf_complete)");

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
#define RNDIS_MSG_QUERY   0x00000004U
#define RNDIS_MSG_QUERY_C 0x80000004U
#define RNDIS_MSG_SET     0x00000005U
#define RNDIS_MSG_PACKET  0x00000001U

#define OID_802_3_PERMANENT_ADDRESS  0x01010101U

#define NVSP_CHANNEL_TYPE_CONTROL 1

#define OID_GEN_CURRENT_PACKET_FILTER   0x0001010EU
#define RNDIS_PACKET_TYPE_DIRECTED      0x00000001U
#define RNDIS_PACKET_TYPE_ALL_MULTICAST 0x00000004U
#define RNDIS_PACKET_TYPE_BROADCAST     0x00000008U

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

class HyperVNic : public NicDriver
{
public:
	HyperVNic(KDevice& device);

	bool Initialize() override;

	static void Callback(void* context) { ((HyperVNic*)context)->OnCallback(); }

	// NicDriver interface
	bool SendFrame(const void* const frame, const size_t length) override;
	void GetMac(uint8_t out[6]) const override;

private:
	void OnCallback();
	void SendNvsp(const nvsp_message& msg, bool requestCompletion = false);
	void SendNvspCompletion(const nvsp_message& msg, uint64_t transId);
	bool SendRndisPacket(uint32_t channelType, const void* rndisData, uint32_t rndisLen, bool requestCompletion = false, uint32_t section = 0);
	uint32_t AcquireSendSection();
	void ReleaseSendSection(uint64_t transId);

	//Send-buffer sections: 0 is for init-time control messages; a data frame holds one until SendRndisPacketComplete
	static constexpr uint32_t MaxSendSections = 64; //Width of m_sendSectionsInUse
	static constexpr uint64_t SendSectionTag = 0x534E'4400'0000'0000; //trans_id = tag | section
	static constexpr uint64_t SendSectionTagMask = 0xFFFF'FF00'0000'0000;

	HyperVChannel m_channel;
	KEvent m_event;

	KMacAddress m_macAddress;

	uint8_t* m_sendBuf;
	uint32_t m_sendSectionSize;
	uint32_t m_sendSectionCount;
	uint64_t m_sendSectionsInUse; //Bit per section; changed with interrupts off (completions run in the ISR)

	uint8_t* m_recvBuf;

	nvsp_message m_response;
};
