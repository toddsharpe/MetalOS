#pragma once

#include <cstddef>
#include <cstdint>
#include "kernel/FileSystems/FileSystem.h"
#include "kernel/FileSystems/SectorCache.h"
#include "kernel/Devices/BlockDevice.h"

//A FAT32 volume, read-only for now: long or short names in any case, '/' or '\' between them, and a path not from the root starts in the default directory
class Fat32 : public FileSystem
{
public:
	constexpr Fat32() :
		m_volume(),
		m_cache(),
		m_sectorSize(),
		m_sectorsPerCluster(),
		m_fatStart(),
		m_dataStart(),
		m_clusterCount(),
		m_root(),
		m_directory()
	{

	}

	//False if the device doesn't hold a FAT32 volume; the cache is this volume's alone
	bool Mount(BlockDevice& volume, SectorCache& cache);

	//Where a path that doesn't start at the root begins; false if it isn't a directory
	bool SetDirectory(const char* const path);

	bool Open(KFile& file, const char* const path, const KFileAccess access) override;
	size_t Read(KFile& file, void* const buffer, const size_t bytes) override;

private:
	struct entry_t
	{
		uint32_t Cluster;
		uint32_t Size;
		bool Directory;
	};

	bool IsData(const uint32_t cluster) const;
	uint64_t ClusterSector(const uint32_t cluster) const;

	//The cluster after this one in its chain, read from the FAT; zero if the FAT can't be read
	uint32_t Next(const uint32_t cluster);

	//The index-th cluster of the file's chain: from its run, else walked on from the nearest cluster known
	uint32_t ClusterAt(KFile& file, const uint32_t index);

	bool Find(const uint32_t directory, const char* const name, const size_t length, entry_t& found);
	bool Walk(const char* const path, entry_t& found);

	BlockDevice* m_volume;
	SectorCache* m_cache;
	size_t m_sectorSize;
	uint32_t m_sectorsPerCluster;
	uint64_t m_fatStart;
	uint64_t m_dataStart;
	uint32_t m_clusterCount;
	uint32_t m_root;
	uint32_t m_directory;
};
