#pragma once

#include <string>
#include <shared\MetalOS.Types.h>
#include <kernel\MetalOS.Internal.h>
#include <KFile.h>
#include <Graphics/FrameBuffer.h>

//TODO(tsharpe): Refactor

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

class HardDriveDriver
{
public:
	virtual KFile* OpenFile(const std::string& path, const GenericAccess access) const = 0; //TODO: make KFile a param, return bool
	virtual size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const = 0;
};

class GraphicsDriver : public Driver
{
public:
	virtual Result Initialize() {}
	virtual Result Read(char* buffer, size_t length, size_t* bytesRead = nullptr) {}
	virtual Result Write(const char* buffer, size_t length) {}
	virtual Result EnumerateChildren() {}

	virtual bool Write(Graphics::FrameBuffer& framebuffer) = 0;
};
