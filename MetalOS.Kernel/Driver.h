#pragma once

#include "Device.h"
#include <string>
#include "MetalOS.Kernel.h"

class Device;
class Driver
{
public:
	Driver(Device& device);
	
	virtual Result Initialize() = 0;
	virtual Result Read(const char* buffer, size_t length) = 0;
	virtual Result Write(const char* buffer, size_t length) = 0;
	virtual Result EnumerateChildren() = 0;

protected:
	Device& m_device;
};

