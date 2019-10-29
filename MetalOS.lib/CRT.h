#pragma once
#include "MetalOS.h"

class CRT
{
public:
	CRT() = delete;

	//Memory Routines
	static void memcpy(void* dest, const void* source, UINT32 size);
	static void memset(void* dest, UINT32 c, UINT32 size);
	static int memcmp(const void* ptr1, const void* ptr2, UINT32 num);

	//String
	static UINT32 strcpy(CHAR16* dest, const CHAR16* source);
	static UINT32 strlen(const CHAR16* str);
	static UINT32 strncpy(CHAR16* dest, const CHAR16* source, UINT32 num);
	static void strrev(CHAR16* str);

	//TODO: another file?
	//Paths
	static void GetDirectoryName(const CHAR16* source, CHAR16* destination);
};


