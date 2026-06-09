#pragma once

#include "kernel/KDevice.h"
#include "Drivers/RamDrive.h"
#include "Lib/Math.h"

static constexpr CString RamDriveHid = "RDRV";
class RamDriveDriver : public StorageDriver
{
public:

	RamDriveDriver(KDevice& device) : StorageDriver(device)
	{

	}

	Result Initialize() override
	{
		//Map space
		const paddr_t address = (paddr_t)m_device.Context;
		Assert(address);
		constexpr size_t pages = x64::SizeToPages(RamDriveSize);
		const void* virtualAddress = KeVirtualMap(address, RamDriveSize);

		//Create underlying driver
		m_ramDrive = KeAlloc<RamDrive>(AllocType::Kernel, virtualAddress, pages);
		Assert(m_ramDrive);

		//Set properties
		m_device.Class = KDeviceClass::Storage;
		m_device.Name = KeCopy("RamDrive", AllocType::Kernel);

		char buffer[64] = { 0 };
		sprintf(buffer, "Ram Drive - Files: %zu", m_ramDrive->FileCount());
		m_device.Description = KeCopy(buffer, AllocType::Kernel);

		return Result::Success;
	}

	Result Enumerate() override
	{
		return Result::Success;
	}

	Result OpenFile(KFile& file, const char* const path, const KFileAccess access) const override
	{
		Assert(access == KFileAccess::Read);

		void* address;
		size_t length;
		const bool result = m_ramDrive->Open(path, address, length);
		if (!result)
			return Result::Failed;
		
		file = KFile();
		file.Context = address;
		file.Length = length;
		file.Access = access;
		file.Position = 0;

		return Result::Success;
	}

	size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const override
	{
		AssertOp(handle.Position, <, handle.Length);

		const size_t read = Min(bytesToRead, handle.Length - handle.Position);
		const void* source = MakePointer<void*>(handle.Context, handle.Position);
		memcpy(buffer, source, read);
		return read;
	}

private:
	RamDrive* m_ramDrive;
};

