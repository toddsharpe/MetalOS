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
		static constexpr uint32_t Clusters = Sectors - Reserved - 2 * FatSectors;

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

		void WriteFsInfo(const uint32_t freeCount, const uint32_t nextFree)
		{
			uint8_t* const info = Disk.Sector(1);
			FatTestPut32(info, 0x41615252);
			FatTestPut32(info + 484, 0x61417272);
			FatTestPut32(info + 488, freeCount);
			FatTestPut32(info + 492, nextFree);
			FatTestPut32(info + 508, 0xAA550000);
		}

		uint32_t FsInfoFree()
		{
			uint32_t value;
			memcpy(&value, Disk.Sector(1) + 488, sizeof(value));
			return value;
		}

		//The short entry with the 11 padded 8.3 characters along the directory's chain, null if it isn't there; longName says whether long name entries tied to it by checksum come first
		uint8_t* FindEntry(const uint32_t directory, const char* const shortName, bool* const longName = nullptr)
		{
			bool tied = false;
			uint8_t sum = 0;
			for (uint32_t cluster = directory; cluster >= 2 && cluster < 0x0FFFFFF8; cluster = GetFat(cluster))
			{
				uint8_t* const data = Cluster(cluster);
				for (size_t offset = 0; offset < 512; offset += 32)
				{
					uint8_t* const entry = data + offset;
					if (entry[0] == 0xE5)
					{
						tied = false;
						continue;
					}
					if (entry[11] == 0x0F)
					{
						tied = true;
						sum = entry[13];
						continue;
					}

					if (memcmp(entry, shortName, 11) == 0)
					{
						if (longName)
							*longName = tied && sum == Checksum(entry);
						return entry;
					}
					tied = false;
				}
			}
			return nullptr;
		}

		static uint8_t Checksum(const uint8_t* const entry)
		{
			uint8_t checksum = 0;
			for (size_t i = 0; i < 11; i++)
				checksum = static_cast<uint8_t>(((checksum & 1) << 7) + (checksum >> 1) + entry[i]);
			return checksum;
		}

		static uint32_t EntryCluster(const uint8_t* const entry)
		{
			uint16_t high;
			uint16_t low;
			memcpy(&high, entry + 20, sizeof(high));
			memcpy(&low, entry + 26, sizeof(low));
			return (static_cast<uint32_t>(high) << 16) | low;
		}

		//Clusters the FAT says are free, to check the free count against
		uint32_t CountFree()
		{
			uint32_t free = 0;
			for (uint32_t cluster = 2; cluster < Clusters + 2; cluster++)
				free += (GetFat(cluster) & 0x0FFFFFFF) == 0 ? 1 : 0;
			return free;
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
				const uint8_t checksum = static_cast<uint8_t>(Checksum(entry) + checksumSkew);

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

	//Writes in chunks, moving the position as KeWriteFile does
	size_t FatTestWrite(Fat32& fs, KFile& file, const uint8_t* const data, const size_t size, const size_t chunk)
	{
		size_t total = 0;
		while (total < size)
		{
			const size_t got = fs.Write(file, data + total, size - total < chunk ? size - total : chunk);
			if (got == 0)
				break;
			file.Position += got;
			total += got;
		}
		return total;
	}

	//Closes what the test had open, as closing its handle would
	void FatTestClose(Fat32& fs, KFile& file)
	{
		if (file.Access != KFileAccess::None)
			fs.Close(file);
		file = {};
	}

	bool FatTestOpen(Fat32& fs, KFile& file, const char* const path, const KFileAccess access, const KFileCreate create)
	{
		FatTestClose(fs, file);
		return fs.Open(file, path, access, create);
	}

	//The whole of a file, read from its start
	bool FatTestSame(Fat32& fs, const char* const path, const uint8_t* const expected, const size_t size)
	{
		static uint8_t out[8192];
		KFile file = {};
		if (!FatTestOpen(fs, file, path, KFileAccess::Read, KFileCreate::OpenExisting))
			return false;

		const bool same = file.Length == size && FatTestRead(fs, file, out, 777) == size && memcmp(out, expected, size) == 0;
		FatTestClose(fs, file);
		return same;
	}

	uint8_t FatTestData[8192];
	uint8_t FatTestOut[8192];
	uint8_t FatTestExpect[8192];
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
		Assert(FatTestOpen(fs, file, "readme.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(file.Length, (size_t)100);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)100);
		Assert(memcmp(FatTestOut, FatTestData, 100) == 0);
		Assert(FatTestOpen(fs, file, "/README.TXT", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "/./readme.txt", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//A long name in any case, and its short alias
	{
		Assert(FatTestOpen(fs, file, "metalos-wmsvr.EXE", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(file.Length, (size_t)3000);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)3000);
		Assert(memcmp(FatTestOut, FatTestData, 3000) == 0);
		Assert(FatTestOpen(fs, file, "METALO~1.EXE", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//A file in consecutive clusters reads its whole sectors in one request, however many clusters they span
	{
		Assert(FatTestOpen(fs, file, "/metalos-wmsvr.exe", KFileAccess::Read, KFileCreate::OpenExisting));
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
			Assert(FatTestOpen(fs, file, "\\EFI\\BOOT\\term.exe", KFileAccess::Read, KFileCreate::OpenExisting));
			memset(FatTestOut, 0, sizeof(FatTestOut));
			AssertEqual(FatTestRead(fs, file, FatTestOut, chunk), (size_t)5000);
			Assert(memcmp(FatTestOut, FatTestData, 5000) == 0);
		}
	}

	//A position moved forward or back reads from there
	{
		Assert(FatTestOpen(fs, file, "/efi/boot/TERM.EXE", KFileAccess::Read, KFileCreate::OpenExisting));
		file.Position = 4000;
		AssertEqual(fs.Read(file, FatTestOut, 600), (size_t)600);
		Assert(memcmp(FatTestOut, FatTestData + 4000, 600) == 0);
		file.Position = 100;
		AssertEqual(fs.Read(file, FatTestOut, 50), (size_t)50);
		Assert(memcmp(FatTestOut, FatTestData + 100, 50) == 0);
	}

	//A path not from the root starts in the default directory; ".." climbs, to the root from its child
	{
		Assert(!FatTestOpen(fs, file, "term.exe", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(fs.SetDirectory("/efi/boot"));
		Assert(FatTestOpen(fs, file, "term.exe", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "./TERM.EXE", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "../BOOT/term.exe", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "..\\..\\readme.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "/readme.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!fs.SetDirectory("/readme.txt"));
	}

	//Missing, a directory, a file used as a directory, or a deleted entry: no file
	{
		Assert(!FatTestOpen(fs, file, "/missing.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/EFI", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/readme.txt/x", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/gone.txt", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//An empty file reads nothing, and so does a file at its end
	{
		Assert(FatTestOpen(fs, file, "empty.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(file.Length, (size_t)0);
		AssertEqual(fs.Read(file, FatTestOut, 10), (size_t)0);

		Assert(FatTestOpen(fs, file, "/readme.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		file.Position = 100;
		AssertEqual(fs.Read(file, FatTestOut, 10), (size_t)0);
	}

	//A long name whose checksum doesn't tie it to its short entry is only the short name
	{
		Assert(!FatTestOpen(fs, file, "/Bad Checksum.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "/BADSUM.TXT", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//A directory of many clusters is searched to its last entry
	{
		Assert(FatTestOpen(fs, file, "/many/File number 39.dat", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(fs.Read(file, FatTestOut, 1), (size_t)1);
		AssertEqual(FatTestOut[0], FatTestData[39]);
		Assert(FatTestOpen(fs, file, "/MANY/FILE0000.DAT", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//A chain that ends before the file's size gives what it has
	{
		Assert(FatTestOpen(fs, file, "/short.bin", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(file.Length, (size_t)2048);
		AssertEqual(FatTestRead(fs, file, FatTestOut, 4096), (size_t)1024);
	}

	//A volume whose FSInfo is missing has no free count
	AssertEqual(fs.FreeClusters(), Fat32::Unknown);
}

void Fat32Write_test()
{
	for (size_t i = 0; i < sizeof(FatTestData); i++)
		FatTestData[i] = static_cast<uint8_t>(i * 7 + 3);

	//FSInfo's hint points at a cluster in use, to be passed over
	FatTestVolume volume;
	volume.AddFile(FatTestVolume::Root, "OLD     TXT", nullptr, FatTestData, 1000);
	volume.AddEntry(FatTestVolume::Root, "LOCKED  TXT", nullptr, 0x21, 0, 0);
	volume.WriteFsInfo(FatTestVolume::Clusters - 3, 3);

	Fat32 fs;
	SectorCache cache;
	Assert(fs.Mount(volume.Disk, cache));
	AssertEqual(fs.FreeClusters(), FatTestVolume::Clusters - 3);
	KFile file = {};

	//Only a write open makes a file, and only with a disposition that allows it
	{
		Assert(!FatTestOpen(fs, file, "/new.bin", KFileAccess::ReadWrite, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/new.bin", KFileAccess::Read, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/new.bin", KFileAccess::Read, KFileCreate::OpenAlways));
		Assert(FatTestOpen(fs, file, "/new.bin", KFileAccess::ReadWrite, KFileCreate::CreateAlways));
		AssertEqual(file.Length, (size_t)0);
	}

	//A file written in pieces reads back the same, in one run of new clusters, and so does the volume mounted again
	{
		AssertEqual(FatTestWrite(fs, file, FatTestData, 5000, 700), (size_t)5000);
		AssertEqual(file.Length, (size_t)5000);
		AssertEqual(file.Run, (uint32_t)10);
		AssertEqual(fs.FreeClusters(), FatTestVolume::Clusters - 13);
		Assert(FatTestSame(fs, "new.bin", FatTestData, 5000));

		Fat32 again;
		SectorCache againCache;
		Assert(again.Mount(volume.Disk, againCache));
		AssertEqual(again.FreeClusters(), FatTestVolume::Clusters - 13);
		Assert(FatTestSame(again, "/NEW.BIN", FatTestData, 5000));
	}

	//A read open doesn't write, and nor does a read-only file open for writing
	{
		Assert(FatTestOpen(fs, file, "/new.bin", KFileAccess::Read, KFileCreate::OpenExisting));
		AssertEqual(fs.Write(file, FatTestData, 10), (size_t)0);
		Assert(FatTestOpen(fs, file, "/locked.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/locked.txt", KFileAccess::Write, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/locked.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(FatTestOpen(fs, file, "/locked.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!fs.Delete("/locked.txt"));
		FatTestClose(fs, file);
		Assert(!fs.Delete("/locked.txt"));
	}

	//Writing over the middle changes only those bytes; past the end the file grows, the gap reading as zeros
	{
		Assert(FatTestOpen(fs, file, "/new.bin", KFileAccess::ReadWrite, KFileCreate::OpenAlways));
		AssertEqual(file.Length, (size_t)5000);
		file.Position = 1000;
		AssertEqual(FatTestWrite(fs, file, FatTestData + 100, 1500, 1500), (size_t)1500);
		file.Position = 7000;
		AssertEqual(FatTestWrite(fs, file, FatTestData, 10, 10), (size_t)10);
		AssertEqual(file.Length, (size_t)7010);

		memcpy(FatTestExpect, FatTestData, 5000);
		memcpy(FatTestExpect + 1000, FatTestData + 100, 1500);
		memset(FatTestExpect + 5000, 0, 2000);
		memcpy(FatTestExpect + 7000, FatTestData, 10);
		Assert(FatTestSame(fs, "/new.bin", FatTestExpect, 7010));
	}

	//Opening to create over a file empties it, its clusters freed
	{
		const uint32_t free = fs.FreeClusters();
		Assert(FatTestOpen(fs, file, "/new.bin", KFileAccess::Write, KFileCreate::CreateAlways));
		AssertEqual(file.Length, (size_t)0);
		AssertEqual(fs.FreeClusters(), free + 14);
		Assert(FatTestSame(fs, "/new.bin", FatTestData, 0));
	}

	//Any number read a file but one writes, and a file anyone has open isn't emptied, deleted or renamed until the last closes it
	{
		KFile reader = {};
		KFile other = {};
		KFile writer = {};
		Assert(FatTestOpen(fs, reader, "/old.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, other, "/OLD.TXT", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, writer, "/old.txt", KFileAccess::ReadWrite, KFileCreate::OpenAlways));
		Assert(!FatTestOpen(fs, file, "/old.txt", KFileAccess::Write, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/old.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!fs.Delete("/old.txt"));
		Assert(!fs.Rename("/old.txt", "/older.txt"));

		FatTestClose(fs, writer);
		Assert(FatTestOpen(fs, writer, "/old.txt", KFileAccess::Write, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/old.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		FatTestClose(fs, writer);
		FatTestClose(fs, reader);
		Assert(!fs.Delete("/old.txt"));
		FatTestClose(fs, other);
		Assert(fs.Rename("/old.txt", "/older.txt"));
		Assert(fs.Rename("/older.txt", "/old.txt"));
		Assert(FatTestSame(fs, "/old.txt", FatTestData, 1000));
	}

	//Only so many files are open at once, however many opens each has; closing one makes room
	{
		KFile held[Fat32::MaxOpen] = {};
		char name[32];
		for (size_t i = 0; i < Fat32::MaxOpen; i++)
		{
			snprintf(name, sizeof(name), "/held%02u.txt", static_cast<uint32_t>(i));
			Assert(FatTestOpen(fs, held[i], name, KFileAccess::Write, KFileCreate::CreateAlways));
		}
		Assert(!FatTestOpen(fs, file, "/old.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(!FatTestOpen(fs, file, "/held64.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(FatTestOpen(fs, file, "/held00.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		FatTestClose(fs, file);
		FatTestClose(fs, held[0]);
		Assert(FatTestOpen(fs, file, "/old.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		FatTestClose(fs, file);
		for (size_t i = 0; i < Fat32::MaxOpen; i++)
		{
			FatTestClose(fs, held[i]);
			snprintf(name, sizeof(name), "/held%02u.txt", static_cast<uint32_t>(i));
			Assert(fs.Delete(name));
		}
		Assert(!FatTestOpen(fs, file, "/held64.txt", KFileAccess::Read, KFileCreate::OpenExisting));
	}

	//A long name gets an alias with a numeric tail, the next along when that's taken; a name that is its 8.3 alias exactly needs no long name
	{
		Assert(FatTestOpen(fs, file, "/Long File Name.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(FatTestOpen(fs, file, "/Long File Name 2.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(FatTestOpen(fs, file, "/LONGFI~1.TXT", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "/longfi~2.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestOpen(fs, file, "/LONG FILE NAME 2.TXT", KFileAccess::Read, KFileCreate::OpenExisting));

		bool longName = true;
		Assert(FatTestOpen(fs, file, "/UPPER.TXT", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(volume.FindEntry(FatTestVolume::Root, "UPPER   TXT", &longName) != nullptr);
		Assert(!longName);
		Assert(FatTestOpen(fs, file, "/lower.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(volume.FindEntry(FatTestVolume::Root, "LOWER   TXT", &longName) != nullptr);
		Assert(longName);
		Assert(FatTestOpen(fs, file, "/.hidden", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(volume.FindEntry(FatTestVolume::Root, "HIDDEN~1   ") != nullptr);
		Assert(FatTestOpen(fs, file, "/a+b.tar.gz", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(volume.FindEntry(FatTestVolume::Root, "A_BTAR~1GZ ") != nullptr);
	}

	//A name of 255 characters fits, one longer doesn't, and nor does one with a reserved character, a trailing dot or a missing directory
	{
		char name[258] = "/";
		memset(name + 1, 'x', 251);
		memcpy(name + 252, ".txt", 5);
		Assert(FatTestOpen(fs, file, name, KFileAccess::Write, KFileCreate::CreateAlways));
		AssertEqual(FatTestWrite(fs, file, FatTestData, 600, 600), (size_t)600);
		Assert(FatTestSame(fs, name, FatTestData, 600));
		Assert(FatTestSame(fs, "/XXXXXX~1.TXT", FatTestData, 600));

		memcpy(name + 252, "x.txt", 6);
		Assert(!FatTestOpen(fs, file, name, KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/bad?.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/trailing.", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/missing/x.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/old.txt/x.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		Assert(!FatTestOpen(fs, file, "/", KFileAccess::Write, KFileCreate::OpenAlways));
		Assert(!fs.CreateDirectory("/"));
		Assert(!fs.CreateDirectory("/old.txt"));
	}

	//A directory made holds files, and goes only once empty; the root, the default directory and ".." never go
	{
		Assert(fs.CreateDirectory("/Projects"));
		Assert(!fs.CreateDirectory("/projects"));
		Assert(!FatTestOpen(fs, file, "/projects", KFileAccess::Write, KFileCreate::OpenAlways));
		Assert(fs.CreateDirectory("/projects/Sub"));
		Assert(FatTestOpen(fs, file, "/projects/sub/notes.txt", KFileAccess::Write, KFileCreate::CreateAlways));
		AssertEqual(FatTestWrite(fs, file, FatTestData, 100, 100), (size_t)100);

		Assert(fs.SetDirectory("/projects/sub"));
		Assert(FatTestSame(fs, "notes.txt", FatTestData, 100));
		Assert(FatTestSame(fs, "../sub/notes.txt", FatTestData, 100));
		Assert(FatTestSame(fs, "../../old.txt", FatTestData, 1000));
		Assert(!fs.Delete("/projects"));
		Assert(!fs.Delete("/projects/sub"));
		Assert(!fs.Delete(".."));
		Assert(!fs.Delete("/"));
		Assert(fs.SetDirectory("/"));

		uint8_t* const projects = volume.FindEntry(FatTestVolume::Root, "PROJECTS   ");
		Assert(projects != nullptr);
		const uint32_t cluster = FatTestVolume::EntryCluster(projects);
		Assert(!fs.Delete("/projects/sub/notes.txt"));
		FatTestClose(fs, file);
		Assert(fs.Delete("/projects/sub/notes.txt"));
		Assert(!FatTestOpen(fs, file, "/projects/sub/notes.txt", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(fs.Delete("/projects/sub"));
		Assert(fs.Delete("/projects"));
		AssertEqual(projects[0], (uint8_t)0xE5);
		AssertEqual(volume.GetFat(cluster), (uint32_t)0);
		Assert(!fs.Delete("/projects"));
	}

	//A rename within a directory or to another keeps the data, a directory moved gets its new parent in "..", and nothing moves onto a name taken or into itself
	{
		Assert(fs.CreateDirectory("/a"));
		Assert(fs.CreateDirectory("/a/b"));
		Assert(FatTestOpen(fs, file, "/a/data.bin", KFileAccess::Write, KFileCreate::CreateAlways));
		AssertEqual(FatTestWrite(fs, file, FatTestData, 3000, 3000), (size_t)3000);
		Assert(!fs.Rename("/a/data.bin", "/a/Renamed Data.bin"));
		FatTestClose(fs, file);

		Assert(fs.Rename("/a/data.bin", "/a/Renamed Data.bin"));
		Assert(!FatTestOpen(fs, file, "/a/data.bin", KFileAccess::Read, KFileCreate::OpenExisting));
		Assert(FatTestSame(fs, "/a/renamed data.bin", FatTestData, 3000));
		Assert(fs.Rename("/a/renamed data.bin", "/moved.bin"));
		Assert(FatTestSame(fs, "/moved.bin", FatTestData, 3000));

		Assert(!fs.Rename("/moved.bin", "/a/b/../../old.txt"));
		Assert(!fs.Rename("/moved.bin", "/bad|name"));
		Assert(!fs.Rename("/missing", "/x"));
		Assert(!fs.Rename("/", "/x"));
		Assert(!fs.Rename("/a", "/a/a"));
		Assert(!fs.Rename("/a", "/a/b/a"));

		Assert(fs.Rename("/a/b", "/b"));
		Assert(FatTestSame(fs, "/b/../moved.bin", FatTestData, 3000));
		Assert(fs.Rename("/b", "/a/c"));
		Assert(fs.SetDirectory("/a/c"));
		Assert(FatTestSame(fs, "../../moved.bin", FatTestData, 3000));
		Assert(fs.SetDirectory("/"));
	}

	//A directory grows a cluster at a time as it fills, and a deleted name's entries are used again
	{
		Assert(fs.CreateDirectory("/many"));
		char name[40];
		for (int i = 0; i < 40; i++)
		{
			snprintf(name, sizeof(name), "/many/File number %02d.dat", i);
			Assert(FatTestOpen(fs, file, name, KFileAccess::Write, KFileCreate::CreateAlways));
			AssertEqual(FatTestWrite(fs, file, FatTestData + i, 1, 1), (size_t)1);
		}
		Assert(FatTestSame(fs, "/many/file number 39.dat", FatTestData + 39, 1));
		Assert(FatTestSame(fs, "/many/file number 00.dat", FatTestData, 1));

		const uint32_t free = fs.FreeClusters();
		Assert(fs.Delete("/many/File number 07.dat"));
		Assert(FatTestOpen(fs, file, "/many/File number 77.dat", KFileAccess::Write, KFileCreate::CreateAlways));
		AssertEqual(fs.FreeClusters(), free + 1);
		Assert(FatTestSame(fs, "/many/file number 08.dat", FatTestData + 8, 1));
	}

	//Every change reached both FATs, and FSInfo's free count is the FAT's
	{
		Assert(fs.Flush());
		Assert(memcmp(volume.Disk.Sector(FatTestVolume::Reserved), volume.Disk.Sector(FatTestVolume::Reserved + FatTestVolume::FatSectors), FatTestVolume::FatSectors * 512) == 0);
		AssertEqual(volume.FsInfoFree(), fs.FreeClusters());
		AssertEqual(volume.CountFree(), fs.FreeClusters());
	}
}
