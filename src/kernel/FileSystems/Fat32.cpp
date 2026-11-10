#include "kernel/FileSystems/Fat32.h"
#include "Lib/Math.h"
#include <cstring>

namespace
{
	static constexpr size_t EntrySize = 32;
	static constexpr size_t MaxLongName = 255;
	static constexpr size_t LongNameChars = 13;
	static constexpr size_t MaxFileSize = 0xFFFFFFFF;

	static constexpr uint8_t AttrReadOnly = 0x01;
	static constexpr uint8_t AttrVolumeId = 0x08;
	static constexpr uint8_t AttrDirectory = 0x10;
	static constexpr uint8_t AttrArchive = 0x20;
	static constexpr uint8_t AttrLongName = 0x0F;
	static constexpr uint8_t AttrLongNameMask = 0x3F;

	static constexpr uint8_t EntryEnd = 0x00;
	static constexpr uint8_t EntryDeleted = 0xE5;
	static constexpr uint8_t EntryKanjiE5 = 0x05;
	static constexpr uint8_t LongNameLast = 0x40;
	static constexpr uint8_t LongNameOrdinal = 0x1F;

	//Windows keeps a short name's case in byte 12 rather than giving it a long name
	static constexpr uint8_t CaseLowerBase = 0x08;
	static constexpr uint8_t CaseLowerExtension = 0x10;

	static constexpr uint32_t ClusterMask = 0x0FFFFFFF;
	static constexpr uint32_t EndOfChain = 0x0FFFFFFF;

	//Fewer clusters than this make a volume FAT12 or FAT16, whatever its boot sector says
	static constexpr uint32_t MinClusters = 65525;

	//FSInfo's signatures and the fields kept in it
	static constexpr uint32_t FsInfoLead = 0x41615252;
	static constexpr uint32_t FsInfoStruct = 0x61417272;
	static constexpr uint32_t FsInfoTrail = 0xAA550000;
	static constexpr size_t FsInfoFreeCount = 488;
	static constexpr size_t FsInfoNextFree = 492;

	//Where a long name entry's 13 UTF-16 characters sit
	static constexpr size_t LongNameOffsets[LongNameChars] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };

	template <typename T>
	T LittleEndian(const uint8_t* const bytes)
	{
		T value;
		memcpy(&value, bytes, sizeof(T));
		return value;
	}

	template <typename T>
	void PutLittleEndian(uint8_t* const bytes, const T value)
	{
		memcpy(bytes, &value, sizeof(T));
	}

	char Lower(const char c)
	{
		return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
	}

	char Upper(const char c)
	{
		return (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
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

	//The 8.3 name as BASE.EXT, without padding, either part lower-cased if asked
	void ShortName(const uint8_t* const entry, char (&name)[13], const bool lowerBase = false, const bool lowerExtension = false)
	{
		size_t at = 0;
		for (size_t i = 0; i < 8 && entry[i] != ' '; i++)
		{
			const char c = (i == 0 && entry[i] == EntryKanjiE5) ? static_cast<char>(EntryDeleted) : static_cast<char>(entry[i]);
			name[at++] = lowerBase ? Lower(c) : c;
		}

		if (entry[8] != ' ')
		{
			name[at++] = '.';
			for (size_t i = 8; i < 11 && entry[i] != ' '; i++)
				name[at++] = lowerExtension ? Lower(static_cast<char>(entry[i])) : static_cast<char>(entry[i]);
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

	//A name a file can be given: printable, none of the characters FAT reserves, not "." or "..", not ending in a dot or space
	bool ValidName(const char* const name, const size_t length)
	{
		if (length == 0 || length > MaxLongName || name[length - 1] == '.' || name[length - 1] == ' ')
			return false;

		for (size_t i = 0; i < length; i++)
		{
			const char c = name[i];
			if (c < 0x20 || c > 0x7E || strchr("\"*/:<>?\\|", c) != nullptr)
				return false;
		}
		return true;
	}

	//A character as a short name holds it: upper case, or '_' for one it can't (which loses something)
	char ShortChar(const char c, bool& lossy)
	{
		const char upper = Upper(c);
		if ((upper >= 'A' && upper <= 'Z') || (upper >= '0' && upper <= '9') || strchr("$%'-_@~`!(){}^#&", upper) != nullptr)
			return upper;

		lossy = true;
		return '_';
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
	const uint16_t fsInfo = LittleEndian<uint16_t>(boot + 48);

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

	//With mirroring off, the low bits name the one FAT in use; with it on, a change to FAT 0 goes to every copy
	const bool mirrored = (flags & 0x80) == 0;
	const uint32_t active = mirrored ? 0 : (flags & 0x0F);
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
	if (mirrored)
		cache.Mirror(m_fatStart, fatSize, fatSize, fats - 1u);

	if (!IsData(root))
	{
		m_volume = nullptr;
		return false;
	}

	//FSInfo keeps the free count and where to look for a free cluster, when its signatures check out
	m_fsInfo = 0;
	m_freeCount = Unknown;
	m_nextFree = 2;
	const uint8_t* const info = (fsInfo != 0 && fsInfo < reserved) ? cache.Get(fsInfo) : nullptr;
	if (info && LittleEndian<uint32_t>(info) == FsInfoLead && LittleEndian<uint32_t>(info + 484) == FsInfoStruct && LittleEndian<uint32_t>(info + 508) == FsInfoTrail)
	{
		m_fsInfo = fsInfo;
		const uint32_t freeCount = LittleEndian<uint32_t>(info + FsInfoFreeCount);
		const uint32_t nextFree = LittleEndian<uint32_t>(info + FsInfoNextFree);
		m_freeCount = freeCount <= m_clusterCount ? freeCount : Unknown;
		m_nextFree = IsData(nextFree) ? nextFree : 2;
	}
	return true;
}

bool Fat32::SetDirectory(const char* const path)
{
	entry_t entry;
	if (!m_volume || !Walk(path, strlen(path), entry) || !entry.Directory)
		return false;

	m_directory = entry.Cluster;
	return true;
}

bool Fat32::Open(KFile& file, const char* const path, const KFileAccess access, const KFileCreate create)
{
	if (!m_volume)
		return false;

	const bool writing = (access & KFileAccess::Write) != KFileAccess::None;
	entry_t entry;
	open_t* open = nullptr;
	if (Walk(path, strlen(path), entry))
	{
		//A directory isn't a file, and a read-only file isn't written
		if (entry.Directory || entry.Count == 0 || (writing && (entry.Attributes & AttrReadOnly) != 0))
			return false;

		//A second writer, or emptying a file under those who have it open, would leave their view of its clusters out of date
		open = FindOpen(&entry.Slots[entry.Count - 1]);
		if (open && ((writing && open->Writer) || create == KFileCreate::CreateAlways))
			return false;

		//The table is checked before anything changes, so a full one fails an open cleanly
		if (!open && !(open = FindOpen(nullptr)))
			return false;

		if (create == KFileCreate::CreateAlways)
		{
			if (!writing)
				return false;

			FreeChain(entry.Cluster);
			entry.Cluster = 0;
			entry.Size = 0;
			if (!SetEntry(entry.Slots[entry.Count - 1], 0, 0) || !Commit())
				return false;
		}
	}
	else
	{
		uint32_t directory;
		const char* leaf;
		size_t length;
		open = FindOpen(nullptr);
		if (!open || create == KFileCreate::OpenExisting || !writing || !Parent(path, directory, leaf, length) || !ValidName(leaf, length))
			return false;

		uint8_t shortEntry[32] = {};
		shortEntry[11] = AttrArchive;
		const bool added = AddEntries(directory, leaf, length, shortEntry, entry);
		if (!Commit() || !added)
			return false;
	}

	if (open->Count == 0)
	{
		open->Sector = entry.Slots[entry.Count - 1].Sector;
		open->Offset = entry.Slots[entry.Count - 1].Offset;
		open->Writer = false;
	}
	open->Count++;
	open->Writer = open->Writer || writing;

	file = KFile();
	file.Start = entry.Cluster;
	file.Length = entry.Size;
	file.Access = access;
	file.EntrySector = entry.Slots[entry.Count - 1].Sector;
	file.EntryOffset = entry.Slots[entry.Count - 1].Offset;

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

void Fat32::Close(KFile& file)
{
	const location_t at = { file.EntrySector, file.EntryOffset };
	open_t* const open = FindOpen(&at);
	Assert(open);

	if ((file.Access & KFileAccess::Write) != KFileAccess::None)
		open->Writer = false;
	open->Count--;
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

size_t Fat32::Write(KFile& file, const void* const buffer, const size_t bytes)
{
	if (!m_volume || (file.Access & KFileAccess::Write) == KFileAccess::None || file.Position >= MaxFileSize)
		return 0;

	//A write past the end fills the gap with zeros first, as reading there would have given
	const size_t count = Min(bytes, MaxFileSize - file.Position);
	const size_t gap = file.Position > file.Length ? file.Position - file.Length : 0;
	const bool filled = gap == 0 || WriteAt(file, file.Length, nullptr, gap) == gap;
	const size_t done = filled ? WriteAt(file, file.Position, static_cast<const uint8_t*>(buffer), count) : 0;
	return Commit() ? done : 0;
}

bool Fat32::Delete(const char* const path)
{
	entry_t entry;
	if (!m_volume || !Walk(path, strlen(path), entry) || entry.Count == 0 || FindOpen(&entry.Slots[entry.Count - 1]))
		return false;

	//A read-only file stays, and a directory goes only empty and never the root or the default directory
	if ((entry.Attributes & AttrReadOnly) != 0 || (entry.Directory && (entry.Cluster == m_root || entry.Cluster == m_directory || !IsEmpty(entry.Cluster))))
		return false;

	FreeChain(entry.Cluster);
	for (size_t i = 0; i < entry.Count; i++)
	{
		uint8_t* const sector = m_cache->Modify(entry.Slots[i].Sector);
		if (!sector)
			return false;
		sector[entry.Slots[i].Offset] = EntryDeleted;
	}
	return Commit();
}

bool Fat32::Rename(const char* const from, const char* const to)
{
	entry_t source;
	uint32_t directory;
	const char* leaf;
	size_t length;
	if (!m_volume || !Walk(from, strlen(from), source) || source.Count == 0 || FindOpen(&source.Slots[source.Count - 1]) || !Parent(to, directory, leaf, length) || !ValidName(leaf, length))
		return false;

	entry_t taken;
	if (Find(directory, leaf, length, taken))
		return false;

	//A directory can't go into itself or anything below it
	if (source.Directory)
	{
		uint32_t at = directory;
		for (uint32_t steps = 0; steps < m_clusterCount && at != m_root; steps++)
		{
			if (at == source.Cluster || at == 0)
				return false;
			at = ParentOf(at);
		}
	}

	//The new entries keep everything of the old short entry but its name
	uint8_t shortEntry[32];
	const location_t old = source.Slots[source.Count - 1];
	const uint8_t* const sector = m_cache->Get(old.Sector);
	if (!sector)
		return false;
	memcpy(shortEntry, sector + old.Offset, sizeof(shortEntry));

	entry_t added;
	bool done = AddEntries(directory, leaf, length, shortEntry, added);
	for (size_t i = 0; done && i < source.Count; i++)
	{
		uint8_t* const slot = m_cache->Modify(source.Slots[i].Sector);
		done = slot != nullptr;
		if (slot)
			slot[source.Slots[i].Offset] = EntryDeleted;
	}

	//A directory that moved points its ".." at its new parent
	entry_t dotDot;
	if (done && source.Directory && Find(source.Cluster, "..", 2, dotDot))
		done = SetEntry(dotDot.Slots[dotDot.Count - 1], directory == m_root ? 0 : directory, 0);
	return Commit() && done;
}

bool Fat32::CreateDirectory(const char* const path)
{
	uint32_t directory;
	const char* leaf;
	size_t length;
	entry_t taken;
	if (!m_volume || !Parent(path, directory, leaf, length) || !ValidName(leaf, length) || Find(directory, leaf, length, taken))
		return false;

	const uint32_t cluster = Allocate(m_nextFree);
	if (!cluster)
		return false;

	//"." and ".." first, ".." to the root as cluster 0
	bool done = ZeroCluster(cluster);
	uint8_t* const first = done ? m_cache->Modify(ClusterSector(cluster)) : nullptr;
	if (first)
	{
		const uint32_t parent = directory == m_root ? 0 : directory;
		memset(first, ' ', 11);
		first[0] = '.';
		first[11] = AttrDirectory;
		PutLittleEndian<uint16_t>(first + 20, static_cast<uint16_t>(cluster >> 16));
		PutLittleEndian<uint16_t>(first + 26, static_cast<uint16_t>(cluster));
		memset(first + EntrySize, ' ', 11);
		first[EntrySize] = '.';
		first[EntrySize + 1] = '.';
		first[EntrySize + 11] = AttrDirectory;
		PutLittleEndian<uint16_t>(first + EntrySize + 20, static_cast<uint16_t>(parent >> 16));
		PutLittleEndian<uint16_t>(first + EntrySize + 26, static_cast<uint16_t>(parent));
	}

	uint8_t shortEntry[32] = {};
	shortEntry[11] = AttrDirectory;
	PutLittleEndian<uint16_t>(shortEntry + 20, static_cast<uint16_t>(cluster >> 16));
	PutLittleEndian<uint16_t>(shortEntry + 26, static_cast<uint16_t>(cluster));
	entry_t added;
	done = first != nullptr && AddEntries(directory, leaf, length, shortEntry, added);
	if (!done)
		FreeChain(cluster);
	return Commit() && done;
}

bool Fat32::List(const char* const path, const KListVisit visit, void* const context)
{
	entry_t directory;
	if (!m_volume || !Walk(path, strlen(path), directory) || !directory.Directory)
		return false;

	return Names(directory.Cluster, [&](const uint8_t* const entry, const char* const longName, const location_t* const slots, const size_t count) -> bool
	{
		UNUSED(slots);
		UNUSED(count);
		if (entry[0] == '.')
			return true;

		//A short name without a long one shows in the case Windows' flags in byte 12 give it
		char shortName[13];
		if (!longName)
			ShortName(entry, shortName, (entry[12] & CaseLowerBase) != 0, (entry[12] & CaseLowerExtension) != 0);

		KDirEntry item = {};
		item.Name = longName ? longName : shortName;
		item.Size = LittleEndian<uint32_t>(entry + 28);
		item.Directory = (entry[11] & AttrDirectory) != 0;
		return visit(context, item);
	});
}

bool Fat32::Flush()
{
	return m_volume && Commit() && m_volume->Flush();
}

Fat32::open_t* Fat32::FindOpen(const location_t* const at)
{
	for (open_t& open : m_open)
	{
		if (at ? (open.Count != 0 && open.Sector == at->Sector && open.Offset == at->Offset) : open.Count == 0)
			return &open;
	}
	return nullptr;
}

bool Fat32::IsData(const uint32_t cluster) const
{
	return cluster >= 2 && cluster < m_clusterCount + 2;
}

uint64_t Fat32::ClusterSector(const uint32_t cluster) const
{
	return m_dataStart + static_cast<uint64_t>(cluster - 2) * m_sectorsPerCluster;
}

bool Fat32::GetFat(const uint32_t cluster, uint32_t& value)
{
	const uint64_t offset = static_cast<uint64_t>(cluster) * sizeof(uint32_t);
	const uint8_t* const data = m_cache->Get(m_fatStart + offset / m_sectorSize);
	if (!data)
		return false;

	value = LittleEndian<uint32_t>(data + offset % m_sectorSize) & ClusterMask;
	return true;
}

//The top four bits of an entry are reserved, so they're kept
bool Fat32::SetFat(const uint32_t cluster, const uint32_t value)
{
	const uint64_t offset = static_cast<uint64_t>(cluster) * sizeof(uint32_t);
	uint8_t* const data = m_cache->Modify(m_fatStart + offset / m_sectorSize);
	if (!data)
		return false;

	uint8_t* const entry = data + offset % m_sectorSize;
	PutLittleEndian<uint32_t>(entry, (LittleEndian<uint32_t>(entry) & ~ClusterMask) | (value & ClusterMask));
	return true;
}

uint32_t Fat32::Next(const uint32_t cluster)
{
	uint32_t value = 0;
	return GetFat(cluster, value) ? value : 0;
}

uint32_t Fat32::Allocate(const uint32_t hint)
{
	if (m_freeCount == 0)
		return 0;

	const uint32_t start = IsData(hint) ? hint : 2;
	for (uint32_t i = 0; i < m_clusterCount; i++)
	{
		const uint32_t cluster = 2 + (start - 2 + i) % m_clusterCount;
		uint32_t value;
		if (!GetFat(cluster, value))
			return 0;
		if (value != 0)
			continue;

		if (!SetFat(cluster, EndOfChain))
			return 0;
		m_nextFree = IsData(cluster + 1) ? cluster + 1 : 2;
		if (m_freeCount != Unknown)
			m_freeCount--;
		return cluster;
	}
	return 0;
}

//A chain can't be longer than the volume has clusters, so a looped one ends
void Fat32::FreeChain(const uint32_t first)
{
	uint32_t cluster = first;
	for (uint32_t steps = 0; IsData(cluster) && steps < m_clusterCount; steps++)
	{
		uint32_t next;
		if (!GetFat(cluster, next) || !SetFat(cluster, 0))
			return;
		if (m_freeCount != Unknown)
			m_freeCount++;
		cluster = next;
	}
}

bool Fat32::ZeroCluster(const uint32_t cluster)
{
	for (uint32_t s = 0; s < m_sectorsPerCluster; s++)
	{
		if (!m_cache->Claim(ClusterSector(cluster) + s))
			return false;
	}
	return true;
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

uint32_t Fat32::Extend(KFile& file, const uint32_t count)
{
	//What the chain has: the clusters its length needs, and any linked on past them
	const size_t clusterBytes = m_sectorsPerCluster * m_sectorSize;
	uint32_t have = static_cast<uint32_t>((file.Length + clusterBytes - 1) / clusterBytes);
	if (have >= count)
		return have;

	uint32_t last = have != 0 ? ClusterAt(file, have - 1) : 0;
	if (have == 0 && IsData(file.Start))
	{
		last = file.Start;
		have = 1;
	}
	while (last != 0 && have < count)
	{
		const uint32_t next = Next(last);
		if (!IsData(next))
			break;
		last = next;
		have++;
	}

	//The rest are new, each after the last so a file written in order stays in one run
	while (have < count)
	{
		const uint32_t cluster = Allocate(last != 0 ? last + 1 : m_nextFree);
		if (!cluster)
			break;

		if (last != 0)
		{
			if (!SetFat(last, cluster))
				break;
		}
		else
		{
			file.Start = cluster;
			file.Run = 0;
		}

		if (file.Run == have && cluster == file.Start + have)
			file.Run++;
		last = cluster;
		have++;
	}
	return have;
}

size_t Fat32::WriteAt(KFile& file, const size_t position, const uint8_t* const data, const size_t bytes)
{
	const size_t clusterBytes = m_sectorsPerCluster * m_sectorSize;
	const uint32_t needed = static_cast<uint32_t>((position + bytes + clusterBytes - 1) / clusterBytes);
	const size_t room = static_cast<size_t>(Extend(file, needed)) * clusterBytes;
	const size_t total = room > position ? Min(bytes, room - position) : 0;

	size_t at = position;
	size_t done = 0;
	while (done < total)
	{
		const uint32_t index = static_cast<uint32_t>(at / clusterBytes);
		const uint32_t cluster = ClusterAt(file, index);
		if (!IsData(cluster))
			break;

		const size_t run = index < file.Run ? file.Run - index : 1;
		const size_t offset = at % clusterBytes;
		const uint64_t sector = ClusterSector(cluster) + offset / m_sectorSize;
		const size_t inSector = offset % m_sectorSize;
		const size_t left = total - done;
		size_t moved;
		if (data && inSector == 0 && left >= m_sectorSize)
		{
			//Whole sectors, up to the end of the run, go straight to the device, and any copy cached is out of date
			const size_t sectors = Min((run * clusterBytes - offset) / m_sectorSize, left / m_sectorSize);
			if (!m_volume->Write(sector, sectors, data + done))
				break;
			m_cache->Discard(sector, sectors);
			moved = sectors * m_sectorSize;
		}
		else
		{
			//A sector wholly past the file's end has nothing to keep, so it isn't read
			uint8_t* const cached = at - inSector >= file.Length ? m_cache->Claim(sector) : m_cache->Modify(sector);
			if (!cached)
				break;
			moved = Min(m_sectorSize - inSector, left);
			if (data)
				memcpy(cached + inSector, data + done, moved);
			else
				memset(cached + inSector, 0, moved);
		}

		done += moved;
		at += moved;
	}

	//The entry gets the new size, and the first cluster an empty file was just given
	if (position + done > file.Length)
		file.Length = position + done;
	SetEntry({ file.EntrySector, file.EntryOffset }, file.Start, static_cast<uint32_t>(file.Length));
	return done;
}

template <typename Visit>
bool Fat32::Scan(const uint32_t directory, Visit visit, uint32_t* const last)
{
	uint32_t cluster = directory;
	for (uint32_t steps = 0; IsData(cluster) && steps < m_clusterCount; steps++)
	{
		if (last)
			*last = cluster;

		for (uint32_t s = 0; s < m_sectorsPerCluster; s++)
		{
			const uint64_t sector = ClusterSector(cluster) + s;
			const uint8_t* const data = m_cache->Get(sector);
			if (!data)
				return false;

			for (size_t offset = 0; offset < m_sectorSize; offset += EntrySize)
			{
				if (!visit(data + offset, location_t{ sector, static_cast<uint32_t>(offset) }))
					return true;
			}
		}
		cluster = Next(cluster);
	}
	return true;
}

template <typename Visit>
bool Fat32::Names(const uint32_t directory, Visit visit)
{
	char longName[MaxLongName + 1] = {};
	bool longValid = false;
	uint8_t longChecksum = 0;
	uint8_t longNext = 0;
	location_t slots[MaxEntries] = {};
	size_t longCount = 0;

	return Scan(directory, [&](const uint8_t* const entry, const location_t at) -> bool
	{
		if (entry[0] == EntryEnd)
			return false;

		const uint8_t attributes = entry[11];
		if (entry[0] == EntryDeleted)
		{
			longValid = false;
			return true;
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
				longCount = 0;
				longValid = ordinal >= 1 && ordinal <= MaxEntries - 1;
			}
			if (!longValid || ordinal != longNext || entry[13] != longChecksum)
			{
				longValid = false;
				return true;
			}

			for (size_t i = 0; i < LongNameChars; i++)
			{
				const uint16_t unit = LittleEndian<uint16_t>(entry + LongNameOffsets[i]);
				const size_t index = (ordinal - 1) * LongNameChars + i;
				if (unit == 0x0000 || index >= MaxLongName)
					break;
				longName[index] = (unit >= 0x20 && unit < 0x7F) ? static_cast<char>(unit) : '?';
			}
			slots[longCount++] = at;
			longNext--;
			return true;
		}

		if (attributes & AttrVolumeId)
		{
			longValid = false;
			return true;
		}

		//A long name tied to this entry is its own, and its entries go with it
		const bool longOwned = longValid && longNext == 0 && longChecksum == ShortChecksum(entry);
		longValid = false;
		const size_t count = longOwned ? longCount : 0;
		slots[count] = at;
		return visit(entry, longOwned ? longName : nullptr, slots, count + 1);
	});
}

//A name in a directory: its long name or its short name, in any case
bool Fat32::Find(const uint32_t directory, const char* const name, const size_t length, entry_t& found)
{
	if (length == 0 || length > MaxLongName)
		return false;

	bool hit = false;
	const bool scanned = Names(directory, [&](const uint8_t* const entry, const char* const longName, const location_t* const slots, const size_t count) -> bool
	{
		bool match = longName && SameName(longName, name, length);
		if (!match)
		{
			char shortName[13];
			ShortName(entry, shortName);
			match = SameName(shortName, name, length);
		}
		if (!match)
			return true;

		found.Cluster = (static_cast<uint32_t>(LittleEndian<uint16_t>(entry + 20)) << 16) | LittleEndian<uint16_t>(entry + 26);
		found.Size = LittleEndian<uint32_t>(entry + 28);
		found.Directory = (entry[11] & AttrDirectory) != 0;
		found.Attributes = entry[11];
		found.Count = count;
		memcpy(found.Slots, slots, count * sizeof(location_t));
		hit = true;
		return false;
	});
	return scanned && hit;
}

//Each name in turn from the root or the default directory, "." staying put
bool Fat32::Walk(const char* const path, const size_t length, entry_t& found)
{
	found = {};
	found.Cluster = (length != 0 && IsSeparator(path[0])) ? m_root : m_directory;
	found.Directory = true;
	found.Attributes = AttrDirectory;

	const char* const end = path + length;
	for (const char* at = path; at < end;)
	{
		if (IsSeparator(*at))
		{
			at++;
			continue;
		}

		const char* const start = at;
		while (at < end && !IsSeparator(*at))
			at++;
		const size_t size = static_cast<size_t>(at - start);
		if (size == 1 && start[0] == '.')
			continue;

		//A file can't be passed through
		if (!found.Directory)
			return false;

		//".." goes up no further than the root, and like the root names no entry a change could reach
		if (size == 2 && start[0] == '.' && start[1] == '.')
		{
			const uint32_t parent = found.Cluster == m_root ? m_root : ParentOf(found.Cluster);
			if (!parent)
				return false;
			found = {};
			found.Cluster = parent;
			found.Directory = true;
			found.Attributes = AttrDirectory;
			continue;
		}

		if (!Find(found.Cluster, start, size, found))
			return false;
	}
	return true;
}

//The directory a path's last name goes in, and that name
bool Fat32::Parent(const char* const path, uint32_t& directory, const char*& leaf, size_t& length)
{
	const char* end = path + strlen(path);
	while (end > path && IsSeparator(end[-1]))
		end--;

	const char* start = end;
	while (start > path && !IsSeparator(start[-1]))
		start--;

	entry_t parent;
	if (start == end || !Walk(path, static_cast<size_t>(start - path), parent) || !parent.Directory)
		return false;

	directory = parent.Cluster;
	leaf = start;
	length = static_cast<size_t>(end - start);
	return true;
}

bool Fat32::IsEmpty(const uint32_t directory)
{
	bool empty = true;
	const bool scanned = Scan(directory, [&](const uint8_t* const entry, const location_t) -> bool
	{
		if (entry[0] == EntryEnd)
			return false;
		if (entry[0] == EntryDeleted || (entry[11] & AttrLongNameMask) == AttrLongName || entry[0] == '.')
			return true;

		empty = false;
		return false;
	});
	return scanned && empty;
}

//The directory above, from its ".." entry; the root's child names cluster 0
uint32_t Fat32::ParentOf(const uint32_t directory)
{
	entry_t dotDot;
	if (!Find(directory, "..", 2, dotDot))
		return 0;
	return dotDot.Cluster == 0 ? m_root : dotDot.Cluster;
}

bool Fat32::ShortTaken(const uint32_t directory, const uint8_t (&alias)[11])
{
	bool taken = false;
	const bool scanned = Scan(directory, [&](const uint8_t* const entry, const location_t) -> bool
	{
		if (entry[0] == EntryEnd)
			return false;
		if (entry[0] == EntryDeleted || (entry[11] & AttrLongNameMask) == AttrLongName)
			return true;

		taken = memcmp(entry, alias, sizeof(alias)) == 0;
		return !taken;
	});
	return !scanned || taken;
}

//The 8.3 alias: the name upper-cased as a short name holds it, with ~N over the end of the base when that lost something or is taken
bool Fat32::MakeAlias(const uint32_t directory, const char* const name, const size_t length, uint8_t (&alias)[11], bool& longName)
{
	char base[8];
	size_t baseLength = 0;
	char extension[3];
	size_t extensionLength = 0;
	bool lossy = false;

	//Leading dots are dropped, and the last dot left starts the extension
	size_t start = 0;
	while (start < length && name[start] == '.')
	{
		start++;
		lossy = true;
	}
	size_t dot = length;
	for (size_t i = start; i < length; i++)
	{
		if (name[i] == '.')
			dot = i;
	}

	for (size_t i = start; i < dot; i++)
	{
		if (name[i] == ' ' || name[i] == '.')
		{
			lossy = true;
			continue;
		}
		const char c = ShortChar(name[i], lossy);
		if (baseLength < sizeof(base))
			base[baseLength++] = c;
		else
			lossy = true;
	}
	for (size_t i = dot + 1; i < length; i++)
	{
		if (name[i] == ' ')
		{
			lossy = true;
			continue;
		}
		const char c = ShortChar(name[i], lossy);
		if (extensionLength < sizeof(extension))
			extension[extensionLength++] = c;
		else
			lossy = true;
	}
	if (baseLength == 0)
	{
		base[baseLength++] = '_';
		lossy = true;
	}

	memset(alias, ' ', sizeof(alias));
	memcpy(alias, base, baseLength);
	memcpy(alias + 8, extension, extensionLength);

	//A name that is its alias exactly, case included, needs no long name
	char formatted[13];
	ShortName(alias, formatted);
	longName = lossy || !(strlen(formatted) == length && memcmp(formatted, name, length) == 0);
	if (!lossy && !ShortTaken(directory, alias))
		return true;

	longName = true;
	for (uint32_t number = 1; number < 1000000; number++)
	{
		char tail[8];
		size_t tailLength = 0;
		for (uint32_t n = number; n != 0; n /= 10)
			tail[tailLength++] = static_cast<char>('0' + n % 10);

		const size_t keep = Min(baseLength, sizeof(base) - 1 - tailLength);
		memset(alias, ' ', 8);
		memcpy(alias, base, keep);
		alias[keep] = '~';
		for (size_t i = 0; i < tailLength; i++)
			alias[keep + 1 + i] = static_cast<uint8_t>(tail[tailLength - 1 - i]);

		if (!ShortTaken(directory, alias))
			return true;
	}
	return false;
}

bool Fat32::FreeSlots(const uint32_t directory, const size_t count, location_t* const slots)
{
	size_t run = 0;
	uint32_t last = directory;
	const bool scanned = Scan(directory, [&](const uint8_t* const entry, const location_t at) -> bool
	{
		if (entry[0] != EntryEnd && entry[0] != EntryDeleted)
		{
			run = 0;
			return true;
		}

		slots[run++] = at;
		return run < count;
	}, &last);
	if (!scanned)
		return false;

	//The run goes on into new clusters, all free, after the chain's last
	while (run < count)
	{
		const uint32_t cluster = Allocate(last + 1);
		if (!cluster || !ZeroCluster(cluster) || !SetFat(last, cluster))
			return false;

		for (uint32_t s = 0; s < m_sectorsPerCluster && run < count; s++)
		{
			for (size_t offset = 0; offset < m_sectorSize && run < count; offset += EntrySize)
				slots[run++] = { ClusterSector(cluster) + s, static_cast<uint32_t>(offset) };
		}
		last = cluster;
	}
	return true;
}

bool Fat32::AddEntries(const uint32_t directory, const char* const name, const size_t length, const uint8_t (&shortEntry)[32], entry_t& added)
{
	uint8_t alias[11];
	bool longName;
	if (!MakeAlias(directory, name, length, alias, longName))
		return false;

	const size_t parts = longName ? (length + LongNameChars - 1) / LongNameChars : 0;
	added = {};
	added.Count = parts + 1;
	if (!FreeSlots(directory, added.Count, added.Slots))
		return false;

	//The alias is upper case, so the case flags Windows keeps in byte 12 are cleared
	uint8_t entry[EntrySize];
	memcpy(entry, shortEntry, sizeof(entry));
	memcpy(entry, alias, sizeof(alias));
	entry[12] = 0;

	//Long name entries last part first, each with 13 characters, the name's end marked by a zero and the rest padded
	const uint8_t checksum = ShortChecksum(entry);
	for (size_t i = 0; i < parts; i++)
	{
		const size_t ordinal = parts - i;
		uint8_t* const sector = m_cache->Modify(added.Slots[i].Sector);
		if (!sector)
			return false;

		uint8_t* const part = sector + added.Slots[i].Offset;
		memset(part, 0, EntrySize);
		part[0] = static_cast<uint8_t>(ordinal | (i == 0 ? LongNameLast : 0));
		part[11] = AttrLongName;
		part[13] = checksum;
		for (size_t k = 0; k < LongNameChars; k++)
		{
			const size_t index = (ordinal - 1) * LongNameChars + k;
			const uint16_t unit = index < length ? static_cast<uint8_t>(name[index]) : (index == length ? 0x0000 : 0xFFFF);
			PutLittleEndian<uint16_t>(part + LongNameOffsets[k], unit);
		}
	}

	const location_t at = added.Slots[parts];
	uint8_t* const sector = m_cache->Modify(at.Sector);
	if (!sector)
		return false;
	memcpy(sector + at.Offset, entry, sizeof(entry));

	added.Cluster = (static_cast<uint32_t>(LittleEndian<uint16_t>(entry + 20)) << 16) | LittleEndian<uint16_t>(entry + 26);
	added.Size = LittleEndian<uint32_t>(entry + 28);
	added.Attributes = entry[11];
	added.Directory = (entry[11] & AttrDirectory) != 0;
	return true;
}

bool Fat32::SetEntry(const location_t at, const uint32_t cluster, const uint32_t size)
{
	uint8_t* const sector = m_cache->Modify(at.Sector);
	if (!sector)
		return false;

	uint8_t* const entry = sector + at.Offset;
	PutLittleEndian<uint16_t>(entry + 20, static_cast<uint16_t>(cluster >> 16));
	PutLittleEndian<uint16_t>(entry + 26, static_cast<uint16_t>(cluster));
	PutLittleEndian<uint32_t>(entry + 28, size);
	return true;
}

bool Fat32::Commit()
{
	if (m_fsInfo != 0)
	{
		uint8_t* const info = m_cache->Modify(m_fsInfo);
		if (!info)
			return false;
		PutLittleEndian<uint32_t>(info + FsInfoFreeCount, m_freeCount);
		PutLittleEndian<uint32_t>(info + FsInfoNextFree, m_nextFree);
	}
	return m_cache->Flush();
}
