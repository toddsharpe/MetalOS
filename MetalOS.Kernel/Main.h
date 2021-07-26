#pragma once

#include "msvc.h"
#include <cstdarg>
#include <efi.h>
#include <LoaderParams.h>
#include "Kernel.h"

extern "C" void main(LOADER_PARAMS* loader);

void* operator new(size_t n);
void operator delete(void* p);
void operator delete(void* p, size_t n);

void* malloc(size_t size);
void free(void* ptr);

void Print(const char* format, ...);

extern "C" void Bugcheck(const char* file, const char* line, const char* assert);
