#include "Uart.h"

#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Output.h"

Uart::Uart(ComPort port) : m_port(port)
{

}

void Uart::Write(const CHAR16* buffer)
{
	char convert_buffer[MaxBuffer / 2] = { 0 };
	wcstombs(convert_buffer, buffer, sizeof(convert_buffer));

	this->Write(convert_buffer);
}

void Uart::Write(const char* buffer)
{
	while (*buffer != '\0')
	{
		__outbyte(static_cast<uint16_t>(m_port), static_cast<uint8_t>(*buffer));
		buffer++;
	}
}
