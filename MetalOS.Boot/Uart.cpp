#include "Uart.h"

#include <intrin.h>
#include <crt_stdio.h>
#include <crt_string.h>

Uart::Uart(ComPort port) : m_port(port)
{

}

void Uart::Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	this->Printf(format, args);
	va_end(args);
}

#define MAXBUFFER 256
void Uart::Printf(const char* format, va_list args)
{
	char buffer[MAXBUFFER] = { 0 };
	vsprintf(buffer, format, args);
	this->Write(buffer, strlen(buffer));
}

void Uart::Write(const char* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		__outbyte(m_port, static_cast<uint8_t>(buffer[i]));
	}
}
