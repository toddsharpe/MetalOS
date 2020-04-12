#pragma once
#include <cstdint>

class HyperVTimer
{
public:
	HyperVTimer(uint32_t timer);
	void SetPeriodic(double seconds, uint8_t vector);

	void Display() const;

private:
	static const uint32_t NumTimers = 4;
	//Hypervisor TLFS 12.5
	enum MSR_REGS
	{
		HV_X64_MSR_STIMER0_CONFIG = 0x400000B0,
		HV_X64_MSR_STIMER0_COUNT = 0x400000B1,
	};
	//Counts: time value measured in 100 nanosecond units
	//One-shot: absolute expiration of time
	//Periodic: period of timer

	struct HV_STIMER_CONFIG_REG
	{
		union
		{
			struct
			{
				uint64_t Enable : 1;
				uint64_t Periodic : 1;
				uint64_t Lazy : 1;
				uint64_t AutoEnable : 1;
				uint64_t ApicVector : 8;
				uint64_t DirectMode : 1;
				uint64_t Reserved0 : 3;
				uint64_t Source : 4;
				uint64_t Reserved1 : 44;
			};
			uint64_t AsUint64;
		};
	};

private:
	uint32_t m_timerId;
	uint32_t m_configRegister;
	uint32_t m_countRegister;
};

