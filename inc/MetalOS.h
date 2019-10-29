#pragma once

//Public facing header

//TODO: clean
typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;

//Types
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;

typedef UINT16          CHAR16;

//PixelBlueGreenRedReserved8BitPerColor
struct BGRRPixel
{	
	UINT32 Blue : 8;
	UINT32 Green : 8;
	UINT32 Red : 8;
	UINT32 Reserved : 8;
};
static_assert(sizeof(BGRRPixel) == sizeof(UINT32), "Pixel greater than UINT32 in size");
typedef struct BGRRPixel Color;



struct Point2D
{
	UINT32 X;
	UINT32 Y;
};

struct Rectangle
{
	Point2D P1;
	Point2D P2;
};

struct ProcessInfo
{
	UINT32 Id;
};

enum SystemCallResult : UINT32
{
	Success = 0,
	Failed
};

enum SystemCall : UINT32
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