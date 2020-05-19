#pragma once
#include <cstdint>
#include <MetalOS.Internal.h>
#include <WindowsTypes.h>

static constexpr uint32_t HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT = 30;

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
	union
	{
		uint8_t b[16];
		struct
		{
			UINT32  Data1;
			UINT16  Data2;
			UINT16  Data3;
			UINT8   Data4[8];
		};
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
	uint32_t child_relid;
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
