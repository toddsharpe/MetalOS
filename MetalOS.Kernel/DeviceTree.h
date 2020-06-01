#pragma once

#include "Device.h"
#include "Driver.h"
#include "AcpiDevice.h"

class DeviceTree
{
public:
	DeviceTree();

	void Populate();
	void EnumerateChildren();
	
	void Display() const;

	bool GetDeviceByHid(const std::string& hid, Device** device);
	bool GetDeviceByName(const std::string& name, Device** device);
	Device* GetDevice(const std::string path);

private:
	static ACPI_STATUS AddAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue);
	static void AttachDriver(Device& device);

	ACPI_STATUS PopulateAcpi();

	Device* m_root;
};

