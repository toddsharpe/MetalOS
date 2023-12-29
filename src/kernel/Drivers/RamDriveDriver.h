#pragma once

#include "Kernel/Devices/Device.h"
#include "Kernel/Driver.h"
#include "Kernel/Drivers/HardDriveDriver.h"

#include "RamDrive.h"
#include "Kernel/Objects/KFile.h"

#include <memory>

static const char* RamDriveHid = "RDRV";

class RamDriveDriver : public Driver, public HardDriveDriver
{
public:
	RamDriveDriver(Device& device);

	Result Initialize() override;
	Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

	Result OpenFile(KFile& file, const std::string& path, const GenericAccess access) const override;
	size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const override;

private:
	RamDrive* m_ramDrive;
};

