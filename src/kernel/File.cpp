#include "kernel/Vmem.h"
#include "kernel/File.h"
#include "kernel/FileSystems/FileSystem.h"
#include "Assert.h"

static FileSystem* m_root = nullptr;

void KeMountRoot(FileSystem& fileSystem)
{
	Assert(!m_root);
	m_root = &fileSystem;
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
