#pragma once

#include "core_crt/stdint.h"
#include "core_crt/stdlib.h"
#include "x64/intrin.h"

class DirectUart
{
public:
	enum class ComPort : uint16_t
	{
		Com1 = 0x3F8,
		Com2 = 0x2F8,
	};

	
	DirectUart(ComPort port) : m_port(port)
	{

	}

	void Write(const char* buffer)
	{
		while (*buffer != '\0')
		{
			__outbyte(static_cast<uint16_t>(m_port), static_cast<uint8_t>(*buffer));
			buffer++;
		}
	}

private:
	const ComPort m_port;
};
