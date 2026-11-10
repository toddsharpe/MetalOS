#include "kernel/Devices/Gpt.h"
#include "Lib/Crc32.h"
#include <cstring>

namespace
{
	static constexpr size_t MinSectorSize = 512;
	static constexpr size_t MaxSectorSize = 4096;

	static constexpr uint32_t HeaderSize = 92;
	static constexpr size_t HeaderCrcOffset = 16;
	static constexpr uint32_t MinEntrySize = 128;

	//A corrupt entry count can't send the read across the disk
	static constexpr uint64_t MaxEntriesBytes = 1024 * 1024;

	static constexpr size_t EntryNameOffset = 56;
	static constexpr size_t EntryNameLength = 36;

	template <typename T>
	T LittleEndian(const uint8_t* const bytes)
	{
		T value;
		memcpy(&value, bytes, sizeof(T));
		return value;
	}
}

namespace Gpt
{
	bool SameGuid(const guid_t& lhs, const guid_t& rhs)
	{
		return memcmp(&lhs, &rhs, sizeof(guid_t)) == 0;
	}

	bool Read(BlockDevice& disk, table_t& table)
	{
		table = {};
		const size_t sectorSize = disk.SectorSize();
		if (sectorSize < MinSectorSize || sectorSize > MaxSectorSize || disk.SectorCount() < 3)
			return false;

		uint8_t sector[MaxSectorSize];
		if (!disk.Read(1, 1, sector) || memcmp(sector, "EFI PART", 8) != 0)
			return false;

		//The header's CRC is over its own size with the CRC field taken as zero
		const uint32_t headerSize = LittleEndian<uint32_t>(sector + 12);
		const uint32_t headerCrc = LittleEndian<uint32_t>(sector + HeaderCrcOffset);
		if (headerSize < HeaderSize || headerSize > sectorSize)
			return false;
		memset(sector + HeaderCrcOffset, 0, sizeof(uint32_t));
		if (Crc32::Of(sector, headerSize) != headerCrc)
			return false;

		const uint64_t myLba = LittleEndian<uint64_t>(sector + 24);
		const uint64_t firstUsable = LittleEndian<uint64_t>(sector + 40);
		const uint64_t lastUsable = LittleEndian<uint64_t>(sector + 48);
		const uint64_t entriesLba = LittleEndian<uint64_t>(sector + 72);
		const uint32_t entryCount = LittleEndian<uint32_t>(sector + 80);
		const uint32_t entrySize = LittleEndian<uint32_t>(sector + 84);
		const uint32_t entriesCrc = LittleEndian<uint32_t>(sector + 88);
		const uint64_t entriesBytes = static_cast<uint64_t>(entryCount) * entrySize;
		if (myLba != 1 || firstUsable > lastUsable || lastUsable >= disk.SectorCount() ||
			entrySize < MinEntrySize || sectorSize % entrySize != 0 || entriesBytes > MaxEntriesBytes)
			return false;

		const uint64_t entrySectors = (entriesBytes + sectorSize - 1) / sectorSize;
		if (entriesLba < 2 || entrySectors > disk.SectorCount() || entriesLba > disk.SectorCount() - entrySectors)
			return false;

		//Each entry goes into the CRC as it's read; one that's used must lie in the usable sectors
		Crc32 crc;
		bool sound = true;
		uint32_t index = 0;
		for (uint64_t at = 0; at < entrySectors; at++)
		{
			if (!disk.Read(entriesLba + at, 1, sector))
				return false;

			for (size_t offset = 0; offset < sectorSize && index < entryCount; offset += entrySize, index++)
			{
				const uint8_t* const entry = sector + offset;
				crc.Add(entry, entrySize);

				guid_t type;
				memcpy(&type, entry, sizeof(guid_t));
				if (SameGuid(type, {}))
					continue;

				const uint64_t first = LittleEndian<uint64_t>(entry + 32);
				const uint64_t last = LittleEndian<uint64_t>(entry + 40);
				if (first < firstUsable || last > lastUsable || first > last)
				{
					sound = false;
					continue;
				}

				if (table.Count == MaxPartitions)
					continue;

				partition_t& partition = table.Partitions[table.Count++];
				partition.Type = type;
				partition.FirstSector = first;
				partition.SectorCount = last - first + 1;
				for (size_t i = 0; i < EntryNameLength; i++)
				{
					const uint16_t unit = LittleEndian<uint16_t>(entry + EntryNameOffset + 2 * i);
					if (unit == 0)
						break;
					partition.Name[i] = (unit >= 0x20 && unit < 0x7F) ? static_cast<char>(unit) : '?';
				}
			}
		}

		if (!sound || crc.Value() != entriesCrc)
		{
			table = {};
			return false;
		}
		return true;
	}
}
