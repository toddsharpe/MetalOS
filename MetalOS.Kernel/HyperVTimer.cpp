#include "HyperVTimer.h"
#include "Main.h"
#include <intrin.h>
#include "HyperV.h"

HyperVTimer::HyperVTimer(uint32_t timerId) : m_timerId(timerId)
{
	HyperV hyperv;
	Assert(hyperv.IsPresent());
	Assert(hyperv.DirectSyntheticTimers());
}

uint64_t HyperVTimer::GetConfigRegister(uint32_t timer)
{
	Assert(timer < 4);
	HV_REG reg = (HV_REG)(HV_REG::HV_X64_MSR_STIMER0_CONFIG + timer);
	return __readmsr(reg);
}

uint64_t HyperVTimer::GetCountRegister(uint32_t timer)
{
	Assert(timer < 4);
	HV_REG reg = (HV_REG)(HV_REG::HV_X64_MSR_STIMER0_COUNT + timer);
	return __readmsr(reg);
}

void HyperVTimer::SetConfigRegister(uint32_t timer, uint64_t value)
{
	Assert(timer < 4);
	HV_REG reg = (HV_REG)(HV_REG::HV_X64_MSR_STIMER0_CONFIG + timer);
	__writemsr(reg, value);
}

void HyperVTimer::SetCountRegister(uint32_t timer, uint64_t value)
{
	Assert(timer < 4);
	HV_REG reg = (HV_REG)(HV_REG::HV_X64_MSR_STIMER0_COUNT + timer);
	__writemsr(reg, value);
}

//TODO: verify direct mode exists
void HyperVTimer::SetPeriodic(double seconds, uint8_t vector)
{
	uint64_t value = seconds * 1000000000 / 100;
	SetCountRegister(m_timerId, value);

	HV_STIMER_CONFIG_REG config = { 0 };
	config.Enable = true;
	config.AutoEnable = true;
	config.Periodic = true;
	config.ApicVector = vector;
	config.DirectMode = true;
	SetConfigRegister(m_timerId, config.Value);
}

void HyperVTimer::Display()
{
	uint64_t config = GetConfigRegister(m_timerId);
	uint64_t count = GetCountRegister(m_timerId);
	Print("Config: 0x%016x Count: 0x%016x\n", config, count);
}
