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
	Print("virt: 0x%016x\n", virtualAddress);
	m_ramDrive = new RamDrive(virtualAddress, SIZE_TO_PAGES(RamDriveSize));

	m_device.Type = DeviceType::Harddrive;
	
	//Set properties
	this->m_device.Name = "RamDrive";

	char buffer[64] = { 0 };
	sprintf(buffer, "Ram Drive - Files: %d", m_ramDrive->FileCount());
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

FileHandle* RamDriveDriver::OpenFile(const std::string& path, GenericAccess access)
{
	Assert(access == GenericAccess::Read);
	
	void* address;
	size_t length;
	bool result = m_ramDrive->Open(path.c_str(), address, length);
	if (!result)
		return nullptr;
	
	FileHandle* handle = new FileHandle();
	handle->Context = address;
	handle->Length = length;
	handle->Access = access;
	handle->Position = 0;
	return handle;
}

size_t RamDriveDriver::ReadFile(FileHandle* handle, void* buffer, size_t bytesToRead)
{
	Assert(handle->Position < handle->Length);

	const size_t read = std::min(bytesToRead, handle->Length - handle->Position);
	const void* source = MakePtr(void*, handle->Context, handle->Position);
	memcpy(buffer, source, read);
	return read;
}
