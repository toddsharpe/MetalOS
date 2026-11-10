#pragma once

#include "kernel/Types.h"

//A tree of files on some storage, such as FAT32 on a disk partition
class FileSystem
{
public:
	//False if there's no such file (or, to create it, no such directory), or this file system can't open it that way
	virtual bool Open(KFile& file, const char* const path, const KFileAccess access, const KFileCreate create) = 0;

	//Up to bytes from the file's position, which the caller advances; zero at the end
	virtual size_t Read(KFile& file, void* const buffer, const size_t bytes) = 0;

	//At the file's position, which the caller advances, growing the file past its end; fewer bytes when the volume fills
	virtual size_t Write(KFile& file, const void* const buffer, const size_t bytes) = 0;

	//A file, or a directory with nothing in it
	virtual bool Delete(const char* const path) = 0;

	//To a name that isn't taken, in the same directory or another
	virtual bool Rename(const char* const from, const char* const to) = 0;

	virtual bool CreateDirectory(const char* const path) = 0;

	//What was written reaches the medium
	virtual bool Flush() = 0;
};
