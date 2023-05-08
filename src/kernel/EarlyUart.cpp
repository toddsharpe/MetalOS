#include "EarlyUart.h"
#include <intrin.h>

EarlyUart::EarlyUart(ComPort port) : m_port(port)
{

}

void EarlyUart::Write(const std::string& string)
{
	for (char c : string)
	{
		__outbyte(static_cast<uint16_t>(m_port), static_cast<uint8_t>(c));
	}
}
