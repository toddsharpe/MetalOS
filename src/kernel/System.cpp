#include "kernel/System.h"
#include "kernel/Arch.h"
#include "kernel/Core/KProcess.h"
#include "kernel/Core/Scheduler.h"
#include "kernel/Kernel.h"
#include "kernel/Plat.h"
#include "MetalOS.Loader.h"
#include "Efi.h"

static EFI_RUNTIME_SERVICES m_runtime(*BootParams.Runtime);

void KePauseSystem()
{
	ArchDisableInterrupts();
	Scheduler::Disable();
}

void KeResumeSystem()
{
	Scheduler::Enable();
	ArchEnableInterrupts();
}

milli_t KeGetTicks()
{
	const nano100_t tsc = PlatReadTime();
	return ToMilli(tsc);
}

nano_t KeGetNanoseconds()
{
	const nano100_t tsc = PlatReadTime();
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
	time.Milliseconds = static_cast<uint16_t>(efiTime.Nanosecond / 1000000);
	time.Nanoseconds = efiTime.Nanosecond;
}
