#pragma once

#include "Device.h"
#include "Driver.h"
#include "MicrosoftHyperV.h"
#include <RamDrive.h>

static const char* RamDriveHid = "RDRV";

class RamDriveDriver : public Driver
{
public:
	RamDriveDriver(Device& device);

	Result Initialize() override;
	Result Read(const char* buffer, size_t length) override;
	Result Write(const char* buffer, size_t length) override;
	Result EnumerateChildren() override;

private:
	RamDrive* m_ramDrive;
};

