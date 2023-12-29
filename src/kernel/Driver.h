#pragma once

#include <string>
#include "user/MetalOS.Types.h"
#include "MetalOS.Internal.h"
#include "Kernel/Objects/KFile.h"
#include <Graphics/Framebuffer.h>

#include <memory>

enum class Result
{
	Success,
	Failed,
	NotImplemented
};

class Device;
class Driver
{
public:
	Driver(Device& device);
	
	virtual Result Initialize() = 0;
	virtual Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) = 0;
	virtual Result Write(const char* buffer, size_t length) = 0;
	virtual Result EnumerateChildren() = 0;

protected:
	Device& m_device;
};
