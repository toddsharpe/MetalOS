#pragma once

#include "Device.h"
#include "Driver.h"
#include <RamDrive.h>

static const char* RamDriveHid = "RDRV";

class RamDriveDriver : public Driver, public virtual HardDriveDriver
{
public:
	RamDriveDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	KFile* OpenFile(const std::string& path, const GenericAccess access) const override;
	size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const override;

private:
	RamDrive* m_ramDrive;
};

