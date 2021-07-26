#pragma once
#include <cstdint>
#include <MetalOS.Internal.h>

//Sources: include\linux\hyperv.h

//Linux types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#define __packed 



static constexpr uint32_t HV_MESSAGE_SIZE = 256;
static constexpr uint32_t HV_MESSAGE_PAYLOAD_BYTE_COUNT = 240;
static constexpr uint32_t HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT = 30;
static constexpr uint32_t MAX_SIZE_CHANNEL_MESSAGE = HV_MESSAGE_PAYLOAD_BYTE_COUNT;

static constexpr uint32_t HV_SYNIC_SINT_COUNT = 16;
static constexpr uint32_t HV_EVENT_FLAGS_COUNT = 256 * 8;
static constexpr uint32_t HV_EVENT_FLAGS_BYTE_COUNT = 256;

#define HV_MESSAGE_TYPE_HYPERVISOR_MASK 0x80000000

enum HV_MESSAGE_TYPE : uint32_t
{
	HvMessageTypeNone = 0x00000000,
	// Memory access messages
	HvMessageTypeUnmappedGpa = 0x80000000,
	HvMessageTypeGpaIntercept = 0x80000001,
	// Timer notifications
	HvMessageTimerExpired = 0x80000010,
	// Error messages
	HvMessageTypeInvalidVpRegisterValue = 0x80000020,
	HvMessageTypeUnrecoverableException = 0x80000021,
	HvMessageTypeUnsupportedFeature = 0x80000022,
	HvMessageTypeTlbPageSizeMismatch = 0x80000023,
	// Trace buffer messages
	HvMessageTypeEventLogBuffersComplete = 0x80000040,
	// Hypercall intercept.
	HvMessageTypeHypercallIntercept = 0x80000050,
	// Platform-specific processor intercept messages
	HvMessageTypeX64IoPortIntercept = 0x80010000,
	HvMessageTypeMsrIntercept = 0x80010001,
	HvMessageTypeX64CpuidIntercept = 0x80010002,
	HvMessageTypeExceptionIntercept = 0x80010003,
	HvMessageTypeX64ApicEoi = 0x80010004,
	HvMessageTypeX64LegacyFpError = 0x80010005,
	HvMessageTypeRegisterIntercept = 0x80010006,
};

//HyperV TLFS Appendix B Page 203
enum HV_STATUS : uint64_t
{
	HV_STATUS_SUCCESS = 0x0,
	HV_STATUS_INVALID_HYPERCALL_CODE = 0x2,
	HV_STATUS_INVALID_HYPERCALL_INPUT = 0x3,
	HV_STATUS_INVALID_ALIGNMENT = 0x4,
	HV_STATUS_INVALID_PARAMETER = 0x5,
	HV_STATUS_ACCESS_DENIED = 0x6,
	HV_STATUS_INVALID_PARTITION_STATE = 0x7,
	HV_STATUS_OPERATION_DENIED = 0x8,
	HV_STATUS_UNKNOWN_PROPERTY = 0x9,
	HV_STATUS_PROPERTY_VALUE_OUT_OF_RANGE = 0xA,
	HV_STATUS_INSUFFICIENT_MEMORY = 0xB,
	HV_STATUS_PARTITION_TOO_DEEP = 0xC,
	HV_STATUS_INVALID_PARTITION_ID = 0xD,
	HV_STATUS_INVALID_VP_INDEX = 0xE,
	HV_STATUS_INVALID_PORT_ID = 0x11,
	HV_STATUS_INVALID_CONNECTION_ID = 0x12,
	HV_STATUS_INSUFFICIENT_BUFFERS = 0x13,
	HV_STATUS_NOT_ACKNOWLEDGED = 0x14,
	HV_STATUS_INVALID_VP_STATE = 0x15,
	HV_STATUS_ACKNOWLEDGED = 0x16,
	HV_STATUS_INVALID_SAVE_RESTORE_STATE = 0x17,
	HV_STATUS_INVALID_SYNIC_STATE = 0x18,
	HV_STATUS_OBJECT_IN_USE = 0x19,
	HV_STATUS_INVALID_PROXIMITY_DOMAIN_INFO = 0x1A,
	HV_STATUS_NO_DATA = 0x1B,
	HV_STATUS_INACTIVE = 0x1C,
	HV_STATUS_NO_RESOURCES = 0x1D,
	HV_STATUS_FEATURE_UNAVAILABLE = 0x1E,
	HV_STATUS_PARTIAL_PACKET = 0x1F,
	HV_STATUS_PROCESSOR_FEATURE_NOT_SUPPORTED = 0x20,
	HV_STATUS_PROCESSOR_CACHE_LINE_FLUSH_SIZE_INCOMPATIBLE = 0x30,
	HV_STATUS_INSUFFICIENT_BUFFER = 0x33,
	HV_STATUS_INCOMPATIBLE_PROCESSOR = 0x37,
	HV_STATUS_INSUFFICIENT_DEVICE_DOMAINS = 0x38,
	HV_STATUS_CPUID_FEATURE_VALIDATION_ERROR = 0x3C,
	HV_STATUS_CPUID_XSAVE_FEATURE_VALIDATION_ERROR = 0x3D,
	HV_STATUS_PROCESSOR_STARTUP_TIMEOUT = 0x3E,
	HV_STATUS_SMX_ENABLED = 0x3F,
	HV_STATUS_INVALID_LP_INDEX = 0x41,
	HV_STATUS_INVALID_REGISTER_VALUE = 0x50,
	HV_STATUS_NX_NOT_DETECTED = 0x55,
	HV_STATUS_INVALID_DEVICE_ID = 0x57,
	HV_STATUS_INVALID_DEVICE_STATE = 58,
	HV_STATUS_PENDING_PAGE_REQUESTS = 0x59,
	HV_STATUS_PAGE_REQUEST_INVALID = 0x60,
	HV_STATUS_OPERATION_FAILED = 0x71,
	HV_STATUS_NOT_ALLOWED_WITH_NESTED_VIRT_ACTIVE = 0x72
};
#define HV_SUCCESS(x) (x == HV_STATUS_SUCCESS)

typedef uint64_t HV_PARTITION_ID;

union HV_PORT_ID
{
	struct
	{
		UINT32 Id : 24;
		UINT32 Reserved : 8;
	};
	UINT32 AsUint32;
};

//If messagepending, write to EOM
struct HV_MESSAGE_FLAGS
{
	UINT8 MessagePending : 1;
	UINT8 Reserved : 7;
};

struct HV_MESSAGE_HEADER
{
	HV_MESSAGE_TYPE MessageType;
	UINT16 Reserved;
	HV_MESSAGE_FLAGS MessageFlags;
	UINT8 PayloadSize;
	union
	{
		UINT64 OriginationId;
		HV_PARTITION_ID Sender;
		HV_PORT_ID Port;
	};
};

struct HV_MESSAGE
{
	HV_MESSAGE_HEADER Header;
	UINT64 Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
};
static_assert((sizeof(HV_MESSAGE)* HV_SYNIC_SINT_COUNT) == PAGE_SIZE, "Bad sim page");

struct HV_SYNIC_EVENT_FLAGS
{
	UINT8 Flags[HV_EVENT_FLAGS_BYTE_COUNT];
};
static_assert((sizeof(HV_SYNIC_EVENT_FLAGS)* HV_SYNIC_SINT_COUNT) == PAGE_SIZE, "Bad event flags page");

struct HV_CONNECTION_ID
{
	union
	{
		struct
		{
			uint32_t Id : 24;
			uint32_t Reserved : 8;
		};
		uint32_t AsUint32;
	};
};

//HV TLFS 3.8
struct HV_HYPERCALL_RESULT_VALUE
{
	union
	{
		struct
		{
			HV_STATUS Status : 16;
			uint64_t Reserved1 : 16;
			uint64_t RepsComplete : 12;
			uint64_t Reserved2 : 20;
		};
		uint64_t AsUint64;
	};
};
static_assert(sizeof(HV_HYPERCALL_RESULT_VALUE) == sizeof(uint64_t), "HV_HYPERCALL_RESULT_VALUE incorrect");

//VMBus Implementation - mostly borrowed from Linux since its not documented anywhere
#pragma pack(push, 1)
/* Version 1 messages */
enum vmbus_channel_message_type : uint32_t
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

struct vmbus_channel_message_header {
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
			uint8_t	msg_sint;
			uint8_t	padding1[3];
			uint32_t	padding2;
		};
	};
	uint64_t monitor_page1;
	uint64_t monitor_page2;
};

struct vmbus_channel_version_response {
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
		UINT32  Data1;
		UINT16  Data2;
		UINT16  Data3;
		UINT8   Data4[8];
	};
} guid_t;

/* The size of the user defined data buffer for non-pipe offers. */
#define MAX_USER_DEFINED_BYTES		120

/* The size of the user defined data buffer for pipe offers. */
#define MAX_PIPE_USER_DEFINED_BYTES	116

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
	uint16_t mmio_megabytes;		/* in bytes * 1024 * 1024 */

	union {
		/* Non-pipes: The user has MAX_USER_DEFINED_BYTES bytes. */
		struct {
			unsigned char user_def[MAX_USER_DEFINED_BYTES];
		} std;

		/*
		 * Pipes:
		 * The following sructure is an integrated pipe protocol, which
		 * is implemented on top of standard user-defined data. Pipe
		 * clients have MAX_PIPE_USER_DEFINED_BYTES left for their own
		 * use.
		 */
		struct {
			uint32_t  pipe_mode;
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
	uint32_t child_relid;//channel id
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

/* Open Channel Result parameters */
struct vmbus_channel_open_result {
	vmbus_channel_message_header header;
	uint32_t child_relid;
	uint32_t openid;
	uint32_t status;
};

/* Close channel parameters; */
struct vmbus_channel_close_channel {
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
#define GPADL_TYPE_RING_BUFFER		1
#define GPADL_TYPE_SERVER_SAVE_AREA	2
#define GPADL_TYPE_TRANSACTION		8

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

/* This is the followup packet that contains more PFNs. */
struct vmbus_channel_gpadl_body
{
	vmbus_channel_message_header header;
	uint32_t msgnumber;
	uint32_t gpadl;
	uint64_t pfn[];
};

struct vmbus_channel_gpadl_created
{
	vmbus_channel_message_header header;
	uint32_t child_relid;
	uint32_t gpadl;
	uint32_t creation_status;
};

struct vmbus_channel_gpadl_teardown
{
	vmbus_channel_message_header header;
	uint32_t child_relid;
	uint32_t gpadl;
};

struct vmbus_channel_gpadl_torndown
{
	vmbus_channel_message_header header;
	uint32_t gpadl;
};

union VmBusResponse
{
	//vmbus_channel_version_supported version_supported;
	vmbus_channel_open_result open_result;
	vmbus_channel_gpadl_torndown gpadl_torndown;
	vmbus_channel_gpadl_created gpadl_created;
	vmbus_channel_version_response version_response;
};
#pragma pack(pop)

#define VERSION_WS2008  ((0 << 16) | (13))
#define VERSION_WIN7    ((1 << 16) | (1))
#define VERSION_WIN8    ((2 << 16) | (4))
#define VERSION_WIN8_1    ((3 << 16) | (0))
#define VERSION_WIN10 ((4 << 16) | (0))
#define VERSION_WIN10_V4_1 ((4 << 16) | (1))
#define VERSION_WIN10_V5 ((5 << 16) | (0))
#define VERSION_WIN10_V5_1 ((5 << 16) | (1))
#define VERSION_WIN10_V5_2 ((5 << 16) | (2))


/* Server Flags */
#define VMBUS_CHANNEL_ENUMERATE_DEVICE_INTERFACE	1
#define VMBUS_CHANNEL_SERVER_SUPPORTS_TRANSFER_PAGES	2
#define VMBUS_CHANNEL_SERVER_SUPPORTS_GPADLS		4
#define VMBUS_CHANNEL_NAMED_PIPE_MODE			0x10
#define VMBUS_CHANNEL_LOOPBACK_OFFER			0x100
#define VMBUS_CHANNEL_PARENT_OFFER			0x200
#define VMBUS_CHANNEL_REQUEST_MONITORED_NOTIFICATION	0x400
#define VMBUS_CHANNEL_TLNPI_PROVIDER_OFFER		0x2000

struct vmpacket_descriptor {
	u16 type;
	u16 offset8;
	u16 len8;
	u16 flags;
	u64 trans_id;
} __packed;

struct vmpacket_header {
	u32 prev_pkt_start_offset;
	struct vmpacket_descriptor descriptor;
} __packed;

struct vmtransfer_page_range {
	u32 byte_count;
	u32 byte_offset;
} __packed;

struct vmtransfer_page_packet_header {
	struct vmpacket_descriptor d;
	u16 xfer_pageset_id;
	u8  sender_owns_set;
	u8 reserved;
	u32 range_cnt;
	struct vmtransfer_page_range ranges[1];
} __packed;

struct vmgpadl_packet_header {
	struct vmpacket_descriptor d;
	u32 gpadl;
	u32 reserved;
} __packed;


struct vmadd_remove_transfer_page_set {
	struct vmpacket_descriptor d;
	u32 gpadl;
	u16 xfer_pageset_id;
	u16 reserved;
} __packed;

/*
 * This is the format for an Establish Gpadl packet, which contains a handle by
 * which this GPADL will be known and a set of GPA ranges associated with it.
 * This can be converted to a MDL by the guest OS.  If there are multiple GPA
 * ranges, then the resulting MDL will be "chained," representing multiple VA
 * ranges.
 */
struct vmestablish_gpadl {
	struct vmpacket_descriptor d;
	u32 gpadl;
	u32 range_cnt;
	struct gpa_range range[1];
} __packed;

/*
 * This is the format for a Teardown Gpadl packet, which indicates that the
 * GPADL handle in the Establish Gpadl packet will never be referenced again.
 */
struct vmteardown_gpadl {
	struct vmpacket_descriptor d;
	u32 gpadl;
	u32 reserved;	/* for alignment to a 8-byte boundary */
} __packed;

/*
 * This is the format for a GPA-Direct packet, which contains a set of GPA
 * ranges, in addition to commands and/or data.
 */
struct vmdata_gpa_direct {
	struct vmpacket_descriptor d;
	u32 reserved;
	u32 range_cnt;
	struct gpa_range range[1];
} __packed;

/* This is the format for a Additional Data Packet. */
struct vmadditional_data {
	struct vmpacket_descriptor d;
	u64 total_bytes;
	u32 offset;
	u32 byte_cnt;
	unsigned char data[1];
} __packed;

union vmpacket_largest_possible_header {
	struct vmpacket_descriptor simple_hdr;
	struct vmtransfer_page_packet_header xfer_page_hdr;
	struct vmgpadl_packet_header gpadl_hdr;
	struct vmadd_remove_transfer_page_set add_rm_xfer_page_hdr;
	struct vmestablish_gpadl establish_gpadl_hdr;
	struct vmteardown_gpadl teardown_gpadl_hdr;
	struct vmdata_gpa_direct data_gpa_direct_hdr;
};

#define VMPACKET_DATA_START_ADDRESS(__packet)	\
	(void *)(((unsigned char *)__packet) +	\
	 ((struct vmpacket_descriptor)__packet)->offset8 * 8)

#define VMPACKET_DATA_LENGTH(__packet)		\
	((((struct vmpacket_descriptor)__packet)->len8 -	\
	  ((struct vmpacket_descriptor)__packet)->offset8) * 8)

#define VMPACKET_TRANSFER_MODE(__packet)	\
	(((struct IMPACT)__packet)->type)

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

#define VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED	1

struct hv_ring_buffer {
	/* Offset in bytes from the start of ring data below */
	uint32_t write_index;

	/* Offset in bytes from the start of ring data below */
	uint32_t read_index;

	uint32_t interrupt_mask;

	/*
	 * WS2012/Win8 and later versions of Hyper-V implement interrupt
	 * driven flow management. The feature bit feat_pending_send_sz
	 * is set by the host on the host->guest ring buffer, and by the
	 * guest on the guest->host ring buffer.
	 *
	 * The meaning of the feature bit is a bit complex in that it has
	 * semantics that apply to both ring buffers.  If the guest sets
	 * the feature bit in the guest->host ring buffer, the guest is
	 * telling the host that:
	 * 1) It will set the pending_send_sz field in the guest->host ring
	 *    buffer when it is waiting for space to become available, and
	 * 2) It will read the pending_send_sz field in the host->guest
	 *    ring buffer and interrupt the host when it frees enough space
	 *
	 * Similarly, if the host sets the feature bit in the host->guest
	 * ring buffer, the host is telling the guest that:
	 * 1) It will set the pending_send_sz field in the host->guest ring
	 *    buffer when it is waiting for space to become available, and
	 * 2) It will read the pending_send_sz field in the guest->host
	 *    ring buffer and interrupt the guest when it frees enough space
	 *
	 * If either the guest or host does not set the feature bit that it
	 * owns, that guest or host must do polling if it encounters a full
	 * ring buffer, and not signal the other end with an interrupt.
	 */
	uint32_t pending_send_sz;
	uint32_t reserved1[12];
	union {
		struct {
			uint32_t feat_pending_send_sz : 1;
		};
		uint32_t value;
	} feature_bits;

	/* Pad it to PAGE_SIZE so that data starts on page boundary */
	uint8_t	reserved2[4028];

	/*
	 * Ring data starts here + RingDataStartOffset
	 * !!! DO NOT place any fields below this !!!
	 */
	uint8_t buffer[0];
};
static_assert(sizeof(hv_ring_buffer) == PAGE_SIZE, "Invalid hv_ring_buffer");


//
// Definitions for the monitored notification facility
//

typedef union _HV_MONITOR_TRIGGER_GROUP
{
	UINT64 AsUINT64;

	struct
	{
		UINT32 Pending;
		UINT32 Armed;
	};

} HV_MONITOR_TRIGGER_GROUP, * PHV_MONITOR_TRIGGER_GROUP;

typedef struct _HV_MONITOR_PARAMETER
{
	HV_CONNECTION_ID    ConnectionId;
	UINT16              FlagNumber;
	UINT16              RsvdZ;
} HV_MONITOR_PARAMETER, * PHV_MONITOR_PARAMETER;

typedef union _HV_MONITOR_TRIGGER_STATE
{
	UINT32 AsUINT32;

	struct
	{
		UINT32 GroupEnable : 4;
		UINT32 RsvdZ : 28;
	};

} HV_MONITOR_TRIGGER_STATE, * PHV_MONITOR_TRIGGER_STATE;

//
// HV_MONITOR_PAGE Layout
// ------------------------------------------------------
// | 0   | TriggerState (4 bytes) | Rsvd1 (4 bytes)     |
// | 8   | TriggerGroup[0]                              |
// | 10  | TriggerGroup[1]                              |
// | 18  | TriggerGroup[2]                              |
// | 20  | TriggerGroup[3]                              |
// | 28  | Rsvd2[0]                                     |
// | 30  | Rsvd2[1]                                     |
// | 38  | Rsvd2[2]                                     |
// | 40  | NextCheckTime[0][0]    | NextCheckTime[0][1] |
// | ...                                                |
// | 240 | Latency[0][0..3]                             |
// | 340 | Rsvz3[0]                                     |
// | 440 | Parameter[0][0]                              |
// | 448 | Parameter[0][1]                              |
// | ...                                                |
// | 840 | Rsvd4[0]                                     |
// ------------------------------------------------------

typedef struct _HV_MONITOR_PAGE
{
	HV_MONITOR_TRIGGER_STATE TriggerState;
	UINT32                   RsvdZ1;

	HV_MONITOR_TRIGGER_GROUP TriggerGroup[4];
	UINT64                   RsvdZ2[3]; // 64

	UINT16                   NextCheckTime[4][32]; // 256
	UINT64                   RsvdZ3[32]; // 256

	UINT16                   Latency[4][32]; // 256
	UINT64                   RsvdZ4[32]; // 256

	HV_MONITOR_PARAMETER     Parameter[4][32]; // 1024

	UINT8                    RsvdZ5[1984];

} HV_MONITOR_PAGE, * PHV_MONITOR_PAGE;
static_assert(sizeof(HV_MONITOR_PAGE) == PAGE_SIZE, "Invalid monitor page");

typedef volatile HV_MONITOR_PAGE* PVHV_MONITOR_PAGE;


