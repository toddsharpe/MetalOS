#pragma once

#include "core_crt/stdint.h"

// VMBus Implementation - mostly borrowed from Linux since its not documented anywhere

#pragma pack(push, 1)
namespace HyperV
{
	static constexpr uint32_t VERSION_WIN10_V5_2 = ((5 << 16) | (2));

	/* Version 1 messages */
	enum class vmbus_channel_message_type : uint32_t
	{
		CHANNELMSG_INVALID = 0,
		CHANNELMSG_OFFERCHANNEL = 1,
		CHANNELMSG_RESCIND_CHANNELOFFER = 2,
		CHANNELMSG_REQUESTOFFERS = 3,
		CHANNELMSG_ALLOFFERS_DELIVERED = 4,
		CHANNELMSG_OPENCHANNEL = 5,
		CHANNELMSG_OPENCHANNEL_RESULT = 6,
		CHANNELMSG_CLOSECHANNEL = 7,
		CHANNELMSG_GPADL_HEADER = 8,
		CHANNELMSG_GPADL_BODY = 9,
		CHANNELMSG_GPADL_CREATED = 10,
		CHANNELMSG_GPADL_TEARDOWN = 11,
		CHANNELMSG_GPADL_TORNDOWN = 12,
		CHANNELMSG_RELID_RELEASED = 13,
		CHANNELMSG_INITIATE_CONTACT = 14,
		CHANNELMSG_VERSION_RESPONSE = 15,
		CHANNELMSG_UNLOAD = 16,
		CHANNELMSG_UNLOAD_RESPONSE = 17,
		CHANNELMSG_18 = 18,
		CHANNELMSG_19 = 19,
		CHANNELMSG_20 = 20,
		CHANNELMSG_TL_CONNECT_REQUEST = 21,
		CHANNELMSG_22 = 22,
		CHANNELMSG_TL_CONNECT_RESULT = 23,
		CHANNELMSG_COUNT
	};

	struct vmbus_channel_message_header
	{
		vmbus_channel_message_type msgtype;
		uint32_t padding;
	};

	struct vmbus_channel_initiate_contact
	{
		vmbus_channel_message_header header;
		uint32_t vmbus_version_requested;
		uint32_t target_vcpu; /* The VCPU the host should respond to */
		union
		{
			uint64_t interrupt_page;
			struct
			{
				uint8_t msg_sint;
				uint8_t padding1[3];
				uint32_t padding2;
			};
		};
		uint64_t monitor_page1;
		uint64_t monitor_page2;
	};

	struct vmbus_channel_version_response
	{
		vmbus_channel_message_header header;
		uint8_t version_supported;

		uint8_t connection_state;
		uint16_t padding;

		/*
		 * On new hosts that support VMBus protocol 5.0, we must use
		 * VMBUS_MESSAGE_CONNECTION_ID_4 for the Initiate Contact Message,
		 * and for subsequent messages, we must use the Message Connection ID
		 * field in the host-returned Version Response Message.
		 *
		 * On old hosts, we should always use VMBUS_MESSAGE_CONNECTION_ID (1).
		 */
		uint32_t msg_conn_id;
	};

	typedef struct
	{
		struct
		{
			uint32_t Data1;
			uint16_t Data2;
			uint16_t Data3;
			uint8_t Data4[8];
		};
	} guid_t;

	/* The size of the user defined data buffer for non-pipe offers. */
	static constexpr size_t MAX_USER_DEFINED_BYTES = 120;

	/* The size of the user defined data buffer for pipe offers. */
	static constexpr size_t MAX_PIPE_USER_DEFINED_BYTES = 116;

	struct vmbus_channel_offer
	{
		guid_t if_type;
		guid_t if_instance;

		/*
		 * These two fields are not currently used.
		 */
		uint64_t reserved1;
		uint64_t reserved2;

		uint16_t chn_flags;
		uint16_t mmio_megabytes; /* in bytes * 1024 * 1024 */

		union
		{
			/* Non-pipes: The user has MAX_USER_DEFINED_BYTES bytes. */
			struct
			{
				unsigned char user_def[MAX_USER_DEFINED_BYTES];
			} std;

			/*
			 * Pipes:
			 * The following sructure is an integrated pipe protocol, which
			 * is implemented on top of standard user-defined data. Pipe
			 * clients have MAX_PIPE_USER_DEFINED_BYTES left for their own
			 * use.
			 */
			struct
			{
				uint32_t pipe_mode;
				unsigned char user_def[MAX_PIPE_USER_DEFINED_BYTES];
			} pipe;
		} u;
		/*
		 * The sub_channel_index is defined in Win8: a value of zero means a
		 * primary channel and a value of non-zero means a sub-channel.
		 *
		 * Before Win8, the field is reserved, meaning it's always zero.
		 */
		uint16_t sub_channel_index;
		uint16_t reserved3;
	};

	struct vmbus_channel_offer_channel
	{
		vmbus_channel_message_header header;
		vmbus_channel_offer offer;
		uint32_t child_relid; // channel id
		uint8_t monitorid;
		/*
		 * win7 and beyond splits this field into a bit field.
		 */
		uint8_t monitor_allocated : 1;
		uint8_t reserved : 7;
		/*
		 * These are new fields added in win7 and later.
		 * Do not access these fields without checking the
		 * negotiated protocol.
		 *
		 * If "is_dedicated_interrupt" is set, we must not set the
		 * associated bit in the channel bitmap while sending the
		 * interrupt to the host.
		 *
		 * connection_id is to be used in signaling the host.
		 */
		uint16_t is_dedicated_interrupt : 1;
		uint16_t reserved1 : 15;
		uint32_t connection_id;
	};

	struct vmbus_channel_gpadl_created
	{
		vmbus_channel_message_header header;
		uint32_t child_relid;
		uint32_t gpadl;
		uint32_t creation_status;
	};

	struct vmbus_channel_open_result
	{
		vmbus_channel_message_header header;
		uint32_t child_relid;
		uint32_t openid;
		uint32_t status;
	};

	struct vmbus_channel_gpadl_torndown
	{
		vmbus_channel_message_header header;
		uint32_t gpadl;
	};

	union VmBusResponse
	{
		// vmbus_channel_version_supported version_supported;
		vmbus_channel_open_result open_result;
		vmbus_channel_gpadl_torndown gpadl_torndown;
		vmbus_channel_gpadl_created gpadl_created;
		vmbus_channel_version_response version_response;
	};

	/* Open Channel parameters */
	struct vmbus_channel_open_channel
	{
		vmbus_channel_message_header header;

		/* Identifies the specific VMBus channel that is being opened. */
		uint32_t child_relid;

		/* ID making a particular open request at a channel offer unique. */
		uint32_t openid;

		/* GPADL for the channel's ring buffer. */
		uint32_t ringbuffer_gpadlhandle;

		/*
		 * Starting with win8, this field will be used to specify
		 * the target virtual processor on which to deliver the interrupt for
		 * the host to guest communication.
		 * Prior to win8, incoming channel interrupts would only
		 * be delivered on cpu 0. Setting this value to 0 would
		 * preserve the earlier behavior.
		 */
		uint32_t target_vp;

		/*
		 * The upstream ring buffer begins at offset zero in the memory
		 * described by RingBufferGpadlHandle. The downstream ring buffer
		 * follows it at this offset (in pages).
		 */
		uint32_t downstream_ringbuffer_pageoffset;

		/* User-specific data to be passed along to the server endpoint. */
		unsigned char userdata[MAX_USER_DEFINED_BYTES];
	};

	/* Close channel parameters; */
	struct vmbus_channel_close_channel
	{
		struct vmbus_channel_message_header header;
		uint32_t child_relid;
	};

/*
 * This structure defines a range in guest physical space that can be made to
 * look virtually contiguous.
 */
#define ANYSIZE_ARRAY 1
	struct gpa_range
	{
		uint32_t byte_count;
		uint32_t byte_offset;
		uint64_t pfn_array[ANYSIZE_ARRAY];
	};

/* Channel Message GPADL */
#define GPADL_TYPE_RING_BUFFER 1
#define GPADL_TYPE_SERVER_SAVE_AREA 2
#define GPADL_TYPE_TRANSACTION 8

	/*
	 * The number of PFNs in a GPADL message is defined by the number of
	 * pages that would be spanned by ByteCount and ByteOffset.  If the
	 * implied number of PFNs won't fit in this packet, there will be a
	 * follow-up packet that contains more.
	 */
	struct vmbus_channel_gpadl_header
	{
		vmbus_channel_message_header header;
		uint32_t child_relid;
		uint32_t gpadl;
		uint16_t range_buflen;
		uint16_t rangecount;
		gpa_range range[];
	};

	struct vmbus_channel_gpadl_body
	{
		vmbus_channel_message_header header;
		uint32_t msgnumber;
		uint32_t gpadl;
		uint64_t pfn[ANYSIZE_ARRAY];
	};

	enum vmbus_packet_type {
		VM_PKT_INVALID = 0x0,
		VM_PKT_SYNCH = 0x1,
		VM_PKT_ADD_XFER_PAGESET = 0x2,
		VM_PKT_RM_XFER_PAGESET = 0x3,
		VM_PKT_ESTABLISH_GPADL = 0x4,
		VM_PKT_TEARDOWN_GPADL = 0x5,
		VM_PKT_DATA_INBAND = 0x6,
		VM_PKT_DATA_USING_XFER_PAGES = 0x7,
		VM_PKT_DATA_USING_GPADL = 0x8,
		VM_PKT_DATA_USING_GPA_DIRECT = 0x9,
		VM_PKT_CANCEL_REQUEST = 0xa,
		VM_PKT_COMP = 0xb,
		VM_PKT_DATA_USING_ADDITIONAL_PKT = 0xc,
		VM_PKT_ADDITIONAL_DATA = 0xd
	};

	/* The size of the user defined data buffer for non-pipe offers. */
	#define MAX_USER_DEFINED_BYTES		120

	struct vmpacket_descriptor {
		uint16_t type;
		uint16_t offset8;
		uint16_t len8;
		uint16_t flags;
		uint64_t trans_id;
	};
#define VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED	1
}
#pragma pack(pop)
