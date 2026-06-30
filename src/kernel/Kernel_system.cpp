#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern Scheduler m_scheduler;
extern EFI_RUNTIME_SERVICES m_runtime;

void KePauseSystem()
{
	ArchDisableInterrupts();
	m_scheduler.Enabled = false;
}

void KeResumeSystem()
{
	m_scheduler.Enabled = true;
	ArchEnableInterrupts();
}

milli_t KeGetTicks()
{
	const nano100_t tsc = HyperV::Tsc::ReadTsc();
	return ToMilli(tsc);
}

nano_t KeGetNanoseconds()
{
	const nano100_t tsc = HyperV::Tsc::ReadTsc();
	return tsc * 100;
}

void KeGetSystemTime(KSystemTime& time)
{
	EFI_TIME efiTime = {};
	m_runtime.GetTime(&efiTime, nullptr);

	time.Year = efiTime.Year;
	time.Month = efiTime.Month;
	time.Day = efiTime.Day;
	time.Hour = efiTime.Hour;
	time.Minute = efiTime.Minute;
	time.Second = efiTime.Second;
	time.Milliseconds = efiTime.Nanosecond / 1000;
	time.Nanoseconds = efiTime.Nanosecond;
}

bool KeIsValid(const void* address)
{
	return m_process.Space.IsValidPointer(address);
}
