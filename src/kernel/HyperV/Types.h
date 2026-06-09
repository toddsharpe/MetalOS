#pragma once

#include "core_crt/stdint.h"

// Hypervisor Top Level Functional Specification v6.0b.pdf

#define HYPERV_PAGE_ALIGN __declspec(align(Arch::PageSize))
#pragma pack(push,8)
namespace HyperV
{
	enum class CPUID_LEAF
	{
		Vendor = 0x40000000,
		Interface = 0x40000001,
		Version = 0x40000002,
		Features = 0x40000003,
		Recommendations = 0x40000004,
		Highest = 0x40000005
	};
	
	//19 Appendix C: Hypervisor Synthetic MSRs 
	enum class MSR
	{
		HV_X64_MSR_GUEST_OS_ID = 0x40000000,
		HV_X64_MSR_HYPERCALL = 0x40000001,
		HV_X64_MSR_VP_INDEX = 0x40000002,
		HV_X64_MSR_RESET = 0x40000003,
		HV_X64_MSR_VP_RUNTIME = 0x40000010,
		HV_X64_MSR_TIME_REF_COUNT = 0x40000020,
		HV_X64_MSR_REFERENCE_TSC = 0x40000021,
		HV_X64_MSR_TSC_FREQUENCY = 0x40000022, // Frequency of TSC in Hz
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

		// HyperV TLFS 11.8
		// Each Processor has its own copy of these
		SCONTROL = 0x40000080, // SynIC Control
		SVERSION = 0x40000081, // SynIC Version RO
		SIEFP = 0x40000082,	   // Interrupt Event Flags Page
		SIMP = 0x40000083,	   // Interrupt Message Page
		EOM = 0x40000084,	   // End of message (write-only trigger)
		SINT0 = 0x40000090,	   // Interrupt Source 0
		SINT1 = 0x40000091,	   // Interrupt Source 1
		SINT2 = 0x40000092,	   // Interrupt Source 2
		SINT3 = 0x40000093,	   // Interrupt Source 3
		SINT4 = 0x40000094,	   // Interrupt Source 4
		SINT5 = 0x40000095,	   // Interrupt Source 5
		SINT6 = 0x40000096,	   // Interrupt Source 6
		SINT7 = 0x40000097,	   // Interrupt Source 7
		SINT8 = 0x40000098,	   // Interrupt Source 8
		SINT9 = 0x40000099,	   // Interrupt Source 9
		SINT10 = 0x4000009A,   // Interrupt Source 10
		SINT11 = 0x4000009B,   // Interrupt Source 11
		SINT12 = 0x4000009C,   // Interrupt Source 12
		SINT13 = 0x4000009D,   // Interrupt Source 13
		SINT14 = 0x4000009E,   // Interrupt Source 14
		SINT15 = 0x4000009F,   // Interrupt Source 15
	};

	enum class GUEST_OS_TYPE : uint64_t
	{
		Linux = 0x1,
		FreeBSD = 0x2,
		Xen = 0x3,
		Illumos = 0x4,
	};

	//2.6.2 Encoding the Guest OS Identity MSR for Open Source Operating Systems
	struct GUEST_OS_ID_REG
	{
		union
		{
			struct
			{
				uint64_t BuildNumber : 16;
				uint64_t Version : 32;
				uint64_t OSID : 8;
				GUEST_OS_TYPE OSType : 7;
				uint64_t OpenSource : 1; // Set to 1
			};
			uint64_t Value;
		};
	};

	struct HYPERCALL_REG
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

	//hyperv_vmbus.h
	static constexpr uint32_t VMBUS_MESSAGE_CONNECTION_I = 1;
	static constexpr uint32_t VMBUS_MESSAGE_CONNECTION_ID_4 = 4;
	static constexpr uint32_t VMBUS_MESSAGE_PORT_ID = 1;
	static constexpr uint32_t VMBUS_EVENT_CONNECTION_I = 2;
	static constexpr uint32_t VMBUS_EVENT_PORT_ID = 2;
	static constexpr uint32_t VMBUS_MONITOR_CONNECTION_I = 3;
	static constexpr uint32_t VMBUS_MONITOR_PORT_ID = 3;
	static constexpr uint32_t VMBUS_MESSAGE_SINT = 2;

	static constexpr uint32_t HV_MESSAGE_SIZE = 256;
	static constexpr uint32_t HV_MESSAGE_PAYLOAD_BYTE_COUNT = 240;
	static constexpr uint32_t HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT = 30;
	static constexpr uint32_t MAX_SIZE_CHANNEL_MESSAGE = HV_MESSAGE_PAYLOAD_BYTE_COUNT;

	static constexpr uint32_t HV_SYNIC_SINT_COUNT = 16;
	static constexpr uint32_t HV_EVENT_FLAGS_COUNT = 256 * 8;
	static constexpr uint32_t HV_EVENT_FLAGS_BYTE_COUNT = 256;

	static constexpr uint32_t HV_MESSAGE_TYPE_HYPERVISOR_MASK = 0x80000000;

	//If messagepending, write to EOM
	struct HV_MESSAGE_FLAGS
	{
		uint8_t MessagePending : 1;
		uint8_t Reserved : 7;
	};

	enum class HV_MESSAGE_TYPE : uint32_t
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

	typedef uint64_t HV_PARTITION_ID;

	union HV_PORT_ID
	{
		struct
		{
			uint32_t Id : 24;
			uint32_t Reserved : 8;
		};
		uint32_t AsUint32;
	};
	struct HV_MESSAGE_HEADER
	{
		HV_MESSAGE_TYPE MessageType;
		uint16_t Reserved;
		HV_MESSAGE_FLAGS MessageFlags;
		uint8_t PayloadSize;
		union
		{
			uint64_t OriginationId;
			HV_PARTITION_ID Sender;
			HV_PORT_ID Port;
		};
	};

	

	struct HV_MESSAGE
	{
		HV_MESSAGE_HEADER Header;
		uint64_t Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	};
	static_assert((sizeof(HV_MESSAGE)* HV_SYNIC_SINT_COUNT) == Arch::PageSize, "Bad sim page");

	struct HV_SYNIC_EVENT_FLAGS
	{
		uint8_t Flags[HV_EVENT_FLAGS_BYTE_COUNT];
	};
	static_assert((sizeof(HV_SYNIC_EVENT_FLAGS)* HV_SYNIC_SINT_COUNT) == Arch::PageSize, "Bad event flags page");

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

	typedef uint64_t (*Hypercall)(uint64_t, uint64_t, uint64_t);

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
	#define HV_SUCCESS(x) (x == HyperV::HV_STATUS_SUCCESS)

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

	struct HV_POST_MESSAGE_INPUT
	{
		HV_CONNECTION_ID ConnectionId;
		uint32_t Reserved;
		HV_MESSAGE_TYPE MessageType;
		uint32_t PayloadSize;
		uint64_t Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	};

	//11.11 Inter-Partition Communication Interfaces 
	enum class HYPERCALL_CODE : uint64_t
	{
		HvPostMessage = 0x005C,
		HvSignalEvent = 0x005d,
	};

	//HV TLFS 3.7
	struct HV_HYPERCALL_INPUT_VALUE
	{
		union
		{
			struct
			{
				HYPERCALL_CODE CallCode : 16;
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
	static_assert(sizeof(HV_HYPERCALL_INPUT_VALUE) == sizeof(uint64_t), "HV_HYPERCALL_INPUT_VALUE incorrect");

	struct HV_SIGNAL_EVENT_INPUT
	{
		union
		{
			struct
			{
				HV_CONNECTION_ID ConnectionId;
				uint16_t Flag;
				uint16_t Reserved;
			};
			uint64_t AsUint64;
		};

	};
	static_assert(sizeof(HV_SIGNAL_EVENT_INPUT) == sizeof(uint64_t), "Invalid HV_SIGNAL_EVENT_INPUT");
}
#pragma pack(pop)
