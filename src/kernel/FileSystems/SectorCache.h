#pragma once

#include <cstddef>
#include <cstdint>
#include "kernel/Devices/BlockDevice.h"

//A few of a block device's sectors kept in memory, the one used longest ago replaced; apart from its file system so a static one is all zero
class SectorCache
{
public:
	static constexpr size_t MaxSectorSize = 4096;
	static constexpr size_t Slots = 16;

	constexpr SectorCache() : m_device(), m_tick(), m_slots()
	{

	}

	void Attach(BlockDevice& device)
	{
		m_device = &device;
		for (slot_t& slot : m_slots)
			slot.Valid = false;
	}

	//The sector, valid until the next call; null if the device can't read it
	const uint8_t* Get(const uint64_t sector)
	{
		slot_t* oldest = &m_slots[0];
		for (slot_t& slot : m_slots)
		{
			if (slot.Valid && slot.Sector == sector)
			{
				slot.Used = ++m_tick;
				return slot.Data;
			}

			if (!slot.Valid || (oldest->Valid && slot.Used < oldest->Used))
				oldest = &slot;
		}

		oldest->Valid = false;
		if (!m_device->Read(sector, 1, oldest->Data))
			return nullptr;

		oldest->Sector = sector;
		oldest->Used = ++m_tick;
		oldest->Valid = true;
		return oldest->Data;
	}

private:
	struct slot_t
	{
		uint64_t Sector;
		uint64_t Used;
		bool Valid;
		uint8_t Data[MaxSectorSize];
	};

	BlockDevice* m_device;
	uint64_t m_tick;
	slot_t m_slots[Slots];
};
