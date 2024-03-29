#pragma once

#include "msvc.h"
#include <string>

class StringPrinter
{
public:
	virtual void Write(const char* string) = 0;
	
	void Printf(const char* format, ...);
	void Printf(const char* format, va_list args);

	void PrintBytes(const char* buffer, const size_t length);
};

