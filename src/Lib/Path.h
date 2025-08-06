#pragma once

#include "core_crt/string.h"
#include "core_crt/wchar.h"

void GetDirectoryName(const char* source, char* destination)
{
	size_t current = 0;
	size_t marker = 0;
	while (source[current] != L'\0')
	{
		if (source[current] == L'\\')
			marker = current;
		current++;
	}

	//Include trailing slash
	marker++;

	//Copy to destination
	strncpy(destination, source, marker);

	//Add null
	destination[marker + 1] = L'\0';
}

void GetDirectoryName(const wchar_t* source, wchar_t* destination)
{
	size_t current = 0;
	size_t marker = 0;
	while (source[current] != L'\0')
	{
		if (source[current] == L'\\')
			marker = current;
		current++;
	}

	//Include trailing slash
	marker++;

	//Copy to destination
	wcsncpy(destination, source, marker);

	//Add null
	destination[marker + 1] = L'\0';
}

template<typename T>
const T* GetFileName(const T* path)
{
	if (path == nullptr)
		return nullptr;

	const T* pFileName = path;
	for (const T* pCur = path; *pCur != '\0'; pCur++)
	{
		if (*pCur == '/' || *pCur == '\\')
			pFileName = pCur + 1;
	}

	return pFileName;
}
