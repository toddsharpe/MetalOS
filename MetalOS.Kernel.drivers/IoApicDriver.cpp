#include "IoApicDriver.h"

#include "AcpiDevice.h"
#include "Kernel.h"

IoApicDriver::IoApicDriver(Device& device) :
	Driver(device),
	m_apic()
{
	
}

Result IoApicDriver::Initialize()
{
	std::vector<paddr_t> addresses;

	AcpiDevice* device = (AcpiDevice*)&m_device;
	const ACPI_RESOURCE* resource = (const ACPI_RESOURCE*)device->GetResource(ACPI_RESOURCE_TYPE_FIXED_MEMORY32);
	addresses.push_back(resource->Data.FixedMemory32.Address);
	m_apic = (IoApic*)kernel.VirtualMap(nullptr, addresses, MemoryProtection::PageReadWrite);
	
	IdReg idReg = { 0 };
	idReg.AsUint32 = ReadReg(IoApicReg::ID);

	VersionReg versionReg = { 0 };
	versionReg.AsUint32 = ReadReg(IoApicReg::Version);

	this->m_device.Name = "IOAPIC";

	return Result::Success;
}

Result IoApicDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	return Result::NotImplemented;
}

Result IoApicDriver::Write(const char* buffer, size_t length)
{
	return Result::NotImplemented;
}

Result IoApicDriver::EnumerateChildren()
{
	return Result::NotImplemented;
}

Result IoApicDriver::MapInterrupt(InterruptVector vector, uint16_t Irq)
{
	RedirectionEntry entry = ReadEntry(Irq);
	entry.InterruptVector = static_cast<uint8_t>(vector);
	WriteEntry(Irq, entry);
	return Result::Success;
}

Result IoApicDriver::UnmaskInterrupt(uint16_t Irq)
{
	RedirectionEntry entry = ReadEntry(Irq);
	entry.InterruptMasked = 0;
	WriteEntry(Irq, entry);
	return Result::Success;
}

IoApicDriver::RedirectionEntry IoApicDriver::ReadEntry(int pin)
{
	RedirectionEntry entry;
	entry.Low = ReadReg(static_cast<IoApicReg>(IoApicReg::FirstEntry + 2 * pin));
	entry.High = ReadReg(static_cast<IoApicReg>(IoApicReg::FirstEntry + 1 + 2 * pin));
	return entry;
}

void IoApicDriver::WriteEntry(int pin, RedirectionEntry entry)
{
	WriteReg(static_cast<IoApicReg>(IoApicReg::FirstEntry + 2 * pin), entry.Low);
	WriteReg(static_cast<IoApicReg>(IoApicReg::FirstEntry + 1 + 2 * pin), entry.High);
}

void IoApicDriver::WriteReg(IoApicReg reg, const uint32_t value)
{
	m_apic->Index = static_cast<uint32_t>(reg);
	m_apic->Data = value;
}

uint32_t IoApicDriver::ReadReg(IoApicReg reg)
{
	m_apic->Index = static_cast<uint32_t>(reg);
	return m_apic->Data;
}
