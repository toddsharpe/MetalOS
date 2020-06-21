#include "RamDriveDriver.h"
#include "Main.h"
#include "SoftwareDevice.h"
#include <crt_stdio.h>

RamDriveDriver::RamDriveDriver(Device& device) : 
	Driver(device),
	m_ramDrive()
{

}

Result RamDriveDriver::Initialize()
{
	//Map space
	paddr_t address = (paddr_t)this->m_device.GetResource((uint32_t)SoftwareDevice::ResourceType::Context);
	void* virtualAddress = kernel.DriverMapPages(address, SIZE_TO_PAGES(RamDriveSize));
	m_ramDrive = new RamDrive(virtualAddress, SIZE_TO_PAGES(RamDriveSize));
	
	//Set properties
	this->m_device.Name = "RamDrive";

	char buffer[64] = { 0 };
	crt_sprintf(buffer, "Ram Drive - Files: %d", m_ramDrive->FileCount());
	this->m_device.Description = buffer;
	
	return Result();
}

Result RamDriveDriver::Read(const char* buffer, size_t length)
{
	return Result();
}

Result RamDriveDriver::Write(const char* buffer, size_t length)
{
	return Result();
}

Result RamDriveDriver::EnumerateChildren()
{
	return Result();
}
