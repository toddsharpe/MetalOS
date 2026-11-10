#pragma once

#include "Lib/System.h"
#include "Lib/String.h"
#include "Assert.h"

typedef uint32_t (*KThreadStart)(void* const arg);
typedef uint32_t (*UThreadStart)(void* const arg);

enum class KWaitResult
{
	None,
	Signaled,
	Timeout
};

enum class KFileAccess
{
	None = 0,
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write
};

inline KFileAccess operator&(KFileAccess lhs, KFileAccess rhs)
{
	return static_cast<KFileAccess>(
		static_cast<size_t>(lhs) &
		static_cast<size_t>(rhs)
	);
}

class FileSystem;
struct KFile
{
	void* Context;
	FileSystem* FileSystem;
	size_t Position;
	size_t Length;
	KFileAccess Access;

	//The file system's own: FAT32's first cluster, how many clusters run on consecutively from it, and the cluster its last read reached with that cluster's index in the chain
	uint32_t Start;
	uint32_t Run;
	uint32_t Cursor;
	uint32_t CursorIndex;
};

struct KModule
{
	char Name[64];
	void* ImageBase;
	size_t ImageSize; //SizeOfImage as loaded, so finding an IP's module never reads its (process-writable) headers
};

struct guid_t
{
	uint32_t  Data1;
	uint16_t  Data2;
	uint16_t  Data3;
	uint8_t   Data4[8];
};

struct PdbLookup
{
	char Function[256];
	size_t Line;
};

typedef void (*Action)(void* arg);
struct ActionContext
{
	Action Action;
	void* Context;

	void Invoke() const
	{
		Assert(Action);
		Action(Context);
	}

	bool IsEmpty()
	{
		return Action == nullptr && Context == nullptr;
	}
};
