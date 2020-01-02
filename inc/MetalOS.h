#pragma once
#include <cstdint>

//Public facing header

//PixelBlueGreenRedReserved8BitPerColor
struct BGRRPixel
{	
	uint32_t Blue : 8;
	uint32_t Green : 8;
	uint32_t Red : 8;
	uint32_t Reserved : 8;
};
static_assert(sizeof(BGRRPixel) == sizeof(uint32_t), "Pixel greater than UINT32 in size");
typedef struct BGRRPixel Color;

struct Point2D
{
	uint32_t X;
	uint32_t Y;
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

enum SystemCallResult : uint32_t
{
	Success = 0,
	Failed
};

enum SystemCall : uint32_t
{
	GetProcessInformation = 1,
};

#if !defined(EXPORT)
#define SYSTEMCALL __declspec(dllimport) SystemCallResult
#else
#define SYSTEMCALL __declspec(dllexport) SystemCallResult
#endif

//Prototypes
SYSTEMCALL GetProcessInfo(ProcessInfo* info);