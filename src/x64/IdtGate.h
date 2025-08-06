#pragma once

#include "x64/Types.h"

namespace x64
{
	enum IdtGateType
	{
		TaskGate32 = 0x5,
		InterruptGate16 = 0x6,
		TrapGate16 = 0x7,
		InterruptGate32 = 0xE,
		TrapGate32 = 0xF
	};
	
	// Intel SDM Vol 3A Figure 6-7
	struct IdtGate
	{
		uint16_t Offset1;
		SegmentSelector SegmentSelector;
		uint16_t InterruptStackTable : 3;
		uint16_t Zeros : 5;
		uint16_t Type : 4;
		uint16_t Zero : 1;
		uint16_t PrivilegeLevel : 2; // DPL
		uint16_t Present : 1;
		uint16_t Offset2;
		uint32_t Offset3;
		uint32_t Reserved;

		constexpr IdtGate() :
			Offset1(),
			SegmentSelector(),
			InterruptStackTable(),
			Zeros(),
			Type(),
			Zero(),
			PrivilegeLevel(),
			Present(),
			Offset2(),
			Offset3(),
			Reserved()
		{
			
		}

		constexpr IdtGate(const uint64_t isrAddress, const uint16_t stackIdx = 0, const uint16_t priv = KernelDpl) :
			Offset1(isrAddress),
			SegmentSelector(GdtIndex::KernelCode, priv),
			InterruptStackTable(stackIdx),
			Zeros(),
			Type(IdtGateType::InterruptGate32),
			Zero(),
			PrivilegeLevel(priv),
			Present(true),
			Offset2(isrAddress >> 16),
			Offset3(isrAddress >> 32),
			Reserved()
		{

		}
	};
	static_assert(sizeof(IdtGate) == 16, "Size mismatch, only 64-bit supported.");
}
