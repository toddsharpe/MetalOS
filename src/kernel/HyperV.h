#pragma once


#include <cstdint>

#include <string>
#include <bitset>
#include <intrin.h>
#include <windows/types.h>
#include <linux\hyperv.h>
#include "MetalOS.Kernel.h"

#include <list>

//TODO: separate into hypervplatform and hypervinfo
//platform is basically the hal
//hypervinfo will be like cpuid, just a light weight way to check features
class HyperV
{
public:
	typedef uint64_t HV_SYNIC_SINT_INDEX;

	HyperV();

	void Initialize();

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

	static void SetSynicMessagePage(paddr_t address)
	{
		HV_INTERRUPT_PAGE_REGISTER reg;
		reg.AsUint64 = __readmsr(SIMP);
		reg.Enable = true;
		reg.BaseAddress = (address >> PageShift);

		__writemsr(SIMP, reg.AsUint64);
	}

	static void SetSynicEventPage(paddr_t address)
	{
		HV_INTERRUPT_PAGE_REGISTER reg;
		reg.AsUint64 = __readmsr(SIEFP);
		reg.Enable = true;
		reg.BaseAddress = (address >> PageShift);

		__writemsr(SIEFP, reg.AsUint64);
	}

	static void SetSintVector(uint32_t sint, uint32_t vector)
	{
		HV_SINT_REGISTER reg = { 0 };
		reg.AsUint64 = __readmsr(SINT0 + sint);
		reg.Vector = vector;
		reg.Masked = false;
		reg.AutoEOI = true;
		//reg.AutoEOI = !DeprecateAutoEOI(); //TODO
		__writemsr(SINT0 + sint, reg.AsUint64);
	}

	static void EnableSynic()
	{
		HV_SCONTROL_REGISTER reg;
		reg.AsUint64 = __readmsr(SCONTROl);
		reg.Enabled = true;
		__writemsr(SCONTROl, reg.AsUint64);
	}

	void WriteSintRegister(uint32_t index, uint64_t value)
	{
		//Assert(index < HV_SYNIC_SINT_COUNT);
		__writemsr(SINT0 + index, value);
	}

	static void SignalEom()
	{
		__writemsr(EOM, 0);
	}

	static void ProcessInterrupts(uint32_t sint, std::list<uint32_t>& channelIds, std::list<HV_MESSAGE>& queue);

	//Code: 0x005C
	//Params:
	//0 - ConnectionId (4 bytes), Padding (4 bytes) - from HvConnectPort
	//8 - MessageType (4 bytes), PayloadSize (4 bytes) - TopBit of MessageType is cleared. PayloadSize (up to 240 bytes)
	//16 Message[0] (8 bytes)
	//248 Message[29] (8 bytes)
	static HV_HYPERCALL_RESULT_VALUE
		HvPostMessage(
			__in HV_CONNECTION_ID ConnectionId,
			__in HV_MESSAGE_TYPE MessageType,
			__in UINT32 PayloadSize,
			__in_ecount(PayloadSize)
			PCVOID Message
		);

	//Code: 0x5D
	//0 - ConnectionId (4 bytes), FlagNumber (2 bytes), Reserved (2 bytes)
	static HV_HYPERCALL_RESULT_VALUE
		HvSignalEvent(
			__in HV_CONNECTION_ID ConnectionId,
			__in UINT16 FlagNumber //relative to base number for port
		);

private:
	typedef std::bitset<std::numeric_limits<uint32_t>::digits> bitset_32;

	//Each slot is for one interrupt source
	//SIEF page: 16-element array of 256-byte event flags (Synic event)
	//SIM page: 16-element array of 256-byte messages HV_MESSAGE (Synic message)
	static const uint32_t HV_SYNIC_SINT_COUNT = 16; // Corresponds to a single synthetic interrupt slot

	typedef uint64_t HV_SPA;
	typedef uint64_t HV_GPA;
	typedef uint64_t HV_GVA;

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

	enum HypercallCodes
	{
		PostMessage = 0x005C,
		SignalEvent = 0x005d,
	};

	struct HV_POST_MESSAGE_INPUT
	{
		HV_CONNECTION_ID ConnectionId;
		uint32_t Reserved;
		HV_MESSAGE_TYPE MessageType;
		uint32_t PayloadSize;
		uint64_t Payload[HV_MESSAGE_MAX_PAYLOAD_QWORD_COUNT];
	};

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

	uint32_t m_highestLeaf;
	std::string m_vendor;
	bool m_isPresent;

	//Features
	bitset_32 m_featuresEax;//4.2.2 Partition Privilege Flags
	bitset_32 m_featuresEbx;
	bitset_32 m_featuresEdx;

	//Recommendations
	bitset_32 m_recommendationsEax;

	//Static State
	static KERNEL_PAGE_ALIGN volatile HV_REFERENCE_TSC_PAGE TscPage;

	//Per CPU State TODO
	// examine, copy to another location (work queue), set type to None, write EOI, and then process
	static KERNEL_PAGE_ALIGN volatile HV_MESSAGE SynicMessages[HV_SYNIC_SINT_COUNT];
	// examine flags, clear using LOCK AND or LOCK CMPXCHG, write EOI, process
	static KERNEL_PAGE_ALIGN volatile HV_SYNIC_EVENT_FLAGS SynicEvents[HV_SYNIC_SINT_COUNT];
	static KERNEL_PAGE_ALIGN volatile uint8_t HypercallPage[PageSize];//Code page, map as execute
	static KERNEL_PAGE_ALIGN volatile uint8_t PostMessagePage[PageSize];

	//Synic Event Page
	//Message buffer

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

