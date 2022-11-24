#pragma once

#include <cstdint>
extern "C"
{
#include <acpi.h>
}
#include <list>
#include "Device.h"

class AcpiDevice : public Device
{
public:
	AcpiDevice(const ACPI_HANDLE object);

	void Initialize() override;
	const void* GetResource(uint32_t type) const override;
	void DisplayDetails() const override;

private:
	static ACPI_STATUS AttachResource(ACPI_RESOURCE* Resource, void* Context);
	static ACPI_STATUS DisplayResource(const ACPI_RESOURCE& Resource);

	const ACPI_HANDLE m_acpiObject;
	std::list<ACPI_RESOURCE> m_resources;
};

