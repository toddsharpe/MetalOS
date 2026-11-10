#pragma once

#include <cstddef>
#include <cstdint>
#include "kernel/FileSystems/FileSystem.h"
#include "kernel/FileSystems/SectorCache.h"
#include "kernel/Devices/BlockDevice.h"

//A FAT32 volume: long or short names in any case, '/' or '\' between them, and a path not from the root starts in the default directory
class Fat32 : public FileSystem
{
public:
	//The free cluster count when the volume's FSInfo didn't give one
	static constexpr uint32_t Unknown = 0xFFFFFFFF;

	//Files open at once, however many opens each has
	static constexpr size_t MaxOpen = 64;

	constexpr Fat32() :
		m_volume(),
		m_cache(),
		m_sectorSize(),
		m_sectorsPerCluster(),
		m_fatStart(),
		m_dataStart(),
		m_clusterCount(),
		m_root(),
		m_directory(),
		m_fsInfo(),
		m_freeCount(),
		m_nextFree(),
		m_open()
	{

	}

	//False if the device doesn't hold a FAT32 volume; the cache is this volume's alone
	bool Mount(BlockDevice& volume, SectorCache& cache);

	//Where a path that doesn't start at the root begins; false if it isn't a directory
	bool SetDirectory(const char* const path);

	uint32_t FreeClusters() const
	{
		return m_freeCount;
	}

	//Any number may read a file but one writes, and one anyone has open isn't emptied, deleted or renamed; false too when MaxOpen files are open
	bool Open(KFile& file, const char* const path, const KFileAccess access, const KFileCreate create) override;
	void Close(KFile& file) override;
	size_t Read(KFile& file, void* const buffer, const size_t bytes) override;
	size_t Write(KFile& file, const void* const buffer, const size_t bytes) override;
	bool Delete(const char* const path) override;
	bool Rename(const char* const from, const char* const to) override;
	bool CreateDirectory(const char* const path) override;
	bool List(const char* const path, const KListVisit visit, void* const context) override;
	bool Flush() override;

private:
	//A long name of 255 characters takes 20 entries, then the short entry
	static constexpr size_t MaxEntries = 21;

	//Where a 32-byte directory entry is
	struct location_t
	{
		uint64_t Sector;
		uint32_t Offset;
	};

	//What a path names, and where its entries are, long name first; the root and the default directory have none
	struct entry_t
	{
		uint32_t Cluster;
		uint32_t Size;
		bool Directory;
		uint8_t Attributes;
		size_t Count;
		location_t Slots[MaxEntries];
	};

	//A file open now, by where its short entry is: how many opens it has, and whether one writes
	struct open_t
	{
		uint64_t Sector;
		uint32_t Offset;
		uint32_t Count;
		bool Writer;
	};

	//The open file whose short entry is there, or with null a free slot; null if there's none
	open_t* FindOpen(const location_t* const at);

	bool IsData(const uint32_t cluster) const;
	uint64_t ClusterSector(const uint32_t cluster) const;

	//The FAT entry for a cluster: the next in its chain, zero for a free one; false if the FAT can't be read or written
	bool GetFat(const uint32_t cluster, uint32_t& value);
	bool SetFat(const uint32_t cluster, const uint32_t value);
	uint32_t Next(const uint32_t cluster);

	//A free cluster, marked the end of a chain, searched for from hint; zero when the volume is full
	uint32_t Allocate(const uint32_t hint);
	void FreeChain(const uint32_t first);
	bool ZeroCluster(const uint32_t cluster);

	//The index-th cluster of the file's chain: from its run, else walked on from the nearest cluster known
	uint32_t ClusterAt(KFile& file, const uint32_t index);

	//The clusters the file's chain has once it's grown toward count, reusing any already chained past its end; fewer when the volume fills
	uint32_t Extend(KFile& file, const uint32_t count);

	//Bytes at a position, or zeros with no data, through the cluster chain
	size_t WriteAt(KFile& file, const size_t position, const uint8_t* const data, const size_t bytes);

	//Calls visit(entry, location) for each entry slot of the directory in order along its chain, until visit returns false; visit mustn't use the cache, and last gets the chain's last cluster
	template <typename Visit>
	bool Scan(const uint32_t directory, Visit visit, uint32_t* const last = nullptr);

	//Calls visit(entry, longName, slots, count) for each name in the directory until it returns false: its short entry, its long name (null when there's none whole and tied to it by checksum), and where its entries are, long name first
	template <typename Visit>
	bool Names(const uint32_t directory, Visit visit);

	bool Find(const uint32_t directory, const char* const name, const size_t length, entry_t& found);
	bool Walk(const char* const path, const size_t length, entry_t& found);
	bool Parent(const char* const path, uint32_t& directory, const char*& leaf, size_t& length);
	bool IsEmpty(const uint32_t directory);
	uint32_t ParentOf(const uint32_t directory);

	bool ShortTaken(const uint32_t directory, const uint8_t (&alias)[11]);
	bool MakeAlias(const uint32_t directory, const char* const name, const size_t length, uint8_t (&alias)[11], bool& longName);

	//count free slots in a row, the directory grown when its chain runs out
	bool FreeSlots(const uint32_t directory, const size_t count, location_t* const slots);

	//A name's entries into free slots: its long name when it has one, then the short entry from the template with the alias over its name
	bool AddEntries(const uint32_t directory, const char* const name, const size_t length, const uint8_t (&shortEntry)[32], entry_t& added);
	bool SetEntry(const location_t at, const uint32_t cluster, const uint32_t size);

	//Modified sectors and the free count to the device, as each change ends
	bool Commit();

	BlockDevice* m_volume;
	SectorCache* m_cache;
	size_t m_sectorSize;
	uint32_t m_sectorsPerCluster;
	uint64_t m_fatStart;
	uint64_t m_dataStart;
	uint32_t m_clusterCount;
	uint32_t m_root;
	uint32_t m_directory;
	uint64_t m_fsInfo;
	uint32_t m_freeCount;
	uint32_t m_nextFree;
	open_t m_open[MaxOpen];
};
