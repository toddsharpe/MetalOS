#include "kernel/Api.h"
#include "kernel/DeviceTree.h"
#include "kernel/Drivers/Driver.h"
#include "Assert.h"

extern DeviceTree m_deviceTree;

bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access)
{
	const KDevice* device = m_deviceTree.GetDeviceByClass(KDeviceClass::Storage);
	Assert(device);
	StorageDriver* const storage = (StorageDriver*)device->Driver;
	Assert(storage);
	const Result result = storage->OpenFile(file, path, access);
	file.Driver = storage;

	return result == Result::Success ? true : false;
}

bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	//Printf("KeReadFile: Buffer %p, Bytes %d\n", buffer, bufferSize);

	const StorageDriver* storage = (StorageDriver*)file.Driver;
	Assert(storage);

	const size_t read = storage->ReadFile(file, buffer, bufferSize);
	file.Position += read;

	if (bytesRead != nullptr)
		*bytesRead = read;
	return true;
}

bool KeSetFilePosition(KFile& file, const size_t position)
{
	if (position >= file.Length)
		return false;

	file.Position = position;
	return true;
}

void* KeLoadFile(const char* const path)
{
	KFile file = {};
	Assert(KeCreateFile(file, path, KFileAccess::Read));

	void* const address = KeVirtualAlloc(file.Length);
	Assert(KeReadFile(file, address, file.Length, nullptr));
	return address;
}
