#pragma once

#include "Lib/Time.h"

// 12.5 Synthetic Timer MSRs
namespace HyperV::Timer
{
	static constexpr size_t NumTimers = 4;

	//Counts: time value measured in 100 nanosecond units
	//One-shot: absolute expiration of time
	//Periodic: period of timer

	//12.5.1 Synthetic Timer Configuration Register
	struct TimerConfigReg
	{
		union
		{
			struct
			{
				uint64_t Enable : 1; //set if timer is enabled 
				uint64_t Periodic : 1;//set if timer is periodic, cleared if one-shot 
				uint64_t Lazy : 1;//set if timer is lazy, cleared if not 
				uint64_t AutoEnable : 1;//set if writing the corresponding counter implicitly causes the counter to be enabled, cleared if not 
				uint64_t ApicVector : 8;//controls the interrupt vector that is asserted upon timer expiration. See section 12.2 for further  context.
				uint64_t DirectMode : 1; // assert an interrupt upon timer expiration. See section 12.2 for further information.
				uint64_t Reserved0 : 3;
				uint64_t Source : 4; //synthetic interrupt source 
				uint64_t Reserved1 : 44;
			};
			uint64_t AsUint64;
		};
	};

	void Enable(const nano_t value, const size_t timerId, const uint8_t irq);
	void Disable(const size_t timerId);
};
