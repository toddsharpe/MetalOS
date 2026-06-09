#pragma once

#include "Lib/System.h"
#include "Lib/List.h"
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

class Driver;
struct KFile
{
	void* Context;
	Driver* Driver;
	size_t Position;
	size_t Length;
	KFileAccess Access;
};

struct KModule
{
	StaticString<64> Name;
	void* ImageBase;
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
	StaticString<256> Function;
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
