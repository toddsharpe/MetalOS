#pragma once

#include "MetalOS.crt.h"
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

//UEFI definition of CHAR16
//Might be a way to tell the compiler to use uint16_t for wchar_t to make this more "standards compliant"
typedef uint16_t CHAR16;

//Wide String
int wcscmp(const CHAR16* str1, const CHAR16* str2);
uint32_t wcscpy(CHAR16* dest, const CHAR16* source);
size_t wcslen(const CHAR16* str);
uint32_t wcsncpy(CHAR16* dest, const CHAR16* source, uint32_t num);
void wcsrev(CHAR16* str); //Bonus function not in crt

int swprintf(CHAR16* const _Buffer, CHAR16 const* const _Format, ...);
int vwprintf(CHAR16* const _Buffer, CHAR16 const* const _Format, va_list _ArgList); //TODO: vector includes this header and MSVC chooses it for some reason.


#ifdef __cplusplus
}
#endif
