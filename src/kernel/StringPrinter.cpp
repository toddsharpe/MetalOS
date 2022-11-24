#include "StringPrinter.h"

#include <stdarg.h>

void StringPrinter::Printf(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	this->Printf(format, args);
	va_end(args);
}

void StringPrinter::Printf(const char* format, const va_list args)
{
	char buffer[1024];

	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';

	this->Write(buffer);
}

void StringPrinter::PrintBytes(const char* buffer, const size_t length)
{
	const uint32_t width = 16;
	
	//Print headers
	this->Printf("A: 0x%016x S: 0x%016x\n", buffer, length);
	this->Printf("---- ");
	for (size_t i = 0; i < width; i++)
	{
		this->Printf("%02x ", (unsigned char)i);
	}
	this->Printf("\n");

	char line[width] = { 0 };
	for (size_t i = 0; i < length; i++)
	{
		if (i != 0 && i % width == 0)
		{
			//Print characters
			this->Printf(" | ");
			for (const auto c : line)
			{
				this->Printf("%c", isprint(c) ? c : '.');
			}

			memset(line, 0, width);
			this->Printf("\n");
		}

		if (i % width == 0)
			this->Printf("%02x - ", (i / width) << 4);

		this->Printf("%02x ", (unsigned char)buffer[i]);
		line[i % width] = (unsigned char)buffer[i];
	}

	//Print the rest of the line
	if ((length % width) != 0)
		for (size_t i = 0; i < width - (length % width); i++)
		{
			this->Printf("   ");
		}

	//Print characters
	this->Printf(" | ");
	for (size_t i = 0; i < (length % width); i++)
	{
		char c = line[i];
		this->Printf("%c", isprint(c) ? c : '.');
	}

	this->Printf("\n");
}
