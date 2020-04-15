#include "Uart.h"

#include <cstdarg>
#include <crt_stdio.h>
#include <intrin.h>
#include <crt_string.h>

Uart::Uart(ComPort port) : m_port(port)
{

}

#define MAXBUFFER 256
void Uart::Print(const char* format, ...)
{
	char buffer[MAXBUFFER] = { 0 };

	va_list args;
	va_start(args, format);
	crt_vsprintf(buffer, format, args);
	va_end(args);

	this->Write(buffer, strlen(buffer));
}

void Uart::Write(const char* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		__outbyte(m_port, static_cast<uint8_t>(buffer[i]));
	}
}
