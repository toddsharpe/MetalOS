#include "kernel/FileSystems/Fat32.h"
#include "kernel/FileSystems/SectorCache.h"
#include "MemoryDisk.h"
#include <cstdio>
#include <cstring>

namespace
{
	void FatTestPut16(uint8_t* const at, const uint32_t value)
	{
		const uint16_t narrow = static_cast<uint16_t>(value);
		memcpy(at, &narrow, sizeof(narrow));
	}

	void FatTestPut32(uint8_t* const at, const uint32_t value)
	{
		memcpy(at, &value, sizeof(value));
	}

	//A FAT32 volume laid out as Microsoft's spec has it, just big enough to count as FAT32: 512-byte sectors, a sector a cluster, 32 reserved sectors and two FATs
	class FatTestVolume
	{
	public:
		static constexpr uint32_t Sectors = 70000;
		static constexpr uint32_t Reserved = 32;
		static constexpr uint32_t FatSectors = 545;
		static constexpr uint32_t Root = 2;
		static constexpr uint32_t Eoc = 0x0FFFFFFF;

		FatTestVolume() : Disk(512, Sectors), m_next(Root + 1)
		{
			WriteBootSector(Disk.Sector(0), Sectors, FatSectors);
			SetFat(0, 0x0FFFFFF8);
			SetFat(1, Eoc);
			SetFat(Root, Eoc);
		}

		static void WriteBootSector(uint8_t* const boot, const uint32_t sectors, const uint32_t fatSectors)
		{
			boot[0] = 0xEB;
			boot[1] = 0x58;
			boot[2] = 0x90;
			memcpy(boot + 3, "MSWIN4.1", 8);
			FatTestPut16(boot + 11, 512);
			boot[13] = 1;
			FatTestPut16(boot + 14, Reserved);
			boot[16] = 2;
			boot[21] = 0xF8;
			FatTestPut32(boot + 32, sectors);
			FatTestPut32(boot + 36, fatSectors);
			FatTestPut32(boot + 44, Root);
			FatTestPut16(boot + 48, 1);
			FatTestPut16(boot + 50, 6);
			boot[66] = 0x29;
			memcpy(boot + 82, "FAT32   ", 8);
			boot[510] = 0x55;
			boot[511] = 0xAA;
		}

		void SetFat(const uint32_t cluster, const uint32_t value)
		{
			for (uint32_t fat = 0; fat < 2; fat++)
				FatTestPut32(Disk.Sector(Reserved + fat * FatSectors) + cluster * 4, value);
		}

		uint32_t GetFat(const uint32_t cluster)
		{
			uint32_t value;
			memcpy(&value, Disk.Sector(Reserved) + cluster * 4, sizeof(value));
			return value;
		}

		uint8_t* Cluster(const uint32_t cluster)
		{
			return Disk.Sector(Reserved + 2 * FatSectors + (cluster - 2));
		}

		//A file's data in a chain of clusters (each but the first a cluster apart when fragmented) and its entry; returns its first cluster
		uint32_t AddFile(const uint32_t directory, const char* const shortName, const char* const longName, const uint8_t* const data, const size_t size, const bool fragmented = false, const uint8_t checksumSkew = 0)
		{
			uint32_t first = 0;
			uint32_t previous = 0;
			for (size_t at = 0; at < size; at += 512)
			{
				if (fragmented && previous != 0)
					m_next++;
				const uint32_t cluster = m_next++;
				memcpy(Cluster(cluster), data + at, size - at < 512 ? size - at : 512);
				if (previous != 0)
					SetFat(previous, cluster);
				else
					first = cluster;
				previous = cluster;
			}
			if (previous != 0)
				SetFat(previous, Eoc);

			AddEntry(directory, shortName, longName, 0x20, first, static_cast<uint32_t>(size), checksumSkew);
			return first;
		}

		uint32_t AddDirectory(const uint32_t parent, const char* const shortName, const char* const longName)
		{
			const uint32_t cluster = m_next++;
			SetFat(cluster, Eoc);
			memset(Cluster(cluster), 0, 512);
			AddEntry(cluster, ".          ", nullptr, 0x10, cluster, 0);
			AddEntry(cluster, "..         ", nullptr, 0x10, parent == Root ? 0 : parent, 0);
			AddEntry(parent, shortName, longName, 0x10, cluster, 0);
			return cluster;
		}

		//The short entry, after its long name's entries (last part first) when it has one; shortName is the 11 padded 8.3 characters
		void AddEntry(const uint32_t directory, const char* const shortName, const char* const longName, const uint8_t attributes, const uint32_t cluster, const uint32_t size, const uint8_t checksumSkew = 0)
		{
			uint8_t entry[32] = {};
			memcpy(entry, shortName, 11);
			entry[11] = attributes;
			FatTestPut16(entry + 20, cluster >> 16);
			FatTestPut16(entry + 26, cluster & 0xFFFF);
			FatTestPut32(entry + 28, size);

			if (longName != nullptr)
			{
				uint8_t checksum = 0;
				for (size_t i = 0; i < 11; i++)
					checksum = static_cast<uint8_t>(((checksum & 1) << 7) + (checksum >> 1) + entry[i]);
				checksum = static_cast<uint8_t>(checksum + checksumSkew);

				static constexpr size_t offsets[13] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
				const size_t length = strlen(longName);
				const size_t parts = (length + 12) / 13;
				for (size_t part = parts; part >= 1; part--)
				{
					uint8_t lfn[32] = {};
					lfn[0] = static_cast<uint8_t>(part | (part == parts ? 0x40 : 0));
					lfn[11] = 0x0F;
					lfn[13] = checksum;
					for (size_t k = 0; k < 13; k++)
					{
						const size_t at = (part - 1) * 13 + k;
						const uint32_t unit = at < length ? static_cast<uint8_t>(longName[at]) : (at == length ? 0x0000 : 0xFFFF);
						FatTestPut16(lfn + offsets[k], unit);
					}
					memcpy(FreeSlot(directory), lfn, sizeof(lfn));
				}
			}
			memcpy(FreeSlot(directory), entry, sizeof(entry));
		}

		MemoryDisk Disk;

	private:
		//The directory's first unused entry, its chain grown when it's full
		uint8_t* FreeSlot(const uint32_t directory)
		{
			uint32_t cluster = directory;
			for (;;)
			{
				uint8_t* const data = Cluster(cluster);
				for (size_t offset = 0; offset < 512; offset += 32)
				{
					if (data[offset] == 0x00)
						return data + offset;
				}

				uint32_t next = GetFat(cluster);
				if (next >= 0x0FFFFFF8)
				{
					next = m_next++;
					SetFat(cluster, next);
					SetFat(next, Eoc);
					memset(Cluster(next), 0, 512);
				}
				cluster = next;
			}
		}

		uint32_t m_next;
	};

	//Reads the rest of an open file in chunks, as KeReadFile does
	size_t FatTestRead(Fat32& fs, KFile& file, uint8_t* const out, const size_t chunk)
	{
		size_t total = 0;
		for (size_t got; (got = fs.Read(file, out + total, chunk)) != 0;)
		{
			file.Position += got;
			total += got;
		}
		return total;
	}

	uint8_t FatTestData[8192];
	uint8_t FatTestOut[8192];
}

void Fat32_test()
{
	for (size_t i = 0; i < sizeof(FatTestData); i++)
		FatTestData[i] = static_cast<uint8_t>(i * 7 + 3);

	//The whole volume is built before it's mounted, so the cache never holds a stale sector
	FatTestVolume volume;
	volume.AddFile(FatTestVolume::Root, "README  TXT", nullptr, FatTestData, 100);
	volume.AddFile(FatTestVolume::Root, "METALO~1EXE", "MetalOS-WMSvr.exe", FatTestData, 3000);
	const uint32_t efi = volume.AddDirectory(FatTestVolume::Root, "EFI        ", nullptr);
	const uint32_t boot = volume.AddDirectory(efi, "BOOT       ", nullptr);
	volume.AddFile(boot, "TERM    EXE", "term.exe", FatTestData, 5000, true);
	volume.AddFile(boot, "EMPTY   TXT", nullptr, FatTestData, 0);
	volume.AddFile(FatTestVolume::Root, "\xE5ONE    TXT", nullptr, FatTestData, 10);
	volume.AddFile(FatTestVolume::Root, "BADSUM  TXT", "Bad Checksum.txt", FatTestData, 10, false, 1);
	const uint32_t cut = volume.AddFile(FatTestVolume::Root, "SHORT   BIN", nullptr, FatTestData, 2048);
	volume.SetFat(cut + 1, FatTestVolume::Eoc);
	const uint32_t many = volume.AddDirectory(FatTestVolume::Root, "MANY       ", nullptr);
	for (int i = 0; i < 40; i++)
	{
		char shortName[12];
		char longName[32];
		snprintf(shortName, sizeof(shortName), "FILE%04dDAT", i);
		snprintf(longName, sizeof(longName), "File number %02d.dat", i);
		volume.AddFile(many, shortName, longName, FatTestData + i, 1);
	}

	//A blank partition isn't FAT32, and nor is one with too few clusters, whatever its boot sector says
	{
		MemoryDisk blank(512, 1000);
		Fat32 none;
		SectorCache noneCache;
		Assert(!none.Mount(blank, noneCache));

		MemoryDisk small(512, 20000);
		FatTestVolume::WriteBootSector(small.Sector(0), 20000, 160);
		Assert(!none.Mount(small, noneCache));
	}

	Fat32 fs;
	SectorCache cache;
	Assert(fs.Mount(volume.Disk, cache));
	KFile file = {};

	//A short name in any case, from the root
	{
		Assert(fs.Open(file, "readme.txt", KFileAccess::Read));
		AssertEqual(file.Length, (size_t)100);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)100);
		Assert(memcmp(FatTestOut, FatTestData, 100) == 0);
		Assert(fs.Open(file, "/README.TXT", KFileAccess::Read));
		Assert(fs.Open(file, "/./readme.txt", KFileAccess::Read));
	}

	//A long name in any case, and its short alias
	{
		Assert(fs.Open(file, "metalos-wmsvr.EXE", KFileAccess::Read));
		AssertEqual(file.Length, (size_t)3000);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)3000);
		Assert(memcmp(FatTestOut, FatTestData, 3000) == 0);
		Assert(fs.Open(file, "METALO~1.EXE", KFileAccess::Read));
	}

	//A file in consecutive clusters reads its whole sectors in one request, however many clusters they span
	{
		Assert(fs.Open(file, "/metalos-wmsvr.exe", KFileAccess::Read));
		const size_t before = volume.Disk.Reads;
		AssertEqual(fs.Read(file, FatTestOut, 2560), (size_t)2560);
		AssertEqual(volume.Disk.Reads, before + 1);
		Assert(memcmp(FatTestOut, FatTestData, 2560) == 0);
	}

	//A fragmented file through nested directories reads the same in any chunk size
	{
		static constexpr size_t chunks[] = { 1, 7, 511, 512, 513, 4096 };
		for (const size_t chunk : chunks)
		{
			Assert(fs.Open(file, "\\EFI\\BOOT\\term.exe", KFileAccess::Read));
			memset(FatTestOut, 0, sizeof(FatTestOut));
			AssertEqual(FatTestRead(fs, file, FatTestOut, chunk), (size_t)5000);
			Assert(memcmp(FatTestOut, FatTestData, 5000) == 0);
		}
	}

	//A position moved forward or back reads from there
	{
		Assert(fs.Open(file, "/efi/boot/TERM.EXE", KFileAccess::Read));
		file.Position = 4000;
		AssertEqual(fs.Read(file, FatTestOut, 600), (size_t)600);
		Assert(memcmp(FatTestOut, FatTestData + 4000, 600) == 0);
		file.Position = 100;
		AssertEqual(fs.Read(file, FatTestOut, 50), (size_t)50);
		Assert(memcmp(FatTestOut, FatTestData + 100, 50) == 0);
	}

	//A path not from the root starts in the default directory; ".." climbs, to the root from its child
	{
		Assert(!fs.Open(file, "term.exe", KFileAccess::Read));
		Assert(fs.SetDirectory("/efi/boot"));
		Assert(fs.Open(file, "term.exe", KFileAccess::Read));
		Assert(fs.Open(file, "./TERM.EXE", KFileAccess::Read));
		Assert(fs.Open(file, "../BOOT/term.exe", KFileAccess::Read));
		Assert(fs.Open(file, "..\\..\\readme.txt", KFileAccess::Read));
		Assert(fs.Open(file, "/readme.txt", KFileAccess::Read));
		Assert(!fs.SetDirectory("/readme.txt"));
	}

	//Missing, a directory, a file used as a directory, a deleted entry, or open for writing: no file
	{
		Assert(!fs.Open(file, "/missing.txt", KFileAccess::Read));
		Assert(!fs.Open(file, "/EFI", KFileAccess::Read));
		Assert(!fs.Open(file, "/readme.txt/x", KFileAccess::Read));
		Assert(!fs.Open(file, "/gone.txt", KFileAccess::Read));
		Assert(!fs.Open(file, "/readme.txt", KFileAccess::Write));
	}

	//An empty file reads nothing, and so does a file at its end
	{
		Assert(fs.Open(file, "empty.txt", KFileAccess::Read));
		AssertEqual(file.Length, (size_t)0);
		AssertEqual(fs.Read(file, FatTestOut, 10), (size_t)0);

		Assert(fs.Open(file, "/readme.txt", KFileAccess::Read));
		file.Position = 100;
		AssertEqual(fs.Read(file, FatTestOut, 10), (size_t)0);
	}

	//A long name whose checksum doesn't tie it to its short entry is only the short name
	{
		Assert(!fs.Open(file, "/Bad Checksum.txt", KFileAccess::Read));
		Assert(fs.Open(file, "/BADSUM.TXT", KFileAccess::Read));
	}

	//A directory of many clusters is searched to its last entry
	{
		Assert(fs.Open(file, "/many/File number 39.dat", KFileAccess::Read));
		AssertEqual(fs.Read(file, FatTestOut, 1), (size_t)1);
		AssertEqual(FatTestOut[0], FatTestData[39]);
		Assert(fs.Open(file, "/MANY/FILE0000.DAT", KFileAccess::Read));
	}

	//A chain that ends before the file's size gives what it has
	{
		Assert(fs.Open(file, "/short.bin", KFileAccess::Read));
		AssertEqual(file.Length, (size_t)2048);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)1024);
	}
}
