#pragma once

#include "kernel/KDevice.h"
#include "Drivers/RamDrive.h"
#include "Lib/Math.h"

static constexpr char RamDriveHid[] = "RDRV";
class RamDriveDriver : public StorageDriver
{
public:

	RamDriveDriver(KDevice& device) : StorageDriver(device)
	{

	}

	bool Initialize() override
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
		StrCopy(m_device.Name, Str("RamDrive"));
		snprintf(m_device.Description, sizeof(m_device.Description), "Ram Drive - Files: %zu", m_ramDrive->FileCount());

		return true;
	}

	bool OpenFile(KFile& file, const char* const path, const KFileAccess access) const override
	{
		//Read-only drive
		if (access != KFileAccess::Read)
			return false;

		void* address;
		size_t length;
		const bool result = m_ramDrive->Open(path, address, length);
		if (!result)
			return false;
		
		file = KFile();
		file.Context = address;
		file.Length = length;
		file.Access = access;
		file.Position = 0;

		return true;
	}

	size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const override
	{
		//End of file (also covers empty files)
		if (handle.Position >= handle.Length)
			return 0;

		const size_t read = Min(bytesToRead, handle.Length - handle.Position);
		const void* source = MakePointer<void*>(handle.Context, handle.Position);
		memcpy(buffer, source, read);
		return read;
	}

private:
	RamDrive* m_ramDrive;
};

