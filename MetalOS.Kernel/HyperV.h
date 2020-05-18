#pragma once
#include <cstdint>
#include "msvc.h"
#include <string>
#include <bitset>
#include <intrin.h>
#include "MetalOS.Kernel.h"
//#include "Main.h"

//TODO: separate into hypervplatform and hypervinfo
//platform is basically the hal
//hypervinfo will be like cpuid, just a light weight way to check features
class HyperV
{
public:
	typedef uint64_t HV_SYNIC_SINT_INDEX;


	HyperV();

	void Initialize();

	void Connect();

	bool IsPresent()
	{
		return m_isPresent;
	}

	bool DirectSyntheticTimers() { return m_featuresEdx[19]; }

	bool AccessPartitionReferenceCounter() { return m_featuresEax[9]; }

	bool DeprecateAutoEOI() { return m_recommendationsEax[9]; }

	bool InputInXMM() { return m_featuresEdx[4]; }

	static void EOI()
	{
		__writemsr(HV_X64_MSR_EOI, 0);
	}

	nano100_t ReadTsc();
	uint64_t TscFreq()
	{
		return __readmsr(HV_X64_MSR_TSC_FREQUENCY);
	}

	void SetSynicMessagePage(paddr_t address)
	{
		HV_INTERRUPT_PAGE_REGISTER reg;
		reg.AsUint64 = __readmsr(SIMP);
		reg.Enable = true;
		reg.BaseAddress = (address >> PAGE_SHIFT);

		__writemsr(SIMP, reg.AsUint64);
	}

	void SetSynicEventPage(paddr_t address)
	{
		HV_INTERRUPT_PAGE_REGISTER reg;
		reg.AsUint64 = __readmsr(SIEFP);
		reg.Enable = true;
		reg.BaseAddress = (address >> PAGE_SHIFT);

		__writemsr(SIEFP, reg.AsUint64);
	}

	void EnableSynic()
	{
		HV_SCONTROL_REGISTER reg;
		reg.AsUint64 = __readmsr(SCONTROl);
		reg.Enabled = true;
		__writemsr(SCONTROl, reg.AsUint64);
	}

	uint64_t ReadSintRegister(uint32_t index)
	{
		//Assert(index < HV_SYNIC_SINT_COUNT);
		return __readmsr(SINT0 + index);
	}

	void WriteSintRegister(uint32_t index, uint64_t value)
	{
		//Assert(index < HV_SYNIC_SINT_COUNT);
		__writemsr(SINT0 + index, value);
	}

	void SignalEom()
	{
		__writemsr(EOM, 0);
	}

	static void OnInterrupt(void* arg) { ((HyperV*)arg)->OnInterrupt(); };
	void OnInterrupt();

private:
	static uint32_t ThreadLoop(void* arg);

	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	//Each slot is for one interrupt source
	//SIEF page: 16-element array of 256-byte event flags (Synic event)
	//SIM page: 16-element array of 256-byte messages HV_MESSAGE (Synic message)
	static const uint32_t HV_SYNIC_SINT_COUNT = 16; // Corresponds to a single synthetic interrupt slot

	typedef uint64_t HV_SPA;
	typedef uint64_t HV_GPA;
	typedef uint64_t HV_GVA;
	typedef uint64_t HV_PARTITION_ID;
	typedef const void* PCVOID;
	typedef uint64_t UINT64;
	typedef uint32_t UINT32;
	typedef uint16_t UINT16;
	typedef uint8_t UINT8;

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

	typedef UINT32 HV_VP_INDEX;
#define HV_ANY_VP ((HV_VP_INDEX)-1)
#define HV_VP_INDEX_SELF ((HV_VP_INDEX)-2) 

	enum HV_CPUID
	{
		VENDOR = 0x40000000,
		INTERFACE = 0x40000001,
		VERSION = 0x40000002,
		FEATURES = 0x40000003,
		PERF_RECOMMENDATIONS = 0x40000004,

	};

	enum HV_REG
	{
		HV_X64_MSR_GUEST_OS_ID = 0x40000000,
		HV_X64_MSR_HYPERCALL = 0x40000001,
		HV_X64_MSR_VP_INDEX = 0x40000002,
		HV_X64_MSR_RESET = 0x40000003,
		HV_X64_MSR_VP_RUNTIME = 0x40000010,
		HV_X64_MSR_TIME_REF_COUNT = 0x40000020,
		HV_X64_MSR_REFERENCE_TSC = 0x40000021,
		HV_X64_MSR_TSC_FREQUENCY = 0x40000022, //Frequency of TSC in Hz
		HV_X64_MSR_APIC_FREQUENCY = 0x40000023,
		HV_X64_MSR_NPIEP_CONFIG = 0x40000040,
		HV_X64_MSR_EOI = 0x40000070,
		HV_X64_MSR_ICR = 0x40000071,
		HV_X64_MSR_TPR = 0x40000072,

		HV_X64_MSR_STIMER0_CONFIG = 0x400000B0,
		HV_X64_MSR_STIMER0_COUNT = 0x400000B1,
		HV_X64_MSR_STIMER1_CONFIG = 0x400000B2,
		HV_X64_MSR_STIMER1_COUNT = 0x400000B3,
		HV_X64_MSR_STIMER2_CONFIG = 0x400000B4,
		HX_X64_MSR_STIMER2_COUNT = 0x400000B5,
		HV_X64_MSR_STIMER3_CONFIG = 0x400000B6,
		HV_X64_MSR_STIMER3_COUNT = 0x400000B7,

		//HyperV TLFS 11.8
		//Each Processor has its own copy of these
		SCONTROl = 0x40000080, //SynIC Control
		SVERSION = 0x40000081, //SynIC Version RO
		SIEFP = 0x40000082, //Interrupt Event Flags Page
		SIMP = 0x40000083, //Interrupt Message Page
		EOM = 0x40000084, //End of message (write-only trigger)
		SINT0 = 0x40000090, //Interrupt Source 0
		SINT1 = 0x40000091, //Interrupt Source 1
		SINT2 = 0x40000092, //Interrupt Source 2
		SINT3 = 0x40000093, //Interrupt Source 3
		SINT4 = 0x40000094, //Interrupt Source 4
		SINT5 = 0x40000095, //Interrupt Source 5
		SINT6 = 0x40000096, //Interrupt Source 6
		SINT7 = 0x40000097, //Interrupt Source 7
		SINT8 = 0x40000098, //Interrupt Source 8
		SINT9 = 0x40000099, //Interrupt Source 9
		SINT10 = 0x4000009A, //Interrupt Source 10
		SINT11 = 0x4000009B, //Interrupt Source 11
		SINT12 = 0x4000009C, //Interrupt Source 12
		SINT13 = 0x4000009D, //Interrupt Source 13
		SINT14 = 0x4000009E, //Interrupt Source 14
		SINT15 = 0x4000009F, //Interrupt Source 15
	};

	struct HV_REF_TSC_REG
	{
		union
		{
			struct
			{
				uint64_t Enable : 1;
				uint64_t Reserved : 11;
				uint64_t GPAPageNumber : 52;
			};
			uint64_t AsUint64;
		};
	};

	typedef struct _HV_REFERENCE_TSC_PAGE
	{
		volatile uint32_t TscSequence;
		uint32_t Reserved1;
		volatile uint64_t TscScale;
		volatile int64_t TscOffset;
		uint64_t Reserved2[509];
	} HV_REFERENCE_TSC_PAGE, * PHV_REFERENCE_TSC_PAGE;


	struct HV_SCONTROL_REGISTER
	{
		union
		{
			struct
			{
				uint64_t Enabled : 1;
				uint64_t Reserved : 63;
			};
			uint64_t AsUint64;
		};
	};

	//TODO: change Value to AsUint64 everywhere
	struct HV_SVERSION_REGISTER
	{
		union
		{
			struct
			{
				uint64_t SyncICVersion : 32;
				uint64_t Reserved : 32;
			};
			uint64_t Value;
		};
	};

	struct HV_HYPERCALL_REGISTER
	{
		union
		{
			struct
			{
				uint64_t Enabled : 1;
				uint64_t Locked : 1;
				uint64_t Reserved : 10;
				uint64_t HypercallGPFN : 42; // Guest physical page number
			};
			uint64_t AsUint64;
		};
	};

	struct HV_INTERRUPT_PAGE_REGISTER //SIEFP, SIMP
	{
		union
		{
			struct
			{
				uint64_t Enable : 1;
				uint64_t Reserved : 11;
				uint64_t BaseAddress : 52;//Low 12 bits assumed to be 0, GPA = guest physical address
			};
			uint64_t AsUint64;
		};
	};

	struct HV_SINT_REGISTER
	{
		union
		{
			struct
			{
				uint64_t Vector : 8; //[16-255]
				uint64_t Reserved0 : 8;
				uint64_t Masked : 1;
				uint64_t AutoEOI : 1;
				uint64_t Polling : 1;
				uint64_t Reserved1 : 45;
			};
			uint64_t AsUint64;
		};
	};
	//Default value: 0x10000 (masked by default)

#define HV_MESSAGE_TYPE_HYPERVISOR_MASK 0x80000000

	typedef enum
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
	} HV_MESSAGE_TYPE;

	typedef struct
	{
		UINT8 MessagePending : 1;
		UINT8 Reserved : 7;
	} HV_MESSAGE_FLAGS;
	//If messagepending, write to EOM

	typedef union
	{
		UINT32 AsUint32;
		struct
		{
			UINT32 Id : 24;
			UINT32 Reserved : 8;
		};
	} HV_PORT_ID;

	//three types of ports
	//message ports - HvPostMessage hypercall
	//event ports - HvSignalEvent
	//monitor ports - monitor pages
	enum HV_PORT_TYPE
	{
		HvPortTypeMessage = 1,
		HvPortTypeEvent = 2,
		HvPortTypeMonitor = 3
	};

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

	typedef struct
	{
		UINT64 AsUINT64;
		struct
		{
			UINT32 Pending;
			UINT32 Armed;
		};
	} HV_MONITOR_TRIGGER_GROUP, * PHV_MONITOR_TRIGGER_GROUP;

	typedef struct
	{
		HV_CONNECTION_ID ConnectionId;
		UINT16 FlagNumber;
		UINT16 ReservedZ;
	} HV_MONITOR_PARAMETER, * PHV_MONITOR_PARAMETER;


	typedef struct
	{
		HV_PORT_TYPE PortType;
		UINT32 ReservedZ;
		union
		{
			struct
			{
				UINT64 ReservedZ;
			} MessageConnectionInfo;
			struct
			{
				UINT64 ReservedZ;
			} EventConnectionInfo;
			struct
			{
				HV_GPA MonitorAddress;
			} MonitorConnectionInfo;
		};
	} HV_CONNECTION_INFO, * PHV_CONNECTION_INFO;

	typedef struct
	{
		HV_PORT_TYPE PortType;
		UINT32 ReservedZ;
		union
		{
			struct
			{
				HV_SYNIC_SINT_INDEX TargetSint;
				HV_VP_INDEX TargetVp;
				UINT64 ReservedZ;
			} MessagePortInfo;
			struct
			{
				HV_SYNIC_SINT_INDEX TargetSint;
				HV_VP_INDEX TargetVp;
				UINT16 BaseFlagNumber;
				UINT16 FlagCount;
				UINT32 ReservedZ;
			} EventPortInfo;
			struct
			{
				HV_GPA MonitorAddress;
				UINT64 ReservedZ;
			} MonitorPortInfo;
		};
	} HV_PORT_INFO;


#define HV_MESSAGE_SIZE 256
#define HV_MESSAGE_MAX_PAYLOAD_BYTE_COUNT 240
#define HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT 30
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

	typedef struct
	{
		HV_MESSAGE_HEADER Header;
		UINT64 Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	} HV_MESSAGE;
	static_assert((sizeof(HV_MESSAGE) * HV_SYNIC_SINT_COUNT) == PAGE_SIZE, "Bad sim page");

#define HV_EVENT_FLAGS_COUNT (256 * 8)
#define HV_EVENT_FLAGS_BYTE_COUNT 256
	typedef struct
	{
		UINT8 Flags[HV_EVENT_FLAGS_BYTE_COUNT];
	} HV_SYNIC_EVENT_FLAGS;
	static_assert((sizeof(HV_SYNIC_EVENT_FLAGS)* HV_SYNIC_SINT_COUNT) == PAGE_SIZE, "Bad event flags page");

	typedef UINT32 HV_INTERCEPT_ACCESS_TYPE_MASK;

	typedef struct
	{
		UINT16 Cpl : 2;
		UINT16 Cr0Pe : 1;
		UINT16 Cr0Am : 1;
		UINT16 EferLma : 1;
		UINT16 DebugActive : 1;
		UINT16 InterruptionPending : 1;
		UINT16 Reserved1 : 4;
		UINT16 Reserved2 : 5;
	} HV_X64_VP_EXECUTION_STATE;

	typedef struct _HV_X64_SEGMENT_REGISTER {
		UINT64 Base;
		UINT32 Limit;
		UINT16 Selector;
		union {
			struct {
				UINT16 SegmentType : 4;
				UINT16 NonSystemSegment : 1;
				UINT16 DescriptorPrivilegeLevel : 2;
				UINT16 Present : 1;
				UINT16 Reserved : 4;
				UINT16 Available : 1;
				UINT16 Long : 1;
				UINT16 Default : 1;
				UINT16 Granularity : 1;
			};
			UINT16 Attributes;
		};
	} HV_X64_SEGMENT_REGISTER, * PHV_X64_SEGMENT_REGISTER;

	typedef struct
	{
		HV_VP_INDEX VpIndex;
		UINT8 InstructionLength;
		HV_INTERCEPT_ACCESS_TYPE_MASK InterceptAccessType;
		HV_X64_VP_EXECUTION_STATE ExecutionState;
		HV_X64_SEGMENT_REGISTER CsSegment;
		UINT64 Rip;
		UINT64 Rflags;
	} HV_X64_INTERCEPT_MESSAGE_HEADER;


	enum VENDOR_ID : uint64_t
	{
		Microsoft = 0x001,
		HPE = 0x002,
		LANCOM = 0x200,
	};

	enum OSID : uint64_t
	{
		Undefined = 0,
		MSDOS = 1,
		Windows3x = 2,
		Windows9x = 3,
		WindowsNT = 4,
		WindowsCE = 5
	};

	enum OSType : uint64_t
	{
		Linux = 0x1,
		FreeBSD = 0x2,
		Xen = 0x3,
		Illumos = 0x4,
	};

	//Hypervisor TLFS 2.6.2 for Open Source OS
	struct GUEST_OS_ID_REG
	{
		union
		{
			struct
			{
				uint64_t BuildNumber : 16;
				uint64_t Version : 32;
				uint64_t OSID : 8;
				OSType OSType : 7;
				uint64_t OpenSource : 1; // Set to 1
			};
			uint64_t Value;
		};
	};

	//HV TLFS 3.7
	struct HV_HYPERCALL_INPUT_VALUE
	{
		union
		{
			struct
			{
				uint64_t CallCode : 16;
				uint64_t Fast : 1;
				uint64_t HeaderSize : 9;
				uint64_t IsNested : 1;
				uint64_t ReservedZ1 : 5;
				uint64_t RepCount : 12;
				uint64_t ReservedZ2 : 4;
				uint64_t RepStartIndex : 12;
				uint64_t ReservedZ3 : 4;
			};
			uint64_t AsUint64;
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

	enum HypercallCodes
	{
		PostMessage = 0x005C
	};

	//Code: 0x005C
	//Params:
	//0 - ConnectionId (4 bytes), Padding (4 bytes) - from HvConnectPort
	//8 - MessageType (4 bytes), PayloadSize (4 bytes) - TopBit of MessageType is cleared. PayloadSize (up to 240 bytes)
	//16 Message[0] (8 bytes)
	//248 Message[29] (8 bytes)
	HV_HYPERCALL_RESULT_VALUE
		HvPostMessage(
			__in HV_CONNECTION_ID ConnectionId,
			__in HV_MESSAGE_TYPE MessageType,
			__in UINT32 PayloadSize,
			__in_ecount(PayloadSize)
			PCVOID Message
		);

	struct HV_POST_MESSAGE_INPUT
	{
		HV_CONNECTION_ID ConnectionId;
		uint32_t Reserved;
		HV_MESSAGE_TYPE MessageType;
		uint32_t PayloadSize;
		uint64_t Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	};

	//Code: 0x5D
	//0 - ConnectionId (4 bytes), FlagNumber (2 bytes), Reserved (2 bytes)
	HV_HYPERCALL_RESULT_VALUE
		HvSignalEvent(
			__in HV_CONNECTION_ID ConnectionId,
			__in UINT16 FlagNumber //relative to base number for port
		);

	uint32_t m_highestLeaf;
	std::string m_vendor;
	bool m_isPresent;

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

	enum VMBUS_SINT
	{
		VMBUS_MESSAGE_CONNECTION_ID = 1,
		VMBUS_MESSAGE_CONNECTION_ID_4 = 4,
		VMBUS_MESSAGE_PORT_ID = 1,
		VMBUS_EVENT_CONNECTION_ID = 2,
		VMBUS_EVENT_PORT_ID = 2,
		VMBUS_MONITOR_CONNECTION_ID = 3,
		VMBUS_MONITOR_PORT_ID = 3,
		VMBUS_MESSAGE_SINT = 2,
	};

#define VERSION_WS2008  ((0 << 16) | (13))
#define VERSION_WIN7    ((1 << 16) | (1))
#define VERSION_WIN8    ((2 << 16) | (4))
#define VERSION_WIN8_1    ((3 << 16) | (0))
#define VERSION_WIN10 ((4 << 16) | (0))
#define VERSION_WIN10_V4_1 ((4 << 16) | (1))
#define VERSION_WIN10_V5 ((5 << 16) | (0))
#define VERSION_WIN10_V5_1 ((5 << 16) | (1))
#define VERSION_WIN10_V5_2 ((5 << 16) | (2))

	//Vmbus
#pragma pack(push, 1)
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

	//Features
	bitset_32 m_featuresEax;//4.2.2 Partition Privilege Flags
	bitset_32 m_featuresEbx;
	bitset_32 m_featuresEdx;

	//Recommendations
	bitset_32 m_recommendationsEax;

	//Static State
	static KERNEL_PAGE_ALIGN volatile HV_REFERENCE_TSC_PAGE TscPage;

	//Per CPU State TODO
	static KERNEL_PAGE_ALIGN volatile HV_MESSAGE SynicMessages[HV_SYNIC_SINT_COUNT]; // - examine, copy to another location (work queue), set type to None, write EOI, and then process
	static KERNEL_PAGE_ALIGN volatile HV_SYNIC_EVENT_FLAGS SynicEvents[HV_SYNIC_SINT_COUNT];// examine flags, clear using LOCK AND or LOCK CMPXCHG, write EOI, process
	static KERNEL_PAGE_ALIGN volatile uint8_t HypercallPage[PAGE_SIZE];//Code page, map as execute
	static KERNEL_PAGE_ALIGN volatile uint8_t PostMessagePage[PAGE_SIZE];

	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage1[PAGE_SIZE];
	static KERNEL_PAGE_ALIGN volatile uint8_t MonitorPage2[PAGE_SIZE];

	//Synic Event Page
	//Message buffer

	Handle m_threadSignal;
	Handle m_connectSemaphore;
	std::list<HV_MESSAGE> m_queue;
	uint32_t m_msg_conn_id;

	//Port allocated from receivers memory pool - specifies virtual processor and SINTx (base flag number and flag count) to specify event flags
	//Connections are allocated from senders memory to a valid port. One-way communcation

	/**
	Hypercalls - simple and rep (repeat)
	repeat - rep count and rep start index. rep calls can be partially completed and index returns progress
	only from cpl of zero

	Specify physical address of input/output. 8 byte aligned. 
	RCX - HV_HYPERCALL_INPUT_VALUE
	RDX - input parameters (GPA if Fast flag is zero, or value if 1)
	R8 - output parameters (GPA if fast flag is zero, or value if 1)

	//Also can use xmm if supported - later
	*/
};

