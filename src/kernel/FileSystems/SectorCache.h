#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include "kernel/Devices/BlockDevice.h"

//A few of a block device's sectors kept in memory, the one used longest ago replaced and a modified one written back then or at a flush; apart from its file system so a static one is all zero
class SectorCache
{
public:
	static constexpr size_t MaxSectorSize = 4096;
	static constexpr size_t Slots = 16;

	constexpr SectorCache() : m_device(), m_tick(), m_mirrorFirst(), m_mirrorCount(), m_mirrorStride(), m_mirrorCopies(), m_slots()
	{

	}

	void Attach(BlockDevice& device)
	{
		m_device = &device;
		m_mirrorCopies = 0;
		for (slot_t& slot : m_slots)
		{
			slot.Valid = false;
			slot.Dirty = false;
		}
	}

	//Sectors in [first, first + count) are written again stride, 2 * stride... sectors on, copies times
	void Mirror(const uint64_t first, const uint64_t count, const uint64_t stride, const uint32_t copies)
	{
		m_mirrorFirst = first;
		m_mirrorCount = count;
		m_mirrorStride = stride;
		m_mirrorCopies = copies;
	}

	//The sector, valid until the next call; null if the device can't read it, or can't take the modified sector it replaces
	const uint8_t* Get(const uint64_t sector)
	{
		slot_t* const slot = Find(sector);
		return slot ? slot->Data : nullptr;
	}

	//The sector to change in place; it's written back when replaced or flushed
	uint8_t* Modify(const uint64_t sector)
	{
		slot_t* const slot = Find(sector);
		if (!slot)
			return nullptr;

		slot->Dirty = true;
		return slot->Data;
	}

	//The sector to change in place, all zero rather than read, for one whose contents don't matter
	uint8_t* Claim(const uint64_t sector)
	{
		slot_t* const slot = Find(sector, false);
		if (!slot)
			return nullptr;

		memset(slot->Data, 0, sizeof(slot->Data));
		slot->Dirty = true;
		return slot->Data;
	}

	//Sectors written to the device directly: any copy held is out of date, modified or not
	void Discard(const uint64_t sector, const uint64_t count)
	{
		for (slot_t& slot : m_slots)
		{
			if (slot.Valid && slot.Sector >= sector && slot.Sector - sector < count)
			{
				slot.Valid = false;
				slot.Dirty = false;
			}
		}
	}

	//Every modified sector to the device; false if one couldn't be written
	bool Flush()
	{
		bool written = true;
		for (slot_t& slot : m_slots)
		{
			if (slot.Valid && slot.Dirty)
				written = WriteBack(slot) && written;
		}
		return written;
	}

private:
	struct slot_t
	{
		uint64_t Sector;
		uint64_t Used;
		bool Valid;
		bool Dirty;
		uint8_t Data[MaxSectorSize];
	};

	slot_t* Find(const uint64_t sector, const bool read = true)
	{
		slot_t* oldest = &m_slots[0];
		for (slot_t& slot : m_slots)
		{
			if (slot.Valid && slot.Sector == sector)
			{
				slot.Used = ++m_tick;
				return &slot;
			}

			if (!slot.Valid || (oldest->Valid && slot.Used < oldest->Used))
				oldest = &slot;
		}

		if (oldest->Valid && oldest->Dirty && !WriteBack(*oldest))
			return nullptr;

		oldest->Valid = false;
		if (read && !m_device->Read(sector, 1, oldest->Data))
			return nullptr;

		oldest->Sector = sector;
		oldest->Used = ++m_tick;
		oldest->Valid = true;
		oldest->Dirty = false;
		return oldest;
	}

	bool WriteBack(slot_t& slot)
	{
		if (!m_device->Write(slot.Sector, 1, slot.Data))
			return false;

		if (slot.Sector >= m_mirrorFirst && slot.Sector - m_mirrorFirst < m_mirrorCount)
		{
			for (uint32_t copy = 1; copy <= m_mirrorCopies; copy++)
			{
				if (!m_device->Write(slot.Sector + copy * m_mirrorStride, 1, slot.Data))
					return false;
			}
		}

		slot.Dirty = false;
		return true;
	}

	BlockDevice* m_device;
	uint64_t m_tick;
	uint64_t m_mirrorFirst;
	uint64_t m_mirrorCount;
	uint64_t m_mirrorStride;
	uint32_t m_mirrorCopies;
	slot_t m_slots[Slots];
};
