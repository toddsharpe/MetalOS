#pragma once
#include <cstdint>
#include "msvc.h"
#include <string>
#include <bitset>
#include <intrin.h>
#include "MetalOS.Kernel.h"

//TODO: separate into hypervplatform and hypervinfo
//platform is basically the hal
//hypervinfo will be like cpuid, just a light weight way to check features
class HyperV
{
public:
	HyperV();

	void Initialize();

	bool IsPresent()
	{
		return m_isPresent;
	}

	bool DirectSyntheticTimers() { return m_featuresEdx[19]; }

	bool AccessPartitionReferenceCounter() { return m_featuresEax[9]; }

	static void EOI()
	{
		__writemsr(HV_X64_MSR_EOI, 0);
	}

	nano100_t ReadTsc();
	uint64_t TscFreq()
	{
		return __readmsr(HV_X64_MSR_TSC_FREQUENCY);
	}

private:
	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	static const uint32_t HV_SYNIC_SINT_COUNT = 16;

	typedef uint64_t HV_SPA;
	typedef uint64_t HV_GPA;
	typedef uint64_t HV_GVA;
	typedef uint64_t HV_SYNIC_SINT_INDEX;
	typedef uint64_t HV_PARTITION_ID;
	typedef uint16_t HV_STATUS;
	typedef const void* PCVOID;
	typedef uint64_t UINT64;
	typedef uint32_t UINT32;
	typedef uint16_t UINT16;
	typedef uint8_t UINT8;

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
			uint64_t Value;
		};
	};

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

	struct HV_INTERRUPT_PAGE_REGISTER //SIEFP, SIMP
	{
		union
		{
			struct
			{
				uint64_t Enable : 1;
				uint64_t Reserved : 11;
				uint64_t BaseAddress : 52;//Low 12 bits assumed to be 0
			};
			uint64_t Value;
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
			uint64_t Value;
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

	enum HV_PORT_TYPE
	{
		HvPortTypeMessage = 1,
		HvPortTypeEvent = 2,
		HvPortTypeMonitor = 3
	};

	typedef union
	{
		UINT32 AsUint32;
		struct
		{
			UINT32 Id : 24;
			UINT32 Reserved : 8;
		};
	} HV_CONNECTION_ID;

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
	typedef struct
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
	} HV_MESSAGE_HEADER;
	typedef struct
	{
		HV_MESSAGE_HEADER Header;
		UINT64 Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	} HV_MESSAGE;

#define HV_EVENT_FLAGS_COUNT (256 * 8)
#define HV_EVENT_FLAGS_BYTE_COUNT 256
	typedef struct
	{
		UINT8 Flags[HV_EVENT_FLAGS_BYTE_COUNT];
	} HV_SYNIC_EVENT_FLAGS;

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

	//Each slot is for one interrupt source
	//SIEF page: 16-element array of 256-byte event flags
	//SIM page: 16-element array of 256-byte messages HV_MESSAGE


	HV_STATUS
		HvPostMessage(
			__in HV_CONNECTION_ID ConnectionId,
			__in HV_MESSAGE_TYPE MessageType,
			__in UINT32 PayloadSize,
			__in_ecount(PayloadSize)
			PCVOID Message
		);


	uint32_t m_highestLeaf;
	std::string m_vendor;
	bool m_isPresent;
	bitset_32 m_featuresEax;//4.2.2 Partition Privilege Flags
	bitset_32 m_featuresEbx;
	bitset_32 m_featuresEdx;

	//State
	static KERNEL_PAGE_ALIGN HV_REFERENCE_TSC_PAGE TscPage;
};

