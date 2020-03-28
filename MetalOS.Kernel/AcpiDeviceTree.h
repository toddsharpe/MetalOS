#pragma once

#include "AcpiDevice.h"

class AcpiDeviceTree
{
public:
	AcpiDeviceTree();
	ACPI_STATUS Populate();
	void PrintTree() const;

	bool GetDeviceByHid(const std::string& hid, AcpiDevice** device);
	bool GetDeviceByName(const std::string& name, AcpiDevice** device);

private:
	static ACPI_STATUS AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue);

	AcpiDevice* m_root;
};

