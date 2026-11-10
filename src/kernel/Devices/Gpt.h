#pragma once

#include <cstddef>
#include <cstdint>
#include "kernel/Types.h"
#include "kernel/Devices/BlockDevice.h"

//The GUID partition table: the header in sector 1 and its array of entries, each checked by its CRC
namespace Gpt
{
	//Type GUIDs as they're stored (the first three fields little-endian, as guid_t has them on x64)
	static constexpr guid_t EfiSystem = { 0xC12A7328, 0xF81F, 0x11D2, { 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B } };
	static constexpr guid_t BasicData = { 0xEBD0A0A2, 0xB9E5, 0x4433, { 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 } };

	bool SameGuid(const guid_t& lhs, const guid_t& rhs);

	struct partition_t
	{
		guid_t Type;
		uint64_t FirstSector;
		uint64_t SectorCount;
		char Name[37]; //The UTF-16 name, as ASCII with '?' for the rest
	};

	//The partitions kept from a table; more than this are skipped
	static constexpr size_t MaxPartitions = 8;

	struct table_t
	{
		size_t Count;
		partition_t Partitions[MaxPartitions];
	};

	//False when the disk has no GPT, or its header or entries don't check out; only the primary table is read
	bool Read(BlockDevice& disk, table_t& table);
}
