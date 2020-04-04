#include "ProcessorDriver.h"
#include "Cpuid.h"
#include "Main.h"
#include <intrin.h>

ProcessorDriver::ProcessorDriver(AcpiDevice& device) : Driver(device)
{
	Cpuid cpu;
	Assert(cpu.IsIntel());
	Assert(cpu.Hypervisor());
	Assert(cpu.Apic());
	Assert(!cpu.X2Apic());


	_IA32_APIC_BASE_MSR msr = { 0 };
	msr.Value = __readmsr(IA32_APIC_BASE);
	Assert(msr.ApicEnabled);
}
