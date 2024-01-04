#pragma once

#include <stdint.h>
#include <Graphics/Types.h>
#include <type_traits>

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

typedef void* Handle;
typedef void* HThread;
typedef void* HProcess;
typedef void* HSharedMemory;
typedef void* HRingBuffer;
typedef void* HWindow;
typedef void* HFile;
typedef void* HEvent;

enum GenericAccess
{
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write

};

inline GenericAccess operator&(GenericAccess lhs, GenericAccess rhs)
{
	return static_cast<GenericAccess>(
		static_cast<std::underlying_type<GenericAccess>::type>(lhs) &
		static_cast<std::underlying_type<GenericAccess>::type>(rhs)
	);
}

enum class StandardHandle
{
	Input,
	Output,
	Error
};

typedef size_t (*ThreadStart)(void* parameter);

//TODO(tsharpe): Mapping PDB here 1:1 with a loaded module isn't the same as 1:1 with binary. Optimize later.
constexpr size_t MaxModuleName = 16;
struct Module
{
	void* ImageBase;
	void* PDB;
	char Name[MaxModuleName];
};

constexpr size_t MaxLoadedModules = 8;
struct ProcessEnvironmentBlock
{
	uint32_t ProcessId;
	void* ImageBase;
	int argc;
	char** argv;
	void* PDB;
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
