#pragma once

#include <stdint.h>
#include <Graphics/Types.h>

struct ReadOnlyBuffer
{
	const void* Data;
	size_t Length;
};

struct Buffer
{
	void* Data;
	size_t Length;

	ReadOnlyBuffer GetRef()
	{
		return { Data, Length };
	}
};

enum class MemoryAllocationType
{
	Commit = 1,
	Reserve = 2,
	CommitReserve = Commit | Reserve,
};

enum class MemoryProtection
{
	PageRead = 1,
	PageWrite = 2,
	PageExecute = 4,
	PageReadWrite = PageRead | PageWrite,
	PageReadExecute = PageRead | PageExecute,
	PageReadWriteExecute = PageRead | PageWrite | PageExecute
};

typedef void* Handle;
typedef void* HThread;
typedef void* HProcess;
typedef void* HSharedMemory;
typedef void* HRingBuffer;
typedef void* HWindow;
typedef void* HFile;
typedef void* HEvent;

enum class GenericAccess
{
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write
};

enum class StandardHandle
{
	Input,
	Output,
	Error
};

typedef size_t (*ThreadStart)(void* parameter);

constexpr size_t MaxModuleName = 16;
struct Module
{
	void* Address;
	char Name[MaxModuleName];
};

constexpr size_t MaxLoadedModules = 8;
struct ProcessEnvironmentBlock
{
	uint32_t ProcessId;
	uintptr_t BaseAddress;
	Module LoadedModules[MaxLoadedModules];
	size_t ModuleIndex;
};

struct ThreadEnvironmentBlock
{
	ThreadEnvironmentBlock* SelfPointer;
	ProcessEnvironmentBlock* PEB;
	ThreadStart ThreadStart;
	Handle Handle;
	void* Arg;
	uint32_t ThreadId;
	uint32_t Error;
};

typedef uint16_t VirtualKey;


enum class MessageType
{
	KeyEvent,
	MouseEvent,
	PaintEvent,
};

struct MessageHeader
{
	MessageType MessageType;
};

struct KeyEvent
{
	VirtualKey Key;
	struct
	{
		uint16_t Pressed : 1;
	} Flags;
};

struct MouseButtonState
{
	uint8_t LeftPressed : 1;
	uint8_t RightPressed : 1;
};

struct MouseEvent
{
	MouseButtonState Buttons;
	uint16_t XPosition;
	uint16_t YPosition;
};

struct PaintEvent
{
	Graphics::Rectangle Region;
};

struct Message
{
	MessageHeader Header;
	union
	{
		KeyEvent KeyEvent;
		MouseEvent MouseEvent;
		PaintEvent PaintEvent;
	};
};

typedef void (*MemberFunction)(void* arg);
struct CallContext
{
	MemberFunction Handler;
	void* Context;
};

enum class WaitStatus
{
	None,
	Signaled,
	Timeout,
	Abandoned,
	BrokenPipe,
	//Failed conflicts with macro
};

enum class DayOfWeek
{
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

struct SystemTime
{
	uint16_t Year;
	uint16_t Month;
	DayOfWeek DayOfTheWeek;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute;
	uint16_t Second;
	uint16_t Milliseconds;
};
