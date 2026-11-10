#pragma once

#include "kernel/Types.h"
#include "kernel/Display.h"
#include "kernel/Devices/BlockDevice.h"

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

//A storage controller, whose disks are block devices
class StorageDriver : public Driver
{
public:
	StorageDriver(KDevice& device) : Driver(device)
	{

	}

	virtual BlockDevice* GetDisk(const size_t index) = 0; //Null past the last disk
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
