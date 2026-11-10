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
static SectorCache m_espCache;
static Fat32 m_esp;

//File systems keep state between calls (FAT32's sector cache), so opens and reads take turns: an auto-reset event that starts signaled
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

	size_t esp = Gpt::MaxPartitions;
	for (size_t i = 0; i < m_table.Count; i++)
	{
		const Gpt::partition_t& entry = m_table.Partitions[i];
		m_partitions[i] = Partition(*disk, entry.FirstSector, entry.SectorCount);

		const bool isEsp = Gpt::SameGuid(entry.Type, Gpt::EfiSystem);
		if (isEsp && esp == Gpt::MaxPartitions)
			esp = i;

		const char* const type = isEsp ? "EFI system" : Gpt::SameGuid(entry.Type, Gpt::BasicData) ? "basic data" : "other";
		Printf("Storage: disk 0 partition %u: \"%s\" (%s), %u MB at sector %u\n", static_cast<uint32_t>(i), entry.Name, type,
			static_cast<uint32_t>((entry.SectorCount * disk->SectorSize()) >> 20), static_cast<uint32_t>(entry.FirstSector));
	}

	//The boot files are in the EFI system partition's \EFI\BOOT, so a bare name like term.exe resolves there
	if (esp == Gpt::MaxPartitions || !m_esp.Mount(m_partitions[esp], m_espCache) || !m_esp.SetDirectory("\\EFI\\BOOT"))
	{
		Printf("Storage: no FAT32 EFI system partition with \\EFI\\BOOT, so no file system\n");
		return;
	}

	KeMountRoot(m_esp);
	Printf("Storage: FAT32 on partition %u is the root, bare names from \\EFI\\BOOT\n", static_cast<uint32_t>(esp));
}

bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access)
{
	//Nothing is mounted before the device tree comes up, so an early bugcheck's PDB lookup finds nothing rather than faulting again
	if (!m_root)
		return false;

	KeWait(m_lock);
	FileSystem* const root = m_root;
	const bool opened = root->Open(file, path, access);
	m_lock.Set();
	if (!opened)
		return false;

	file.FileSystem = root;
	return true;
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

bool KeSetFilePosition(KFile& file, const size_t position)
{
	//The end itself is a valid position (reads there return nothing), so SEEK_END and ftell at end of file work
	if (position > file.Length)
		return false;

	file.Position = position;
	return true;
}

void* KeLoadFile(const char* const path)
{
	KFile file = {};
	if (!KeCreateFile(file, path, KFileAccess::Read) || file.Length == 0)
		return nullptr;

	void* const address = KeVirtualAlloc(file.Length);
	if (!address)
		return nullptr;

	Assert(KeReadFile(file, address, file.Length, nullptr));
	return address;
}
