#pragma once

#include "msvc.h"
#include <string>
extern "C"
{
#include <acpi.h>
}
#include <vector>
#include <list>
#include "Driver.h"
#include "Device.h"

class AcpiDevice : public Device
{
public:
	AcpiDevice(ACPI_HANDLE object);

	void Initialize() override;
	const void* GetResource(uint32_t type) const override;
	void DisplayDetails() const override;

	const ACPI_HANDLE GetHandle() const
	{
		return m_acpiObject;
	}

private:
	static ACPI_STATUS AttachResource(ACPI_RESOURCE* Resource, void* Context);
	static ACPI_STATUS DisplayResource(const ACPI_RESOURCE& Resource);

	ACPI_HANDLE m_acpiObject;
	std::list<ACPI_RESOURCE> m_resources;
};

