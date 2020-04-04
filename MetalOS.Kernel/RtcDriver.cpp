#include "RtcDriver.h"
#include "x64.h"
#include "Main.h"
#include "x64_support.h"

RtcDriver::RtcDriver(AcpiDevice& device) : Driver(device)
{
	const ACPI_RESOURCE* resource = device.GetResource(ACPI_RESOURCE_TYPE_IO);
	Assert(resource->Data.Io.AddressLength == 2);
	m_controlPort = resource->Data.Io.Minimum;
	m_ioPort = m_controlPort + 1;
}

void RtcDriver::Enable()
{
	cpu_flags_t flags = x64_disable_interrupts();

	//Set frequency to 2Hz
	_STATUS_REGISTER_A statusA;
	statusA.Value = this->ReadRegister(Register::StatusRegisterA);
	statusA.RateSelectionFreq = 0xF;
	this->WriteRegister(Register::StatusRegisterA, statusA.Value);

	//Enable interrupt
	_STATUS_REGISTER_B statusB;
	statusB.Value = this->ReadRegister(Register::StatusRegisterB);
	statusB.PeriodInterruptEnable = true;
	this->WriteRegister(Register::StatusRegisterB, statusB.Value);

	x64_restore_flags(flags);
}

uint8_t RtcDriver::ReadRegister(Register reg) const
{
	x64_write_port(m_controlPort, reg | DisableNMI, std::numeric_limits<uint8_t>::digits);
	return x64_read_port(m_ioPort, std::numeric_limits<uint8_t>::digits);
}

void RtcDriver::WriteRegister(Register reg, uint8_t value)
{
	x64_write_port(m_controlPort, reg | DisableNMI, std::numeric_limits<uint8_t>::digits);
	x64_write_port(m_ioPort, value, std::numeric_limits<uint8_t>::digits);
}

void RtcDriver::Display() const
{
	Print("RTC Driver:\n");
	Print("  Control: 0x%x\n", this->m_controlPort);
	Print("  IO: 0x%x\n", this->m_ioPort);
	Print("  StatusA: 0x%02x\n", this->ReadRegister(Register::StatusRegisterA));
	Print("  StatusB: 0x%02x\n", this->ReadRegister(Register::StatusRegisterB));
	Print("  StatusC: 0x%02x\n", this->ReadRegister(Register::StatusRegisterC));

}
