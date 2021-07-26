#pragma once

#ifndef CONST
#define CONST               const
#endif

//
// Basics
//

typedef char CHAR;
typedef short SHORT;
typedef long LONG;

#ifndef BASETYPES
#define BASETYPES
typedef unsigned long ULONG;
typedef ULONG* PULONG;
typedef unsigned short USHORT;
typedef USHORT* PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR* PUCHAR;
typedef char* PSZ;
#endif  /* !BASETYPES */

/* SAL annotations */
#ifndef IN
#define IN
#define OUT
#define OPTIONAL
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

typedef char CHAR;

#ifndef VOID
#define VOID void
#endif

typedef void* PVOID;
typedef uint8_t UINT8;
typedef uint16_t UINT16;

typedef uint32_t   UINT32;
typedef int32_t    INT32;

typedef uint64_t   UINT64;
typedef int64_t    INT64;

typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

typedef unsigned __int64 ULONG64, * PULONG64;
typedef unsigned __int64 DWORD64, * PDWORD64;

typedef unsigned __int64 DWORD64, * PDWORD64;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT* PFLOAT;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int* PUINT;
typedef const void* PCVOID;

typedef WORD * PWORD;
typedef WORD * LPWORD;
typedef long * LPLONG;
typedef DWORD * PDWORD;
typedef DWORD * LPDWORD;

typedef BYTE* PBYTE;
typedef BYTE* LPBYTE;

//
// Boolean
//

typedef UCHAR BOOLEAN;           // winnt
typedef BOOLEAN* PBOOLEAN;       // winnt

//
// ANSI (Multi-byte Character) types
//
typedef CHAR* PCHAR, * LPCH, * PCH;
typedef CONST CHAR* LPCCH, * PCCH;

typedef struct _STRING {
	USHORT Length;
	USHORT MaximumLength;
	PCHAR Buffer;
} STRING;
typedef STRING* PSTRING;

typedef struct _CSTRING {
	USHORT Length;
	USHORT MaximumLength;
	const char* Buffer;
} CSTRING;
typedef CSTRING* PCSTRING;


// Unicode strings are counted 16-bit character strings. If they are
// NULL terminated, Length does not include trailing NULL.
//
typedef wchar_t* PWSTR;
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

#define NTAPI

typedef __int64 INT_PTR, * PINT_PTR;
typedef unsigned __int64 UINT_PTR, * PUINT_PTR;

typedef __int64 LONG_PTR, * PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, * PULONG_PTR;

#define __int3264   __int64
#define UNALIGNED __unaligned

typedef LONG NTSTATUS;
