#pragma once

#include "Boot/EfiPrint.h"

extern EFI_SYSTEM_TABLE * ST;
extern EFI_BOOT_SERVICES* BS;

static constexpr size_t MaxBuffer = 256;

EFI_STATUS UartPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char buffer[MaxBuffer] = { };
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(buffer);
	return EFI_SUCCESS;
}

EFI_STATUS Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	EFI_STATUS status;
	char buffer[MaxBuffer] = { 0 };
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	//mbstowcs doesn't null-terminate, so keep the zeroed last element
	CHAR16 wide[MaxBuffer] = { 0 };
	mbstowcs(wide, buffer, MaxBuffer - 1);

	//Write to UEFI
	if (EFI_ERROR((status = ST->ConOut->OutputString(ST->ConOut, wide))))
		return status;

	//Write to UART
	DirectUart uart(DirectUart::ComPort::Com1);
	uart.Write(buffer);

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
