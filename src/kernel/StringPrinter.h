#pragma once


#include <string>

class StringPrinter
{
public:
	virtual void Write(const std::string& string) = 0;
	
	void Printf(const char* format, ...);
	void Printf(const char* format, const va_list args);

	void PrintBytes(const char* buffer, const size_t length);
};

