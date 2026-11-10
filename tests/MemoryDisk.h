#pragma once

#include "kernel/Devices/BlockDevice.h"
#include <cstring>

//A disk in memory for the storage tests, its sectors open to the test that builds or checks them
class MemoryDisk : public BlockDevice
{
public:
	MemoryDisk(const size_t sectorSize, const uint64_t sectorCount) :
		m_sectorSize(sectorSize),
		m_sectorCount(sectorCount),
		m_bytes(new uint8_t[sectorSize * sectorCount]())
	{

	}

	~MemoryDisk()
	{
		delete[] m_bytes;
	}

	MemoryDisk(const MemoryDisk&) = delete;
	MemoryDisk& operator=(const MemoryDisk&) = delete;

	uint8_t* Sector(const uint64_t sector)
	{
		return m_bytes + sector * m_sectorSize;
	}

	size_t SectorSize() const override
	{
		return m_sectorSize;
	}

	uint64_t SectorCount() const override
	{
		return m_sectorCount;
	}

	bool Read(const uint64_t sector, const size_t count, void* const buffer) override
	{
		if (count > m_sectorCount || sector > m_sectorCount - count)
			return false;
		Reads++;
		memcpy(buffer, Sector(sector), count * m_sectorSize);
		return true;
	}

	bool Write(const uint64_t sector, const size_t count, const void* const buffer) override
	{
		if (count > m_sectorCount || sector > m_sectorCount - count)
			return false;
		memcpy(Sector(sector), buffer, count * m_sectorSize);
		return true;
	}

	bool Flush() override
	{
		return true;
	}

	//Requests made, for a test that checks how a file system batches them
	size_t Reads = 0;

private:
	size_t m_sectorSize;
	uint64_t m_sectorCount;
	uint8_t* m_bytes;
};
