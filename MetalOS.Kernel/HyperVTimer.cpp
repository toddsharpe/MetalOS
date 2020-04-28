#include "HyperVTimer.h"
#include "Main.h"
#include <intrin.h>
#include "HyperV.h"

HyperVTimer::HyperVTimer(uint32_t timerId) :
	m_timerId(timerId),
	m_configRegister(MSR_REGS::HV_X64_MSR_STIMER0_CONFIG + timerId),
	m_countRegister(MSR_REGS::HV_X64_MSR_STIMER0_COUNT + timerId)
{
	Assert(timerId < NumTimers);
}

void HyperVTimer::SetPeriodic(nano_t value, uint8_t vector)
{
	const nano100_t value100 = value / 100;
	__writemsr(m_countRegister, value100);

	HV_STIMER_CONFIG_REG config = { 0 };
	config.Enable = true;
	config.AutoEnable = true;
	config.Periodic = true;
	config.ApicVector = vector;
	config.DirectMode = true;
	__writemsr(m_configRegister, config.AsUint64);
}

void HyperVTimer::Display() const
{
	const uint64_t config = __readmsr(m_configRegister);
	const uint64_t count = __readmsr(m_countRegister);
	Print("Config: 0x%016x Count: 0x%016x\n", config, count);
}
