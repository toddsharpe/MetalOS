#include <intrin.h>
#include "Assert.h"

#include "HyperVTimer.h"
#include "HyperV.h"

HyperVTimer::HyperVTimer(const uint32_t timerId) :
	m_timerId(timerId),
	m_configRegister(MSR_REGS::HV_X64_MSR_STIMER0_CONFIG + timerId),
	m_countRegister(MSR_REGS::HV_X64_MSR_STIMER0_COUNT + timerId)
{

}

void HyperVTimer::Enable(const nano_t value, const uint8_t vector)
{
	Assert(m_timerId < NumTimers);
	
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

void HyperVTimer::Disable()
{
	HV_STIMER_CONFIG_REG config = { 0 };
	config.AsUint64 = __readmsr(m_configRegister);
	config.Enable = false;
	__writemsr(m_configRegister, config.AsUint64);
}

void HyperVTimer::Display() const
{
	const uint64_t config = __readmsr(m_configRegister);
	const uint64_t count = __readmsr(m_countRegister);
	Printf("Config: 0x%016x Count: 0x%016x\n", config, count);
}
