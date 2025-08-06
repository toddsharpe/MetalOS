#include "kernel/HyperV/Timer.h"
#include "kernel/HyperV/HyperV.h"
#include "Assert.h"

// 12.5 Synthetic Timer MSRs
namespace HyperV::Timer
{
	void Enable(const nano_t value, const size_t timerId, const uint8_t irq)
	{
		Assert(timerId < NumTimers);
		const uint32_t countReg = static_cast<uint32_t>(MSR::HV_X64_MSR_STIMER0_COUNT) + timerId;
		const uint32_t configReg = static_cast<uint32_t>(MSR::HV_X64_MSR_STIMER0_CONFIG) + timerId;

		//12.4.1 Reference Counter MSR 
		const nano100_t value100 = value / 100;
		__writemsr(countReg, value100);

		TimerConfigReg config = { };
		config.Enable = true;
		config.AutoEnable = true;
		config.Periodic = true;
		config.ApicVector = irq;
		config.DirectMode = true;
		__writemsr(configReg, config.AsUint64);
	}

	void Disable(const size_t timerId)
	{
		Assert(timerId < NumTimers);
		const uint32_t configReg = static_cast<uint32_t>(MSR::HV_X64_MSR_STIMER0_CONFIG) + timerId;

		TimerConfigReg config = { };
		config.AsUint64 = __readmsr(configReg);
		config.Enable = false;
		__writemsr(configReg, config.AsUint64);
	}
};
