#pragma once

#include "Boot/EfiPrint.h"

extern EFI_SYSTEM_TABLE * ST;
extern EFI_BOOT_SERVICES* BS;

static constexpr size_t MaxBuffer = 256;

EFI_STATUS UartPrintf(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);
	CHAR16 wideBuffer[MaxBuffer] = { };
	vswprintf(wideBuffer, MaxBuffer, format, args);
	va_end(args);

	char convert[MaxBuffer / 2] = { };
	wcstombs(convert, wideBuffer, sizeof(convert));

	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(convert);
	return EFI_SUCCESS;
}

void UartPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[MaxBuffer] = { 0 };
	int retval = vsnprintf(buffer, sizeof(buffer), format, args);
	buffer[retval] = '\0';
	va_end(args);

	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(buffer);
}

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

	char convert_buffer[MaxBuffer / 2] = { 0 };
	wcstombs(convert_buffer, buffer, sizeof(convert_buffer));

	//Write to UART
	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(convert_buffer);

	return EFI_SUCCESS;
}

void Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[MaxBuffer / 2] = { 0 };
	int retval = vsnprintf(buffer, sizeof(buffer), format, args);
	buffer[retval] = '\0';
	va_end(args);

	CHAR16 wide[MaxBuffer] = { 0 };
	mbstowcs(wide, buffer, MaxBuffer);

	//Write to UEFI
	ST->ConOut->OutputString(ST->ConOut, wide);

	//Write to UART
	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(buffer);
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	Printf("Bugcheck\r\n");
	Printf("%s\r\n%s\r\n", file, line);

	va_list args;
	va_start(args, format);
	Printf(format, args);
	Printf("\r\n");
	va_end(args);

	constexpr UINTN Stall = 1000 * 1000 * 60;//60 minute stall
	BS->Stall(Stall);
}
