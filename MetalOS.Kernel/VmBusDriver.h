#pragma once

#include "Driver.h"

class VmBusDriver : public Driver
{
public:
	VmBusDriver(AcpiDevice& device);
	void Write(const char* buffer, size_t length) override;

private:

};

