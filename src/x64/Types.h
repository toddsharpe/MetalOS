#pragma once

#include "x64/x64.h"
#include "core_crt/stdint.h"

namespace x64
{
	enum class MSR : uint32_t
	{
		IA32_FS_BASE = 0xC0000100,
		IA32_GS_BASE = 0xC0000101,
		IA32_KERNELGS_BASE = 0xC0000102,

		//INTEL SDM Vol 3A 5-22. 5.8.8
		IA32_STAR = 0xC0000081, //IA32_STAR_REG
		IA32_LSTAR = 0xC0000082, //Target RIP
		IA32_FMASK = 0xC0000084, //IA32_FMASK_REG
		IA32_EFER = 0xC0000080,
	};

	enum class GdtIndex : uint16_t
	{
		Empty,
		KernelCode,
		KernelData,
		User32Code,
		UserData,
		UserCode,
		TssEntry,
	};

	//Syscall/Sysret requires specific orderings in GDT
	static_assert(
		(uint32_t)GdtIndex::KernelData == (uint32_t)GdtIndex::KernelCode + 1 &&
		(uint32_t)GdtIndex::UserData == (uint32_t)GdtIndex::User32Code + 1 &&
		(uint32_t)GdtIndex::UserCode == (uint32_t)GdtIndex::User32Code + 2,
		"Invalid kernel GDTs");

#pragma pack(push, 1)
	// Intel SDM Vol 3A 3.4.2 Figure 3-6
	struct SegmentSelector
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

		constexpr SegmentSelector() : Value() { };

		constexpr SegmentSelector(const GdtIndex index, const uint16_t level = KernelDpl) : Value()
		{
			PrivilegeLevel = level;
			TableIndicator = 0;
			Index = static_cast<uint16_t>(index);
		}
	};
	static_assert(sizeof(SegmentSelector) == sizeof(uint16_t), "Size mismatch, only 64-bit supported.");

	static constexpr uint64_t GDT_TYPE_RW = 0x02;
	static constexpr uint64_t GDT_TYPE_EX = 0x08;
	static constexpr uint64_t GDT_TYPE_RWX = GDT_TYPE_RW | GDT_TYPE_EX;

	// Intel SDM Vol 3A Figure 3-8
	struct SEGMENT_DESCRIPTOR
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
	};
	static_assert(sizeof(SEGMENT_DESCRIPTOR) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 7-4
	struct TSS_LDT_ENTRY
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
	};
	static_assert(sizeof(TSS_LDT_ENTRY) == 16, "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 7-11
	struct TASK_STATE_SEGMENT_64
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
	};
	static_assert(sizeof(TASK_STATE_SEGMENT_64) == 104, "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol 3A Figure 3-11
	struct DescriptorTable
	{
		constexpr DescriptorTable() :
			Limit(),
			BaseAddress()
		{

		}

		constexpr DescriptorTable(const uint16_t limit, const uint64_t BaseAddress) :
			Limit(limit),
			BaseAddress(BaseAddress)
		{
			
		}
		
		uint16_t Limit;
		uint64_t BaseAddress;
	};
	static_assert(sizeof(DescriptorTable) == 10, "Size mismatch, only 64-bit supported.");

	//Modern kernel has 5 GDTs (first has to be empty, plus 2x user and 2x kernel), plus the last entry is actually a TSS entry, mandatory.
	struct KERNEL_GDTS
	{
		SEGMENT_DESCRIPTOR Empty;
		SEGMENT_DESCRIPTOR KernelCode;
		SEGMENT_DESCRIPTOR KernelData;
		SEGMENT_DESCRIPTOR UserCode32;
		SEGMENT_DESCRIPTOR UserData;
		SEGMENT_DESCRIPTOR UserCode;
		TSS_LDT_ENTRY TssEntry;
	};

	struct IA32_FMASK_REG
	{
		union
		{
			struct
			{
				uint32_t EFlagsMask;
				uint32_t Reserved;
			};
			uint64_t AsUint64;
		};
	};

	struct IA32_STAR_REG
	{
		union
		{
			struct
			{
				uint64_t Reserved : 32;
				uint64_t SyscallCS : 16; //Adds 8 to this value to get SyscallSS
				uint64_t SysretCS : 16; //Adds 8 to this value to get SysretSS;
			};
			uint64_t AsUint64;
		};
	};
#pragma pack(pop)
}

