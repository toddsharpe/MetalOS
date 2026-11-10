#include "kernel/FileSystems/Fat32.h"
#include "Lib/Math.h"
#include <cstring>

namespace
{
	static constexpr size_t EntrySize = 32;
	static constexpr size_t MaxLongName = 255;
	static constexpr size_t LongNameChars = 13;

	static constexpr uint8_t AttrVolumeId = 0x08;
	static constexpr uint8_t AttrDirectory = 0x10;
	static constexpr uint8_t AttrLongName = 0x0F;
	static constexpr uint8_t AttrLongNameMask = 0x3F;

	static constexpr uint8_t EntryEnd = 0x00;
	static constexpr uint8_t EntryDeleted = 0xE5;
	static constexpr uint8_t EntryKanjiE5 = 0x05;
	static constexpr uint8_t LongNameLast = 0x40;
	static constexpr uint8_t LongNameOrdinal = 0x1F;

	static constexpr uint32_t ClusterMask = 0x0FFFFFFF;

	//Fewer clusters than this make a volume FAT12 or FAT16, whatever its boot sector says
	static constexpr uint32_t MinClusters = 65525;

	//Where a long name entry's 13 UTF-16 characters sit
	static constexpr size_t LongNameOffsets[LongNameChars] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };

	template <typename T>
	T LittleEndian(const uint8_t* const bytes)
	{
		T value;
		memcpy(&value, bytes, sizeof(T));
		return value;
	}

	char Lower(const char c)
	{
		return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
	}

	//A stored name against a path component, ignoring case
	bool SameName(const char* const stored, const char* const name, const size_t length)
	{
		for (size_t i = 0; i < length; i++)
		{
			if (stored[i] == '\0' || Lower(stored[i]) != Lower(name[i]))
				return false;
		}
		return stored[length] == '\0';
	}

	//The 8.3 name as BASE.EXT, without padding
	void ShortName(const uint8_t* const entry, char (&name)[13])
	{
		size_t at = 0;
		for (size_t i = 0; i < 8 && entry[i] != ' '; i++)
			name[at++] = (i == 0 && entry[i] == EntryKanjiE5) ? static_cast<char>(EntryDeleted) : static_cast<char>(entry[i]);

		if (entry[8] != ' ')
		{
			name[at++] = '.';
			for (size_t i = 8; i < 11 && entry[i] != ' '; i++)
				name[at++] = static_cast<char>(entry[i]);
		}
		name[at] = '\0';
	}

	//What a long name's entries carry to tie them to their short entry
	uint8_t ShortChecksum(const uint8_t* const entry)
	{
		uint8_t sum = 0;
		for (size_t i = 0; i < 11; i++)
			sum = static_cast<uint8_t>(((sum & 1) << 7) + (sum >> 1) + entry[i]);
		return sum;
	}

	bool IsSeparator(const char c)
	{
		return c == '/' || c == '\\';
	}
}

bool Fat32::Mount(BlockDevice& volume, SectorCache& cache)
{
	const size_t sectorSize = volume.SectorSize();
	if (sectorSize < 512 || sectorSize > SectorCache::MaxSectorSize)
		return false;

	uint8_t boot[SectorCache::MaxSectorSize];
	if (!volume.Read(0, 1, boot) || boot[510] != 0x55 || boot[511] != 0xAA)
		return false;

	const uint16_t bytesPerSector = LittleEndian<uint16_t>(boot + 11);
	const uint8_t sectorsPerCluster = boot[13];
	const uint16_t reserved = LittleEndian<uint16_t>(boot + 14);
	const uint8_t fats = boot[16];
	const uint16_t rootEntries = LittleEndian<uint16_t>(boot + 17);
	const uint16_t totalSectors16 = LittleEndian<uint16_t>(boot + 19);
	const uint16_t fatSize16 = LittleEndian<uint16_t>(boot + 22);
	const uint32_t totalSectors32 = LittleEndian<uint32_t>(boot + 32);
	const uint32_t fatSize = LittleEndian<uint32_t>(boot + 36);
	const uint16_t flags = LittleEndian<uint16_t>(boot + 40);
	const uint16_t version = LittleEndian<uint16_t>(boot + 42);
	const uint32_t root = LittleEndian<uint32_t>(boot + 44);

	//FAT32's own shape: no fixed root directory, no 16-bit sizes, version 0
	const uint32_t totalSectors = totalSectors16 != 0 ? totalSectors16 : totalSectors32;
	if (bytesPerSector != sectorSize || sectorsPerCluster == 0 || (sectorsPerCluster & (sectorsPerCluster - 1)) != 0 ||
		reserved == 0 || fats == 0 || rootEntries != 0 || fatSize16 != 0 || fatSize == 0 || version != 0 || totalSectors > volume.SectorCount())
		return false;

	const uint64_t metadata = reserved + static_cast<uint64_t>(fats) * fatSize;
	if (metadata >= totalSectors)
		return false;

	//The cluster count makes the FAT type, and the FAT must hold an entry for each cluster and the two reserved ones
	const uint64_t clusters = (totalSectors - metadata) / sectorsPerCluster;
	if (clusters < MinClusters || clusters > ClusterMask - 16 || static_cast<uint64_t>(fatSize) * sectorSize / sizeof(uint32_t) < clusters + 2)
		return false;

	//With mirroring off, the low bits name the one FAT in use
	const uint32_t active = (flags & 0x80) ? (flags & 0x0F) : 0;
	if (active >= fats)
		return false;

	m_volume = &volume;
	m_cache = &cache;
	m_sectorSize = sectorSize;
	m_sectorsPerCluster = sectorsPerCluster;
	m_fatStart = reserved + static_cast<uint64_t>(active) * fatSize;
	m_dataStart = metadata;
	m_clusterCount = static_cast<uint32_t>(clusters);
	m_root = root;
	m_directory = root;
	cache.Attach(volume);

	if (!IsData(root))
	{
		m_volume = nullptr;
		return false;
	}
	return true;
}

bool Fat32::SetDirectory(const char* const path)
{
	entry_t entry;
	if (!m_volume || !Walk(path, entry) || !entry.Directory)
		return false;

	m_directory = entry.Cluster;
	return true;
}

bool Fat32::Open(KFile& file, const char* const path, const KFileAccess access)
{
	//Read-only for now
	entry_t entry;
	if (!m_volume || access != KFileAccess::Read || !Walk(path, entry) || entry.Directory)
		return false;

	file = KFile();
	file.Start = entry.Cluster;
	file.Length = entry.Size;
	file.Access = access;

	//How far the chain runs on in consecutive clusters, as a file written in one go does, so reads there need no FAT and go in one request
	if (IsData(entry.Cluster))
	{
		const size_t clusterBytes = m_sectorsPerCluster * m_sectorSize;
		const uint32_t clusters = static_cast<uint32_t>((entry.Size + clusterBytes - 1) / clusterBytes);
		file.Run = 1;
		for (uint32_t cluster = entry.Cluster; file.Run < clusters && IsData(cluster + 1) && Next(cluster) == cluster + 1; cluster++)
			file.Run++;
	}
	return true;
}

size_t Fat32::Read(KFile& file, void* const buffer, const size_t bytes)
{
	if (file.Position >= file.Length)
		return 0;

	const size_t total = Min(bytes, file.Length - file.Position);
	const size_t clusterBytes = m_sectorsPerCluster * m_sectorSize;
	uint8_t* const out = static_cast<uint8_t*>(buffer);
	size_t position = file.Position;
	size_t done = 0;
	while (done < total)
	{
		//A chain that ends before the file does gives what it has
		const uint32_t index = static_cast<uint32_t>(position / clusterBytes);
		const uint32_t cluster = ClusterAt(file, index);
		if (!IsData(cluster))
			break;

		//The clusters that follow on from this one in the file's run read with it
		const size_t run = index < file.Run ? file.Run - index : 1;
		const size_t offset = position % clusterBytes;
		const uint64_t sector = ClusterSector(cluster) + offset / m_sectorSize;
		const size_t inSector = offset % m_sectorSize;
		const size_t left = total - done;
		size_t moved;
		if (inSector == 0 && left >= m_sectorSize)
		{
			//Whole sectors, up to the end of the run, go straight to the caller
			const size_t sectors = Min((run * clusterBytes - offset) / m_sectorSize, left / m_sectorSize);
			if (!m_volume->Read(sector, sectors, out + done))
				break;
			moved = sectors * m_sectorSize;
		}
		else
		{
			const uint8_t* const data = m_cache->Get(sector);
			if (!data)
				break;
			moved = Min(m_sectorSize - inSector, left);
			memcpy(out + done, data + inSector, moved);
		}

		done += moved;
		position += moved;
	}
	return done;
}

bool Fat32::IsData(const uint32_t cluster) const
{
	return cluster >= 2 && cluster < m_clusterCount + 2;
}

uint64_t Fat32::ClusterSector(const uint32_t cluster) const
{
	return m_dataStart + static_cast<uint64_t>(cluster - 2) * m_sectorsPerCluster;
}

uint32_t Fat32::Next(const uint32_t cluster)
{
	const uint64_t offset = static_cast<uint64_t>(cluster) * sizeof(uint32_t);
	const uint8_t* const data = m_cache->Get(m_fatStart + offset / m_sectorSize);
	return data ? LittleEndian<uint32_t>(data + offset % m_sectorSize) & ClusterMask : 0;
}

uint32_t Fat32::ClusterAt(KFile& file, const uint32_t index)
{
	if (index < file.Run)
		return file.Start + index;

	//Past the run the FAT is walked, from the run's last cluster or from where the last read left off if that's nearer
	uint32_t cluster = file.Run != 0 ? file.Start + file.Run - 1 : file.Start;
	uint32_t at = file.Run != 0 ? file.Run - 1 : 0;
	if (file.Cursor != 0 && file.CursorIndex > at && file.CursorIndex <= index)
	{
		cluster = file.Cursor;
		at = file.CursorIndex;
	}

	for (; at < index; at++)
	{
		if (!IsData(cluster))
			return 0;
		cluster = Next(cluster);
	}

	file.Cursor = cluster;
	file.CursorIndex = index;
	return cluster;
}

//A name in a directory: its long name when its entries are whole and tied to the short entry by checksum, else its short name
bool Fat32::Find(const uint32_t directory, const char* const name, const size_t length, entry_t& found)
{
	if (length == 0 || length > MaxLongName)
		return false;

	char longName[MaxLongName + 1] = {};
	bool longValid = false;
	uint8_t longChecksum = 0;
	uint8_t longNext = 0;

	//A chain can't be longer than the volume has clusters, so a looped one ends
	uint32_t cluster = directory;
	for (uint32_t steps = 0; IsData(cluster) && steps < m_clusterCount; steps++)
	{
		for (uint32_t s = 0; s < m_sectorsPerCluster; s++)
		{
			const uint8_t* const data = m_cache->Get(ClusterSector(cluster) + s);
			if (!data)
				return false;

			for (size_t offset = 0; offset < m_sectorSize; offset += EntrySize)
			{
				const uint8_t* const entry = data + offset;
				if (entry[0] == EntryEnd)
					return false;

				const uint8_t attributes = entry[11];
				if (entry[0] == EntryDeleted)
				{
					longValid = false;
					continue;
				}

				//Long name entries come last part first, each with its ordinal; any break in the run drops the long name
				if ((attributes & AttrLongNameMask) == AttrLongName)
				{
					const uint8_t ordinal = entry[0] & LongNameOrdinal;
					if (entry[0] & LongNameLast)
					{
						memset(longName, 0, sizeof(longName));
						longChecksum = entry[13];
						longNext = ordinal;
						longValid = ordinal >= 1 && ordinal * LongNameChars <= MaxLongName + LongNameChars - 1;
					}
					if (!longValid || ordinal != longNext || entry[13] != longChecksum)
					{
						longValid = false;
						continue;
					}

					for (size_t i = 0; i < LongNameChars; i++)
					{
						const uint16_t unit = LittleEndian<uint16_t>(entry + LongNameOffsets[i]);
						const size_t at = (ordinal - 1) * LongNameChars + i;
						if (unit == 0x0000 || at >= MaxLongName)
							break;
						longName[at] = (unit >= 0x20 && unit < 0x7F) ? static_cast<char>(unit) : '?';
					}
					longNext--;
					continue;
				}

				if (attributes & AttrVolumeId)
				{
					longValid = false;
					continue;
				}

				bool match = longValid && longNext == 0 && longChecksum == ShortChecksum(entry) && SameName(longName, name, length);
				longValid = false;
				if (!match)
				{
					char shortName[13];
					ShortName(entry, shortName);
					match = SameName(shortName, name, length);
				}

				if (match)
				{
					found.Cluster = (static_cast<uint32_t>(LittleEndian<uint16_t>(entry + 20)) << 16) | LittleEndian<uint16_t>(entry + 26);
					found.Size = LittleEndian<uint32_t>(entry + 28);
					found.Directory = (attributes & AttrDirectory) != 0;
					return true;
				}
			}
		}
		cluster = Next(cluster);
	}
	return false;
}

//Each name in turn from the root or the default directory; "." stays put and ".." to the root has cluster 0
bool Fat32::Walk(const char* const path, entry_t& found)
{
	found = { IsSeparator(path[0]) ? m_root : m_directory, 0, true };
	for (const char* at = path; *at != '\0';)
	{
		if (IsSeparator(*at))
		{
			at++;
			continue;
		}

		const char* const start = at;
		while (*at != '\0' && !IsSeparator(*at))
			at++;
		const size_t length = static_cast<size_t>(at - start);
		if (length == 1 && start[0] == '.')
			continue;

		//A file can't be passed through
		if (!found.Directory || !Find(found.Cluster, start, length, found))
			return false;
		if (found.Directory && found.Cluster == 0)
			found.Cluster = m_root;
	}
	return true;
}
