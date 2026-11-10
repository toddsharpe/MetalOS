#include "kernel/Vmem.h"
#include "kernel/File.h"
#include "kernel/Thread.h"
#include "kernel/FileSystems/FileSystem.h"
#include "kernel/FileSystems/Fat32.h"
#include "kernel/FileSystems/SectorCache.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/Devices/DeviceTree.h"
#include "kernel/Devices/Driver.h"
#include "kernel/Devices/Gpt.h"
#include "kernel/Devices/Partition.h"
#include "kernel/Kernel.h"
#include "Assert.h"

static FileSystem* m_root = nullptr;
static Gpt::table_t m_table = {};
static Partition m_partitions[Gpt::MaxPartitions] = {};
static SectorCache m_osCache;
static Fat32 m_os;

//File systems keep state between calls (FAT32's sector cache and open files), so calls into one take turns: an auto-reset event that starts signaled
static KEvent m_lock(false, true);

void KeMountRoot(FileSystem& fileSystem)
{
	m_root = &fileSystem;
}

void KeMountStorage()
{
	//A Storage device's driver is a StorageDriver
	const KDevice* const device = m_deviceTree.GetDeviceByClass(KDeviceClass::Storage);
	BlockDevice* const disk = device ? static_cast<StorageDriver*>(device->Driver)->GetDisk(0) : nullptr;
	if (!disk)
	{
		Printf("Storage: no disk\n");
		return;
	}

	if (!Gpt::Read(*disk, m_table))
	{
		Printf("Storage: disk 0 has no GPT that checks out\n");
		return;
	}

	size_t os = Gpt::MaxPartitions;
	for (size_t i = 0; i < m_table.Count; i++)
	{
		const Gpt::partition_t& entry = m_table.Partitions[i];
		m_partitions[i] = Partition(*disk, entry.FirstSector, entry.SectorCount);

		const bool isData = Gpt::SameGuid(entry.Type, Gpt::BasicData);
		if (isData && os == Gpt::MaxPartitions)
			os = i;

		const char* const type = isData ? "basic data" : Gpt::SameGuid(entry.Type, Gpt::EfiSystem) ? "EFI system" : "other";
		Printf("Storage: disk 0 partition %u: \"%s\" (%s), %u MB at sector %u\n", static_cast<uint32_t>(i), entry.Name, type,
			static_cast<uint32_t>((entry.SectorCount * disk->SectorSize()) >> 20), static_cast<uint32_t>(entry.FirstSector));
	}

	//The OS and its data are on the first basic data partition, a bare name like term.exe at its root; the EFI system partition holds only what the loader loads
	if (os == Gpt::MaxPartitions || !m_os.Mount(m_partitions[os], m_osCache))
	{
		Printf("Storage: no FAT32 basic data partition, so no file system\n");
		return;
	}

	KeMountRoot(m_os);
	Printf("Storage: FAT32 on partition %u (\"%s\") is the root\n", static_cast<uint32_t>(os), m_table.Partitions[os].Name);
}

bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access, const KFileCreate create)
{
	//Nothing is mounted before the device tree comes up, so an early bugcheck's PDB lookup finds nothing rather than faulting again
	if (!m_root)
		return false;

	KeWait(m_lock);
	FileSystem* const root = m_root;
	const bool opened = root->Open(file, path, access, create);
	m_lock.Set();
	if (!opened)
		return false;

	file.FileSystem = root;
	return true;
}

void KeCloseFile(KFile& file)
{
	if (!file.FileSystem)
		return;

	KeWait(m_lock);
	file.FileSystem->Close(file);
	m_lock.Set();
	file.FileSystem = nullptr;
}

bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	Assert(file.FileSystem);
	KeWait(m_lock);
	const size_t read = file.FileSystem->Read(file, buffer, bufferSize);
	m_lock.Set();
	file.Position += read;

	if (bytesRead != nullptr)
		*bytesRead = read;
	return true;
}

bool KeWriteFile(KFile& file, const void* buffer, const size_t bufferSize, size_t* bytesWritten)
{
	Assert(file.FileSystem);
	KeWait(m_lock);

	//An append goes at the end, wherever the position was moved to
	if ((file.Access & KFileAccess::Append) == KFileAccess::Append)
		file.Position = file.Length;
	const size_t written = file.FileSystem->Write(file, buffer, bufferSize);
	m_lock.Set();
	file.Position += written;

	if (bytesWritten != nullptr)
		*bytesWritten = written;
	return written == bufferSize;
}

bool KeSetFilePosition(KFile& file, const size_t position)
{
	//The end itself is a valid position (reads there return nothing), so SEEK_END and ftell at end of file work; past it only for a write, which fills the gap with zeros
	if (position > file.Length && (file.Access & KFileAccess::Write) == KFileAccess::None)
		return false;

	file.Position = position;
	return true;
}

bool KeDeleteFile(const char* const path)
{
	if (!m_root)
		return false;

	KeWait(m_lock);
	const bool deleted = m_root->Delete(path);
	m_lock.Set();
	return deleted;
}

bool KeMoveFile(const char* const from, const char* const to)
{
	if (!m_root)
		return false;

	KeWait(m_lock);
	const bool moved = m_root->Rename(from, to);
	m_lock.Set();
	return moved;
}

bool KeCreateDirectory(const char* const path)
{
	if (!m_root)
		return false;

	KeWait(m_lock);
	const bool created = m_root->CreateDirectory(path);
	m_lock.Set();
	return created;
}

bool KeListDirectory(const char* const path, const KListVisit visit, void* const context)
{
	if (!m_root)
		return false;

	KeWait(m_lock);
	const bool listed = m_root->List(path, visit, context);
	m_lock.Set();
	return listed;
}

void* KeLoadFile(const char* const path)
{
	KFile file = {};
	if (!KeCreateFile(file, path, KFileAccess::Read, KFileCreate::OpenExisting))
		return nullptr;

	KFileScope scope(file);
	if (file.Length == 0)
		return nullptr;

	void* const address = KeVirtualAlloc(file.Length);
	if (!address)
		return nullptr;

	Assert(KeReadFile(file, address, file.Length, nullptr));
	return address;
}
