#include "RamDriveDriver.h"

#include "Kernel.h"
#include "Devices/SoftwareDevice.h"
#include <Assert.h>
#include <shared/MetalOS.System.h>

RamDriveDriver::RamDriveDriver(Device& device) : 
	Driver(device),
	m_ramDrive()
{

}

Result RamDriveDriver::Initialize()
{
	Printf("RamDriveDriver::Initialize\n");
	
	//Map space
	const paddr_t address = (paddr_t)this->m_device.GetResource((uint32_t)SoftwareDevice::ResourceType::Context);
	constexpr size_t pages = SizeToPages(RamDriveSize);
	const void* virtualAddress = kernel.DriverMapPages(address, pages);
	m_ramDrive = new RamDrive(virtualAddress, pages);

	m_device.Type = DeviceType::Harddrive;
	
	//Set properties
	this->m_device.Name = "RamDrive";

	char buffer[64] = { 0 };
	sprintf(buffer, "Ram Drive - Files: %zu", m_ramDrive->FileCount());
	this->m_device.Description = buffer;
	
	return Result::Success;
}

Result RamDriveDriver::Read(char* buffer, size_t length, size_t* bytesRead)
{
	return Result::NotImplemented;
}

Result RamDriveDriver::Write(const char* buffer, size_t length)
{
	return Result::NotImplemented;
}

Result RamDriveDriver::EnumerateChildren()
{
	return Result::NotImplemented;
}

Result RamDriveDriver::OpenFile(KFile& file, const std::string& path, const GenericAccess access) const
{
	Assert(access == GenericAccess::Read);
	
	void* address;
	size_t length;
	const bool result = m_ramDrive->Open(path.c_str(), address, length);
	if (!result)
		return Result::Failed;
	
	file = KFile();
	file.Context = address;
	file.Length = length;
	file.Access = access;
	file.Position = 0;
	return Result::Success;
}

size_t RamDriveDriver::ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const
{
	AssertOp(handle.Position, <, handle.Length);

	const size_t read = std::min(bytesToRead, handle.Length - handle.Position);
	const void* source = MakePointer<void*>(handle.Context, handle.Position);
	memcpy(buffer, source, read);
	return read;
}
