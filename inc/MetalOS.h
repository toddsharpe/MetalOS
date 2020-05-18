#pragma once
#include <cstdint>

//Public facing header

//PixelBlueGreenRedReserved8BitPerColor
struct BGRRPixel
{	
	union
	{
		struct
		{
			uint32_t Blue : 8;
			uint32_t Green : 8;
			uint32_t Red : 8;
			uint32_t Reserved : 8;
		};
		uint32_t AsUint32;
	};
};
static_assert(sizeof(BGRRPixel) == sizeof(uint32_t), "Pixel greater than UINT32 in size");
typedef struct BGRRPixel Color;

struct Point2D
{
	size_t X;
	size_t Y;
};

struct Rectangle
{
	Point2D P1;
	Point2D P2;
};

struct ProcessInfo
{
	uint32_t Id;
};

enum DayOfWeek : uint16_t
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

enum SystemArchitecture
{
	Unknown = 0,
	x64 = 1
};

struct SystemInfo
{
	uint32_t PageSize;
	SystemArchitecture Architecture;
};

enum SystemCallResult : uint32_t
{
	Success = 0,
	Failed
};

enum SystemCall : uint32_t
{
	GetSystemInfoId = 1,
	GetProcessInfoId = 2,
};

#if !defined(EXPORT)
#define SYSTEMCALL __declspec(dllimport) SystemCallResult
#else
#define SYSTEMCALL __declspec(dllexport) SystemCallResult
#endif

enum WaitStatus
{
	None,
	Signaled,
	Timeout,
	Abandoned,
	//Failed conflicts with macro
};

typedef void* Handle;

#define MAX_PATH 256

extern "C"
{
	//Info
	SYSTEMCALL GetSystemInfo(SystemInfo* info);
	SYSTEMCALL GetProcessInfo(ProcessInfo* info);

	//Semaphores
	Handle CreateSemaphore(size_t initial, size_t maximum, const char* name);
	SYSTEMCALL ReleaseSemaphore(Handle hSemaphore, size_t releaseCount, size_t& previousCount);

	//Virtual
	//SYSTEMCALL void* VirtualAlloc(void* address);
}

typedef uint32_t (*ThreadStart)(void* parameter);
