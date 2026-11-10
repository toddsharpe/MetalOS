#pragma once

#include "kernel/Devices/BlockDevice.h"
#include "Assert.h"

//A window onto a run of a disk's sectors, numbered from the run's start
class Partition : public BlockDevice
{
public:
	constexpr Partition() : m_disk(), m_first(), m_count()
	{

	}

	Partition(BlockDevice& disk, const uint64_t first, const uint64_t count) : m_disk(&disk), m_first(first), m_count(count)
	{
		Assert(count <= disk.SectorCount() && first <= disk.SectorCount() - count);
	}

	size_t SectorSize() const override
	{
		return m_disk->SectorSize();
	}

	uint64_t SectorCount() const override
	{
		return m_count;
	}

	bool Read(const uint64_t sector, const size_t count, void* const buffer) override
	{
		return InRange(sector, count) && m_disk->Read(m_first + sector, count, buffer);
	}

	bool Write(const uint64_t sector, const size_t count, const void* const buffer) override
	{
		return InRange(sector, count) && m_disk->Write(m_first + sector, count, buffer);
	}

	bool Flush() override
	{
		return m_disk->Flush();
	}

private:
	bool InRange(const uint64_t sector, const size_t count) const
	{
		return count <= m_count && sector <= m_count - count;
	}

	BlockDevice* m_disk;
	uint64_t m_first;
	uint64_t m_count;
};
