#include "StringPrinter.h"
#include <crt_stdio.h>

void StringPrinter::Printf(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	this->Printf(format, args);
	va_end(args);
}

void StringPrinter::Printf(const char* format, va_list args)
{
	char buffer[255];

	int retval = crt_vsprintf(buffer, format, args);
	buffer[retval] = '\0';

	this->Write(buffer);
}

void StringPrinter::PrintBytes(const char* buffer, const size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (i != 0 && i % 32 == 0)
			this->Printf("\n");

		this->Printf("%02x ", (unsigned char)buffer[i]);
	}
	this->Printf("\n");
}
