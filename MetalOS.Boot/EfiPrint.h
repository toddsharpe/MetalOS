#pragma once

#include <efi.h>
#include <efilib.h>



EFI_STATUS Print(const CHAR16* format, ...);
UINTN SPrint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* fmt, ...);

class EfiPrint
{
public:


	static EFI_STATUS _print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, const CHAR16* format, va_list args);
	static UINTN _sprint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* format, va_list args);
private:

	static UINT32 IntToString(int data, CHAR16* str);
	static void QWordToHex(UINT64 data, CHAR16* string);
	static void DWordToHex(UINT32 data, CHAR16* string);
	static void WordToHex(UINT16 data, CHAR16* string);
	static void ByteToHex(UINT8 data, CHAR16* string);
};


