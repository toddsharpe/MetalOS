#include "Output.h"
#include "Uart.h"
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

extern EFI_SYSTEM_TABLE* ST;

static constexpr size_t MaxBuffer = 256;

EFI_STATUS Print(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);

	EFI_STATUS status;
	CHAR16 buffer[MaxBuffer] = { 0 };
	vswprintf(buffer, MaxBuffer, format, args);
	va_end(args);

	//Write to UEFI
	if (EFI_ERROR((status = ST->ConOut->OutputString(ST->ConOut, buffer))))
		return status;

	//Write to UART
	Uart uart(ComPort::Com1);
	uart.Write(buffer);

	return EFI_SUCCESS;
}

EFI_STATUS UartPrintf(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);
	CHAR16 wideBuffer[MaxBuffer] = { 0 };
	vswprintf(wideBuffer, MaxBuffer, format, args);
	va_end(args);

	Uart uart(ComPort::Com1);
	uart.Write(wideBuffer);
	return EFI_SUCCESS;
}

void UartPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[MaxBuffer] = { 0 };
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	Uart uart(ComPort::Com1);
	uart.Write(buffer);
}

extern "C" void Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[MaxBuffer / 2] = { 0 };
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	CHAR16 wide[MaxBuffer] = { 0 };
	mbstowcs(wide, buffer, MaxBuffer);

	//Write to UEFI
	ST->ConOut->OutputString(ST->ConOut, wide);

	//Write to UART
	Uart uart(ComPort::Com1);
	uart.Write(buffer);
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	Printf("Init Bugcheck\r\n");
	Printf("%s\r\n%s\r\n", file, line);

	va_list args;
	va_start(args, format);
	Printf(format, args);
	Printf("\r\n");
	va_end(args);

	while (true);
}
