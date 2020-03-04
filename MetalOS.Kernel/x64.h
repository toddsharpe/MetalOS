#pragma once

#include "MetalOS.Kernel.h"

//Helpers
extern "C"
{
	uint32_t x64_read_port(uint32_t port, uint8_t width);
	void x64_write_port(uint32_t port, uint32_t value, uint8_t width);
}

class x64
{
public:
	x64() = delete;
	static void Initialize();

private:
#define GDT_EMPTY 0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE 3
#define GDT_USER_DATA 4
#define GDT_TSS_ENTRY 5

#define UserDPL 3
#define KernelDPL 0
	enum IDT_GATE_TYPE
	{
		TaskGate32 = 0x5,
		InterruptGate16 = 0x6,
		TrapGate16 = 0x7,
		InterruptGate32 = 0xE,
		TrapGate32 = 0xF
	};

#pragma pack(push, 1)
	// Intel SDM Vol 3A Figure 3-6
	typedef struct _SEGMENT_SELECTOR
	{
		union
		{
			struct
			{
				uint16_t PrivilegeLevel : 2;
				uint16_t TableIndicator : 1; // 0 is GDT, 1 is LDT
				uint16_t Index : 13;
			};
			uint16_t Value;
		};

		_SEGMENT_SELECTOR() { };

		_SEGMENT_SELECTOR(uint16_t index)
		{
			PrivilegeLevel = KernelDPL;
			TableIndicator = 0;
			Index = index;
		}
	} SEGMENT_SELECTOR, * PSEGME;
	static_assert(sizeof(SEGMENT_SELECTOR) == sizeof(uint16_t), "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 3-8
	typedef struct _SEGMENT_DESCRIPTOR
	{
		union
		{
			struct
			{
				uint64_t SegmentLimit1 : 16;
				uint64_t BaseAddress1 : 16;
				uint64_t BaseAddress2 : 8;
				uint64_t Type : 4;
				uint64_t S : 1; // 1 if code/data, 0 if system segment
				uint64_t DPL : 2; //Descriptor Privilege Level
				uint64_t Present : 1;
				uint64_t SegmentLimit2 : 4;
				uint64_t Available : 1; // For use by OS
				uint64_t L : 1; //Should always be 0 for data
				uint64_t DB : 1;
				uint64_t Granulatiry : 1; // 0=1b-1mb, 1=4kb-4gb
				uint64_t BaseAddress3 : 8;
			};
			uint64_t Value;
		};
	} SEGMENT_DESCRIPTOR, * PSEGMENT_DESCRIPTOR;
	static_assert(sizeof(SEGMENT_DESCRIPTOR) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 7-4
	typedef struct _TSS_LDT_ENTRY
	{
		uint16_t SegmentLimit1;
		uint16_t BaseAddress1;

		uint16_t BaseAddress2 : 8;
		uint16_t Type : 4;
		uint16_t Zero1 : 1;
		uint16_t PrivilegeLevel : 2; // DPL
		uint16_t Present : 1;

		uint16_t Limit : 4;
		uint16_t Available : 1;
		uint16_t Zero2 : 1;
		uint16_t Zero3 : 1;
		uint16_t Granularity : 1;
		uint16_t BaseAddress3 : 8;

		uint32_t BaseAddress4;

		uint32_t Reserved1 : 8;
		uint32_t Zeros : 4;
		uint32_t Reserved2 : 20;
	} TSS_LDT_ENTRY, * PTSS_LDT_ENTRY;
	static_assert(sizeof(TSS_LDT_ENTRY) == 16, "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 7-11
	typedef struct _TASK_STATE_SEGMENT_64
	{
		uint32_t Reserved_0;
		//RSP for privilege levels 0-2
		uint32_t RSP_0_low;
		uint32_t RSP_0_high;
		uint32_t RSP_1_low;
		uint32_t RSP_1_high;
		uint32_t RSP_2_low;
		uint32_t RSP_2_high;
		uint32_t Reserved_1;
		uint32_t Reserved_2;
		//ISTs
		uint32_t IST_1_low;
		uint32_t IST_1_high;
		uint32_t IST_2_low;
		uint32_t IST_2_high;
		uint32_t IST_3_low;
		uint32_t IST_3_high;
		uint32_t IST_4_low;
		uint32_t IST_4_high;
		uint32_t IST_5_low;
		uint32_t IST_5_high;
		uint32_t IST_6_low;
		uint32_t IST_6_high;
		uint32_t IST_7_low;
		uint32_t IST_7_high;
		uint32_t Reserved_3;
		uint32_t Reserved_4;
		uint16_t Reserved_5;
		uint16_t IO_Map_Base;
	} TASK_STATE_SEGMENT_64, * PTASK_STATE_SEGMENT_64;
	static_assert(sizeof(TASK_STATE_SEGMENT_64) == 104, "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 6-7
	typedef struct _IDT_GATE
	{
		uint16_t Offset1;
		SEGMENT_SELECTOR SegmentSelector;
		uint16_t InterruptStackTable : 3;
		uint16_t Zeros : 5;
		uint16_t Type : 4;
		uint16_t Zero : 1;
		uint16_t PrivilegeLevel : 2; // DPL
		uint16_t Present : 1;
		uint16_t Offset2;
		uint32_t Offset3;
		uint32_t Reserved;

		_IDT_GATE() {}

		_IDT_GATE(uint64_t isrAddress)
		{
			Offset1 = (uint16_t)isrAddress;
			SegmentSelector.Value = 0;
			SegmentSelector.PrivilegeLevel = KernelDPL;
			SegmentSelector.Index = GDT_KERNEL_CODE;
			InterruptStackTable = 0;
			Zeros = 0;
			Type = IDT_GATE_TYPE::InterruptGate32;
			Zero = 0;
			PrivilegeLevel = KernelDPL;
			Present = true;
			Offset2 = (uint16_t)(isrAddress >> 16);
			Offset3 = (uint32_t)(isrAddress >> 32);
			Reserved = 0;
		}

		_IDT_GATE(uint64_t isrAddress, uint16_t stack, IDT_GATE_TYPE type)
		{
			Offset1 = (uint16_t)isrAddress;
			SegmentSelector.Value = 0;
			SegmentSelector.PrivilegeLevel = KernelDPL;
			SegmentSelector.Index = GDT_KERNEL_CODE;
			InterruptStackTable = stack;
			Zeros = 0;
			Type = type;
			Zero = 0;
			PrivilegeLevel = KernelDPL;
			Present = true;
			Offset2 = (uint16_t)(isrAddress >> 16);
			Offset3 = (uint32_t)(isrAddress >> 32);
			Reserved = 0;
		}
	} IDT_GATE, * PIDT_GATE;
	static_assert(sizeof(IDT_GATE) == 16, "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 3-11
	typedef struct _DESCRIPTOR_TABLE
	{
		uint16_t Limit;
		uint64_t BaseAddress;
	} DESCRIPTOR_TABLE, * PDESCRIPTOR_TABLE;
	static_assert(sizeof(DESCRIPTOR_TABLE) == 10, "Size mismatch, only 64-bit supported.");

	//Modern kernel has 5 GDTs (first has to be empty, plus 2x user and 2x kernel), plus the last entry is actually a TSS entry, mandatory.
	typedef struct _KERNEL_GDTS
	{
		SEGMENT_DESCRIPTOR Empty;
		SEGMENT_DESCRIPTOR KernelCode;
		SEGMENT_DESCRIPTOR KernelData;
		SEGMENT_DESCRIPTOR UserCode;
		SEGMENT_DESCRIPTOR UserData;
		TSS_LDT_ENTRY TssEntry;
	} KERNEL_GDTS, * PKERNEL_GDTS;
#pragma pack(pop)

	KERNEL_PAGE_ALIGN static volatile uint8_t DOUBLEFAULT_STACK[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile uint8_t NMI_Stack[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile uint8_t DEBUG_STACK[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile uint8_t MCE_STACK[IST_STACK_SIZE];

	KERNEL_GLOBAL_ALIGN static volatile TASK_STATE_SEGMENT_64 TSS64;
	KERNEL_GLOBAL_ALIGN static volatile KERNEL_GDTS KernelGDT;
	KERNEL_GLOBAL_ALIGN static volatile IDT_GATE IDT[IDT_COUNT];

	//Aligned on word boundary so address load is on correct boundary
	__declspec(align(2)) static DESCRIPTOR_TABLE GDTR;
	__declspec(align(2)) static DESCRIPTOR_TABLE IDTR;
};

