#include "kernel/Devices/Gpt.h"
#include "kernel/Devices/Partition.h"
#include "Lib/Crc32.h"
#include "MemoryDisk.h"

namespace
{
	static constexpr uint32_t GptTestEntries = 128;
	static constexpr uint32_t GptTestEntrySize = 128;

	struct GptTestPartition
	{
		guid_t Type;
		uint64_t First;
		uint64_t Last;
		const char* Name;
	};

	template <typename T>
	void GptTestPut(uint8_t* const at, const T value)
	{
		memcpy(at, &value, sizeof(T));
	}

	//Writes a primary GPT as DiscUtils lays one out: the header in sector 1, 128 entries from sector 2, both CRCs filled in
	template <size_t N>
	void GptTestWrite(MemoryDisk& disk, const GptTestPartition (&partitions)[N])
	{
		const size_t sectorSize = disk.SectorSize();
		const uint64_t entrySectors = (GptTestEntries * GptTestEntrySize + sectorSize - 1) / sectorSize;

		uint8_t* const entries = disk.Sector(2);
		memset(entries, 0, entrySectors * sectorSize);
		for (size_t i = 0; i < N; i++)
		{
			uint8_t* const entry = entries + i * GptTestEntrySize;
			memcpy(entry, &partitions[i].Type, sizeof(guid_t));
			GptTestPut<uint64_t>(entry + 32, partitions[i].First);
			GptTestPut<uint64_t>(entry + 40, partitions[i].Last);
			for (size_t c = 0; partitions[i].Name[c] != '\0'; c++)
				GptTestPut<uint16_t>(entry + 56 + 2 * c, static_cast<uint16_t>(partitions[i].Name[c]));
		}

		uint8_t* const header = disk.Sector(1);
		memset(header, 0, sectorSize);
		memcpy(header, "EFI PART", 8);
		GptTestPut<uint32_t>(header + 8, 0x00010000);
		GptTestPut<uint32_t>(header + 12, 92);
		GptTestPut<uint64_t>(header + 24, 1);
		GptTestPut<uint64_t>(header + 32, disk.SectorCount() - 1);
		GptTestPut<uint64_t>(header + 40, 2 + entrySectors);
		GptTestPut<uint64_t>(header + 48, disk.SectorCount() - 2 - entrySectors);
		GptTestPut<uint64_t>(header + 72, 2);
		GptTestPut<uint32_t>(header + 80, GptTestEntries);
		GptTestPut<uint32_t>(header + 84, GptTestEntrySize);
		GptTestPut<uint32_t>(header + 88, Crc32::Of(entries, GptTestEntries * GptTestEntrySize));
		GptTestPut<uint32_t>(header + 16, Crc32::Of(header, 92));
	}

	//Puts the header's CRC right again after a test changes a field in it
	void GptTestResealHeader(MemoryDisk& disk)
	{
		uint8_t* const header = disk.Sector(1);
		GptTestPut<uint32_t>(header + 16, 0);
		GptTestPut<uint32_t>(header + 16, Crc32::Of(header, 92));
	}

	//And the entries' CRC, then the header's that covers it
	void GptTestResealEntries(MemoryDisk& disk)
	{
		GptTestPut<uint32_t>(disk.Sector(1) + 88, Crc32::Of(disk.Sector(2), GptTestEntries * GptTestEntrySize));
		GptTestResealHeader(disk);
	}

	const GptTestPartition GptTestTwo[] =
	{
		{ Gpt::EfiSystem, 2048, 4095, "EFI system partition" },
		{ Gpt::BasicData, 4096, 8191, "MetalOS" },
	};
}

void Gpt_test()
{
	//A table with the boot partition and a data partition reads back whole
	{
		MemoryDisk disk(512, 16384);
		GptTestWrite(disk, GptTestTwo);

		Gpt::table_t table = {};
		Assert(Gpt::Read(disk, table));
		AssertEqual(table.Count, (size_t)2);
		Assert(Gpt::SameGuid(table.Partitions[0].Type, Gpt::EfiSystem));
		AssertEqual(table.Partitions[0].FirstSector, (uint64_t)2048);
		AssertEqual(table.Partitions[0].SectorCount, (uint64_t)2048);
		Assert(strcmp(table.Partitions[0].Name, "EFI system partition") == 0);
		Assert(Gpt::SameGuid(table.Partitions[1].Type, Gpt::BasicData));
		AssertEqual(table.Partitions[1].FirstSector, (uint64_t)4096);
		AssertEqual(table.Partitions[1].SectorCount, (uint64_t)4096);
		Assert(strcmp(table.Partitions[1].Name, "MetalOS") == 0);
	}

	//4096-byte sectors hold 32 entries each, and the table reads the same
	{
		MemoryDisk disk(4096, 4096);
		const GptTestPartition one[] = { { Gpt::EfiSystem, 256, 1023, "ESP" } };
		GptTestWrite(disk, one);

		Gpt::table_t table = {};
		Assert(Gpt::Read(disk, table));
		AssertEqual(table.Count, (size_t)1);
		AssertEqual(table.Partitions[0].SectorCount, (uint64_t)768);
	}

	//No signature, a header that fails its CRC, or entries that fail theirs: no table, and nothing kept
	{
		MemoryDisk disk(512, 16384);
		Gpt::table_t table = {};

		Assert(!Gpt::Read(disk, table));

		GptTestWrite(disk, GptTestTwo);
		disk.Sector(1)[40] ^= 1;
		Assert(!Gpt::Read(disk, table));

		GptTestWrite(disk, GptTestTwo);
		disk.Sector(2)[32] ^= 1;
		Assert(!Gpt::Read(disk, table));
		AssertEqual(table.Count, (size_t)0);
	}

	//Sound CRCs over unsound fields are still refused: an entry past the usable sectors, a header that isn't at sector 1, an entry size that splits sectors
	{
		MemoryDisk disk(512, 16384);
		Gpt::table_t table = {};

		GptTestWrite(disk, GptTestTwo);
		GptTestPut<uint64_t>(disk.Sector(2) + GptTestEntrySize + 40, disk.SectorCount() - 1);
		GptTestResealEntries(disk);
		Assert(!Gpt::Read(disk, table));

		GptTestWrite(disk, GptTestTwo);
		GptTestPut<uint64_t>(disk.Sector(1) + 24, 2);
		GptTestResealHeader(disk);
		Assert(!Gpt::Read(disk, table));

		GptTestWrite(disk, GptTestTwo);
		GptTestPut<uint32_t>(disk.Sector(1) + 84, 384);
		GptTestResealHeader(disk);
		Assert(!Gpt::Read(disk, table));
	}

	//Past eight partitions the rest are skipped, not refused
	{
		MemoryDisk disk(512, 16384);
		GptTestPartition ten[10] = {};
		for (size_t i = 0; i < 10; i++)
			ten[i] = { Gpt::BasicData, 2048 + i * 1024, 2048 + i * 1024 + 1023, "Data" };
		GptTestWrite(disk, ten);

		Gpt::table_t table = {};
		Assert(Gpt::Read(disk, table));
		AssertEqual(table.Count, Gpt::MaxPartitions);
		AssertEqual(table.Partitions[7].FirstSector, (uint64_t)(2048 + 7 * 1024));
	}

	//A partition is a window onto its sectors: numbered from its start, and nothing past its end
	{
		MemoryDisk disk(512, 64);
		for (uint64_t s = 0; s < disk.SectorCount(); s++)
			disk.Sector(s)[0] = static_cast<uint8_t>(s);

		Partition partition(disk, 10, 20);
		AssertEqual(partition.SectorCount(), (uint64_t)20);
		uint8_t sector[512] = {};
		Assert(partition.Read(0, 1, sector));
		AssertEqual(sector[0], (uint8_t)10);
		Assert(partition.Read(19, 1, sector));
		AssertEqual(sector[0], (uint8_t)29);
		Assert(!partition.Read(20, 1, sector));
		Assert(!partition.Read(19, 2, sector));

		sector[0] = 0xAA;
		Assert(partition.Write(5, 1, sector));
		AssertEqual(disk.Sector(15)[0], (uint8_t)0xAA);
		Assert(!partition.Write(20, 1, sector));
	}
}
