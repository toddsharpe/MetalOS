#include "UartDriver.h"
#include "x64.h"
#include "Main.h"

UartDriver::UartDriver(AcpiDevice& device) : Driver(device)
{
	const ACPI_RESOURCE* resource = device.GetResource(ACPI_RESOURCE_TYPE_IO);
	Assert(resource->Data.Io.AddressLength == 8);
	m_port = resource->Data.Io.Minimum;
}

void UartDriver::Write(const char* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		Write(0, static_cast<uint8_t>(buffer[i]));
	}
}

void UartDriver::Write(const char* string)
{
	this->Write(string, strlen(string));
}

uint8_t UartDriver::Read(uint8_t offset)
{
	return x64_read_port(m_port + offset, std::numeric_limits<uint8_t>::digits);
}

void UartDriver::Write(uint8_t offset, uint8_t value)
{
	x64_write_port(m_port + offset, value, std::numeric_limits<uint8_t>::digits);
}

//Returns 0 on hyper-v?
uint16_t UartDriver::ReadDivisorLatch()
{
	uint8_t raw = this->Read(3);
	
	_LINE_CONTROL_REGISTER lcr;
	lcr.Value = raw;
	lcr.Fields.DivisorLatchAccess = true;
	this->Write(3, lcr.Value);

	uint8_t lsb = this->Read(0);
	uint8_t msb = this->Read(1);

	this->Write(3, raw);

	return (((uint16_t)msb) << 8) | lsb;
}
