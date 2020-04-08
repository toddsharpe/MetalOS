#pragma once
#include <cstdint>

class HyperVTimer
{
public:
	HyperVTimer(uint32_t timer);
	void SetPeriodic(double seconds, uint8_t vector);

	void Display();

private:
	static const uint32_t NumTimers = 4;
	//Hypervisor TLFS 12.5
	enum HV_REG
	{
		HV_X64_MSR_STIMER0_CONFIG = 0x400000B0,
		HV_X64_MSR_STIMER0_COUNT = 0x400000B1,
		HV_X64_MSR_STIMER1_CONFIG = 0x400000B2,
		HV_X64_MSR_STIMER1_COUNT = 0x400000B3,
		HV_X64_MSR_STIMER2_CONFIG = 0x400000B4,
		HX_X64_MSR_STIMER2_COUNT = 0x400000B5,
		HV_X64_MSR_STIMER3_CONFIG = 0x400000B6,
		HV_X64_MSR_STIMER3_COUNT = 0x400000B7,
	};
	//Counts: time value measured in 100 nanosecond units
	//One-shot: absolute expiration of time
	//Periodic: period of timer

	static uint64_t GetConfigRegister(uint32_t timer);
	static void SetConfigRegister(uint32_t timer, uint64_t value);

	static uint64_t GetCountRegister(uint32_t timer);
	static void SetCountRegister(uint32_t timer, uint64_t value);

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
			uint64_t Value;
		};
	};

private:
	uint32_t m_timerId;
};

