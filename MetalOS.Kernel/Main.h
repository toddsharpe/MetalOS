#pragma once

#include "msvc.h"
#include <cstdarg>
#include <efi.h>
#include <LoaderParams.h>
#include "Kernel.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Fatal(x) kernel.Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); 
#define Trace() kernel.Print(__FILE__ "-" STR(__LINE__));

extern "C" void main(LOADER_PARAMS* loader);

void* operator new(size_t n);
void operator delete(void* p);
void operator delete(void* p, size_t n);

void* malloc(size_t size);
void free(void* ptr);

void Print(const char* format, ...);

extern "C" void __declspec(noreturn) Bugcheck(const char* file, const char* line, const char* assert);

extern Kernel kernel;
