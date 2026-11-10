#pragma once

#include "kernel/Types.h"

//A tree of files on some storage, such as FAT32 on a disk partition
class FileSystem
{
public:
	//False if there's no such file, or this file system can't open it that way
	virtual bool Open(KFile& file, const char* const path, const KFileAccess access) = 0;

	//Up to bytes from the file's position, which the caller advances; zero at the end
	virtual size_t Read(KFile& file, void* const buffer, const size_t bytes) = 0;
};
