#pragma once

#include <crt_stdlib.h>

template<typename T>
void GetDirectoryName(const T* source, T* destination);
template void GetDirectoryName<char>(const char* source, char* destination);
template void GetDirectoryName<CHAR16>(const CHAR16* source, CHAR16* destination);

template<typename T>
const T* GetFileName(const T* path);
template const char* GetFileName<char>(const char* path);
template const CHAR16* GetFileName<CHAR16>(const CHAR16* path);
