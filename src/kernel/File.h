#pragma once

#include "kernel/Types.h"

class FileSystem;

//Where paths resolve: FAT32 on the boot disk
void KeMountRoot(FileSystem& fileSystem);

//The first disk's partitions from its GPT, and its first basic data partition (MetalOS) as the FAT32 root
void KeMountStorage();

//Each successful create is closed once; closing a file never opened does nothing
bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access, const KFileCreate create);
void KeCloseFile(KFile& file);
bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead);

//False if fewer bytes went than asked, when the volume fills
bool KeWriteFile(KFile& file, const void* buffer, const size_t bufferSize, size_t* bytesWritten);
bool KeSetFilePosition(KFile& file, const size_t position);

bool KeDeleteFile(const char* const path);
bool KeMoveFile(const char* const from, const char* const to);
bool KeCreateDirectory(const char* const path);
bool KeListDirectory(const char* const path, const KListVisit visit, void* const context);
void* KeLoadFile(const char* const path); //Null if the file is missing or empty, or there's no memory for it

//Closes a file the kernel opened when it goes out of scope, whichever way that happens
class KFileScope
{
public:
	KFileScope(KFile& file) : m_file(file)
	{

	}

	~KFileScope()
	{
		KeCloseFile(m_file);
	}

private:
	KFile& m_file;
};
