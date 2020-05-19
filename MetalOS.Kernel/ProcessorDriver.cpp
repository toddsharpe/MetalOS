#include "ProcessorDriver.h"
#include "Cpuid.h"
#include "Main.h"
#include <intrin.h>

//TODO: if apic -> create child device and attach local apic driver to it?
ProcessorDriver::ProcessorDriver(Device& device) : Driver(device)
{
	Cpuid cpu;
	Assert(cpu.IsIntel());
	Assert(cpu.IsHypervisor());
	Assert(cpu.Apic());
	Assert(!cpu.X2Apic());

	_IA32_APIC_BASE_MSR msr = { 0 };
	msr.Value = __readmsr(IA32_APIC_BASE);
	Assert(msr.ApicEnabled);
	m_apic = kernel.DriverMapIoSpace(msr.ApicBase << PAGE_SHIFT, 1);
	Print("APIC mapped to 0x%016x\n", m_apic);

	_TIMER_DIVIDE_CONFIG_REGISTER config = { 0 };
	config.DivideValue == _TIMER_DIVIDE::DivideBy128;
	WriteRegister(_LAPIC_REGISTERS::TimerDivideConfig, config.Value);
	
	_LVT_REGISTER timer = { 0 };
	timer.Vector = 0x40;
	timer.Mask = 1;
	WriteRegister(_LAPIC_REGISTERS::LVTTimer, timer.Value);

	WriteRegister(_LAPIC_REGISTERS::TimerInitialCount, 0x1000);
}

Result ProcessorDriver::Initialize()
{
	return Result::ResultNotImplemented;
}

Result ProcessorDriver::Read(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result ProcessorDriver::Write(const char* buffer, size_t length)
{
	return Result::ResultNotImplemented;
}

Result ProcessorDriver::EnumerateChildren()
{
	return Result::ResultNotImplemented;
}

void ProcessorDriver::WriteRegister(_LAPIC_REGISTERS reg, uint32_t value)
{

}

uint32_t ProcessorDriver::ReadRegister(_LAPIC_REGISTERS reg)
{
	return uint32_t();
}

void ProcessorDriver::Display()
{
	for (size_t i = 0x20; i <= TimerDivideConfig; i += 0x10)
	{
		uint32_t* p = MakePtr(uint32_t*, m_apic, i);
		Print("0x%04x = 0x%08x\n", i, *p);
	}
}
