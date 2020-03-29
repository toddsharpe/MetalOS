#pragma once
#include "AcpiDevice.h"
#include <string>

class AcpiDevice;
class Driver
{
public:
	Driver(AcpiDevice& device);
	//virtual void Read(const char* buffer, )
	virtual void Write(const char* buffer, size_t length) = 0;

private:
	AcpiDevice& m_device;
};

