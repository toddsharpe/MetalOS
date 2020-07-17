#pragma once

#include <cstdint>
#include <cstdarg>

enum ComPort : uint16_t
{
	Com1 = 0x3F8,
	Com2 = 0x2F8,
};

class Uart
{
public:
	Uart(ComPort port);

	void Printf(const char* format, ...);
	void Printf(const char* format, va_list args);
	void Write(const char* buffer, size_t length);

private:
	ComPort m_port;
};

