#pragma once
#include <cstdint>

namespace crt
{
	typedef uint16_t CHAR16;

	//Memory Routines
	void* memcpy(void* _Dst, const void* _Src, size_t _Size);
	void* memset(void* _Dst, int _Val, size_t _Size);
	int memcmp(void const* _Buf1, void const* _Buf2, size_t _Size);
	void* memmove(void* _Dst, void const* _Src, size_t _Size);

	//Wide String (UEFI)
	uint32_t strcpy(CHAR16* dest, const CHAR16* source);
	size_t strlen(const CHAR16* str);
	uint32_t strncpy(CHAR16* dest, const CHAR16* source, uint32_t num);
	int strcmp(const CHAR16* str1, const CHAR16* str2);
	void strrev(CHAR16* str);

	//String
	size_t strlen(char const* _Str);
	int strcmp(char const* _Str1, char const* _Str2);

	//TODO: another file?
	//Paths
	void GetDirectoryName(const CHAR16* source, CHAR16* destination);
};
