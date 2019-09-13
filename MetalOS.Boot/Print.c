#include "Print.h"
#include "Common.h"
#include "String.h"
#include "Memory.h"

CHAR16 Char16_Zero = L'0';
CHAR16 Char16_CapA = L'A';

#define GetHexChar(x) ((x) < 10 ? (Char16_Zero + (x)) : (Char16_CapA + ((x) - 10)))
#define MAXBUFFER 255

//TODO: Calculate string length?
EFI_STATUS Print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, CHAR16* format, ...)
{
	EFI_STATUS status;
	
	CHAR16 buffer[MAXBUFFER];
	CHAR16* buffer_ptr = buffer;

	va_list argptr;
	va_start(argptr, format);

	void* arg = NULL;
	CHAR16* start = format;
	while (*format != L'\0')
	{
		if (*format != L'%')
		{
			format++;
			continue;
		}

		//Copy contents to this point
		efi_memcpy(start, buffer_ptr, (format - start) * sizeof(CHAR16));
		buffer_ptr += (format - start);

		//Advance past %
		format++;

		//Process
		switch (*format)
		{
		case L'b':
			ByteToHex(va_arg(argptr, UINT8), buffer_ptr);
			buffer_ptr += 2;
			break;

		case L'w':
			WordToHex(va_arg(argptr, UINT16), buffer_ptr);
			buffer_ptr += 4;
			break;

		case L'u':
			DWordToHex(va_arg(argptr, UINT32), buffer_ptr);
			buffer_ptr += 8;
			break;
		}
		
		//Advance past type - assume 1 char types
		format++;

		//Update start
		start = format;
	}

	//Copy end of string
	efi_memcpy(start, buffer_ptr, (format - start) * sizeof(CHAR16));

	//Write
	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer));

	return EFI_SUCCESS;
}

EFI_STATUS PrintUint32Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT32 data)
{
	EFI_STATUS status;

	CHAR16 buffer[8];
	DWordToHex(data, buffer);

	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer));
	return EFI_SUCCESS;
}

EFI_STATUS PrintUint16Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT16 data)
{
	EFI_STATUS status;

	CHAR16 buffer[4];
	WordToHex(data, buffer);

	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer));
	return EFI_SUCCESS;
}

EFI_STATUS PrintUint8Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT8 data)
{
	EFI_STATUS status;

	CHAR16 buffer[2];
	ByteToHex(data, buffer);
	
	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer));
	return EFI_SUCCESS;
}

void DWordToHex(UINT32 data, CHAR16* string)
{
	ASSERT(string != NULL);

	WordToHex(DWordHighWord(data), string);
	WordToHex(DWordLowWord(data), string + 4);
}

void WordToHex(UINT16 data, CHAR16* string)
{
	ASSERT(string != NULL);

	ByteToHex(WordHighByte(data), string);
	ByteToHex(WordLowByte(data), string + 2);
}

void ByteToHex(UINT8 data, CHAR16* string)
{
	ASSERT(string != NULL);

	string[0] = GetHexChar(ByteHighNibble(data));
	string[1] = GetHexChar(ByteLowNibble(data));
}