#pragma once

#include <string>
#include <shared\MetalOS.Types.h>
#include <kernel\MetalOS.Internal.h>
#include <Objects/KFile.h>
#include <Graphics/FrameBuffer.h>

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
