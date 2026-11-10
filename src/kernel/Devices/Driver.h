#pragma once

#include "kernel/Types.h"
#include "kernel/Display.h"

class KDevice;
class Driver
{
public:
	Driver(KDevice& device) :
		m_device(device)
	{

	}

	virtual bool Initialize() = 0; //False if the device couldn't be brought up

protected:
	KDevice& m_device;
};

class NicDriver : public Driver
{
public:
	NicDriver(KDevice& device) : Driver(device)
	{

	}

	virtual bool SendFrame(const void* const frame, const size_t length) = 0;
	virtual void GetMac(uint8_t out[6]) const = 0;
};

class DisplayDriver : public Driver
{
public:
	DisplayDriver(KDevice& device) : Driver(device)
	{

	}

	virtual KFramebufferInfo GetFramebuffer() const = 0; //Physical descriptor of the current mode, for mapping into a process

	//Present: push the current framebuffer contents to the display (e.g. synthvid DIRT).
	virtual void Flush() = 0;
};
