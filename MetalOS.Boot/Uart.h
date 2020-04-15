#pragma once

#include <cstdint>
#define GNU_EFI_SETJMP_H
#include <efi.h>

enum ComPort : uint16_t
{
	Com1 = 0x3F8,
	Com2 = 0x2F8,
};

class Uart
{
public:
	Uart(ComPort port);

	//EFI_STATUS Print(const CHAR16* format, ...);
	void Print(const char* format, ...);
	void Write(const char* buffer, size_t length);

private:
	ComPort m_port;
};

