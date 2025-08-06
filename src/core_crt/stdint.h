#pragma once

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;

//NOTE(tsharpe): X64 only
typedef unsigned __int64  uintptr_t;
typedef unsigned __int64 size_t;
typedef __int64         ssize_t;
typedef __int64          ptrdiff_t;
typedef __int64          intptr_t;

#ifdef __cplusplus
static constexpr uint64_t UINT64_MAX = 0xFFFF'FFFF'FFFF'FFFF;
#else
#define UINT64_MAX 0xffffffffffffffffui64
#endif

// Provide a typedef for wchar_t for use under /Zc:wchar_t-
#ifndef _WCHAR_T_DEFINED
    #define _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
#endif

#ifndef NULL
#ifdef __cplusplus
/*C++ NULL definition*/
#define NULL 0
#else
/*C NULL definition*/
#define NULL ((void *)0)
#endif
#endif

#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64
