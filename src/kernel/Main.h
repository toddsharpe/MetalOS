#pragma once


#include <cstdarg>
#include "LoaderParams.h"

extern "C" int main(LOADER_PARAMS* loader);

void* operator new(size_t n);
void operator delete(void* p);
void operator delete(void* p, size_t n);

extern "C" void* malloc(size_t size);
extern "C" void free(void* ptr);
void Bugcheck(const char* file, const char* line, const char* format, ...);

void Print(const char* format, ...);

