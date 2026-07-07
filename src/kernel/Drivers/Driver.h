#pragma once

#include "kernel/Types.h"

enum class Result
{
	Success,
	Failed,
	NotImplemented
};

class KDevice;
class Driver
{
public:
	Driver(KDevice& device) :
		m_device(device)
	{

	}

	virtual Result Initialize() = 0;
	virtual Result Enumerate() = 0;

protected:
	KDevice& m_device;
};

class IoDriver : public Driver
{
public:
	IoDriver(KDevice& device) : Driver(device)
	{

	}

	virtual Result Read(char* const buffer, size_t length, size_t* bytesRead = nullptr) = 0;
	virtual Result Write(const char* const buffer, const size_t length) = 0;
};

class StorageDriver : public Driver
{
public:
	StorageDriver(KDevice& device) : Driver(device)
	{

	}

	virtual Result OpenFile(KFile& file, const char* const path, const KFileAccess access) const = 0;
	virtual size_t ReadFile(const KFile& handle, void* const buffer, const size_t bytesToRead) const = 0;
};

class NicDriver : public Driver
{
public:
	NicDriver(KDevice& device) : Driver(device)
	{

	}

	virtual Result SendFrame(const void* const frame, const size_t length) = 0;
	virtual void GetMac(uint8_t out[6]) const = 0;
};
