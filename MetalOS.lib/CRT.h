#pragma once
#include "MetalOS.h"

class CRT
{
public:
	CRT() = delete;

	//Memory Routines
	static void* memcpy(void* dest, const void* source, UINT32 size);
	static void* memset(void* dest, UINT32 c, UINT32 size);
	static int memcmp(const void* ptr1, const void* ptr2, UINT32 num);
	static void* memmove(void* dest, const void* source, UINT32 size);

	//Wide String (UEFI)
	static UINT32 strcpy(CHAR16* dest, const CHAR16* source);
	static UINT32 strlen(const CHAR16* str);
	static UINT32 strncpy(CHAR16* dest, const CHAR16* source, UINT32 num);
	static UINT32 strcmp(const CHAR16* str1, const CHAR16* str2);
	static void strrev(CHAR16* str);

	//String
	static UINT32 strlen(const char* str);
	static UINT32 strcmp(const char* str1, const char* str2);

	//TODO: another file?
	//Paths
	static void GetDirectoryName(const CHAR16* source, CHAR16* destination);
};
