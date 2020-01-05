#pragma once

#include "crt_string.h"
#include "crt_wchar.h"
#include <type_traits>

template<typename T>
void GetDirectoryName(const T* source, T* destination)
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
	if constexpr(std::is_same<char, T>::value)
		strncpy(destination, source, marker);
	else
		wcsncpy(destination, source, marker);

	//Add null
	destination[marker + 1] = L'\0';
}
