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

class Driver;
class AcpiDevice
{
public:
	static ACPI_STATUS AttachResource(ACPI_RESOURCE* Resource, void* Context);
	static ACPI_STATUS DisplayResource(const ACPI_RESOURCE& Resource);

	AcpiDevice(ACPI_HANDLE object);
	virtual ACPI_STATUS Initialize();

	std::list<AcpiDevice*>& GetChildren()
	{
		return this->m_children;
	}

	const ACPI_HANDLE GetHandle() const
	{
		return m_acpiObject;
	}

	const std::string& GetName() const
	{
		return m_name;
	}

	const std::string& GetFullName() const
	{
		return m_fullName;
	}

	const std::string& GetHid() const
	{
		return m_hid;
	}

	const std::string& GetDescription() const
	{
		return m_description;
	}

	void SetDriver(Driver* driver)
	{
		m_driver = driver;
	}

	Driver* GetDriver()
	{
		return m_driver;
	}

	void Display() const;

	const ACPI_RESOURCE* GetResource(uint32_t type) const;

private:
	ACPI_HANDLE m_acpiObject;
	std::string m_fullName;
	std::string m_name;
	std::string m_hid;
	std::string m_description;
	std::list<AcpiDevice*> m_children;
	std::list<ACPI_RESOURCE> m_resources;

	Driver* m_driver;
};

