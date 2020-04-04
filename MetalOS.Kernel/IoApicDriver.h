#pragma once
#include "Driver.h"

class IoApicDriver : public Driver
{
public:
	IoApicDriver(AcpiDevice& device);

	void Write(const char* buffer, size_t length)
	{

	}
};

