#include "kernel/Vmem.h"
#include "kernel/File.h"
#include "kernel/FileSystems/FileSystem.h"
#include "kernel/Devices/DeviceTree.h"
#include "kernel/Devices/Driver.h"
#include "kernel/Devices/Gpt.h"
#include "kernel/Devices/Partition.h"
#include "kernel/Kernel.h"
#include "Assert.h"

static FileSystem* m_root = nullptr;
static Gpt::table_t m_table = {};
static Partition m_partitions[Gpt::MaxPartitions] = {};

void KeMountRoot(FileSystem& fileSystem)
{
	Assert(!m_root);
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

	for (size_t i = 0; i < m_table.Count; i++)
	{
		const Gpt::partition_t& entry = m_table.Partitions[i];
		m_partitions[i] = Partition(*disk, entry.FirstSector, entry.SectorCount);

		const char* const type = Gpt::SameGuid(entry.Type, Gpt::EfiSystem) ? "EFI system" : Gpt::SameGuid(entry.Type, Gpt::BasicData) ? "basic data" : "other";
		Printf("Storage: disk 0 partition %u: \"%s\" (%s), %u MB at sector %u\n", static_cast<uint32_t>(i), entry.Name, type,
			static_cast<uint32_t>((entry.SectorCount * disk->SectorSize()) >> 20), static_cast<uint32_t>(entry.FirstSector));
	}
}

bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access)
{
	//Nothing is mounted before the device tree comes up, so an early bugcheck's PDB lookup finds nothing rather than faulting again
	if (!m_root || !m_root->Open(file, path, access))
		return false;

	file.FileSystem = m_root;
	return true;
}

bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	//Printf("KeReadFile: Buffer %p, Bytes %d\n", buffer, bufferSize);

	Assert(file.FileSystem);
	const size_t read = file.FileSystem->Read(file, buffer, bufferSize);
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
