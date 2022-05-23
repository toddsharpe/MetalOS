#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <efi.h>

enum class ComPort : uint16_t
{
	Com1 = 0x3F8,
	Com2 = 0x2F8,
};

class Uart
{
public:
	Uart(ComPort port);

	void Write(const CHAR16* buffer);
	void Write(const char* buffer);

private:
	static constexpr size_t MaxBuffer = 256;

	const ComPort m_port;
};

