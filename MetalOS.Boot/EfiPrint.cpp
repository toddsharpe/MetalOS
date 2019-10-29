#include "EfiPrint.h"
#include "CRT.h"
#include "Error.h"

CHAR16 Char16_Zero = L'0';
CHAR16 Char16_CapA = L'A';

#define GetHexChar(x) ((x) < 10 ? (Char16_Zero + (x)) : (Char16_CapA + ((x) - 10)))
#define MAXBUFFER 255

#define ByteHighNibble(x) (((UINT8)x) >> 4)
#define ByteLowNibble(x) (((UINT8)x) & 0x0F)

#define WordHighByte(x) (((UINT16)x) >> 8)
#define WordLowByte(x) (((UINT16)x) & 0xFF)

#define DWordHighWord(x) (((UINT32)x) >> 16)
#define DWordLowWord(x) (((UINT32)x) & 0xFFFF)

#define QWordHighDWord(x) (((UINT64)x) >> 32)
#define QWordLowDWord(x) (((UINT64)x) & 0xFFFFFFFF)

EFI_STATUS Print(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);
	EFI_STATUS status = EfiPrint::_print(ST->ConOut, format, args);
	va_end(args);

	return EFI_SUCCESS;
}

UINTN SPrint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	EFI_STATUS status = EfiPrint::_sprint(Str, StrSize, fmt, args);
	va_end(args);

	return EFI_SUCCESS;
}

//TODO: Calculate string length?
EFI_STATUS EfiPrint::_print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, const CHAR16* format, va_list args)
{
	EFI_STATUS status;

	CHAR16 buffer[MAXBUFFER] = { 0 };
	_sprint(buffer, MAXBUFFER, format, args);

	//Write
	if (EFI_ERROR((status = conOut->OutputString(conOut, buffer))))
		return status;

	return EFI_SUCCESS;
}

//TODO: we never check strsize
UINTN EfiPrint::_sprint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* format, va_list args)
{
	CHAR16* buffer_ptr = Str;

	void* arg = nullptr;
	const CHAR16* start = format;
	while (*format != L'\0')
	{
		if (*format != L'%')
		{
			format++;
			continue;
		}

		//Copy contents to this point
		CRT::strncpy(buffer_ptr, start, (format - start));
		buffer_ptr += (format - start);

		//Advance past %
		format++;

		bool zeroes = false;
		if (*format == L'0')
		{
			zeroes = true;
			format++;
		}

		UINT8 size = 0;
		if ((*format >= L'1') && (*format <= L'9'))
		{
			size = *format - Char16_Zero;
			format++;
		}

		//Process
		switch (*format)
		{
		case L'b':
			ByteToHex(va_arg(args, UINT8), buffer_ptr);
			buffer_ptr += 2;
			break;

		case L'w':
			WordToHex(va_arg(args, UINT16), buffer_ptr);
			buffer_ptr += 4;
			break;

		case L'u':
			DWordToHex(va_arg(args, UINT32), buffer_ptr);
			buffer_ptr += 8;
			break;

		case L'q':
			QWordToHex(va_arg(args, UINT64), buffer_ptr);
			buffer_ptr += 16;
			break;

		case L'x':
		{
			if (size == 2)
			{
				ByteToHex(va_arg(args, UINT8), buffer_ptr);
				buffer_ptr += 2;
			}
			else if (size == 4)
			{
				WordToHex(va_arg(args, UINT16), buffer_ptr);
				buffer_ptr += 4;
			}
			else if (size == 8)
			{
				DWordToHex(va_arg(args, UINT32), buffer_ptr);
				buffer_ptr += 8;
			}
			else if (size == 16)
			{
				QWordToHex(va_arg(args, UINT64), buffer_ptr);
				buffer_ptr += 16;
			}
			else
			{
				Print(L"Error!\n");
			}
			break;
		}

		case L'd':
		{
			UINT32 length = IntToString(va_arg(args, int), buffer_ptr);
			buffer_ptr += length;
			break;
		}

		case L'S':
		{
			buffer_ptr += CRT::strcpy(buffer_ptr, va_arg(args, CHAR16*));
			break;
		}

		case L's':
		{
			char* a = va_arg(args, char*);
			while (*a != '\0')
			{
				CHAR16 n = *a;
				*buffer_ptr = n;
				buffer_ptr++;
				a++;
			}
			break;
		}

		case L'c':
			*buffer_ptr = va_arg(args, CHAR16);
			buffer_ptr++;
			break;
		}

		//Advance past type - assume 1 char types
		format++;

		//Update start
		start = format;
	}

	//Copy end of string
	CRT::strncpy(buffer_ptr, start, (format - start));

	return 0;
}

//TODO: rewrite without using length (subtract pointers)
UINT32 EfiPrint::IntToString(int data, CHAR16* str)
{
	UINT32 length = 0;
	if (data < 0)
	{
		*str = L'-';
		data *= -1;
		str++;
		length++;
	}

	CHAR16* start = str;
	while (data > 0)
	{
		int digit = data % 10;
		*str = (Char16_Zero + digit);
		str++;
		data /= 10;
		length++;
	}

	CRT::strrev(start);
	return length;
}

void EfiPrint::QWordToHex(UINT64 data, CHAR16* string)
{
	DWordToHex(QWordHighDWord(data), string);
	DWordToHex(QWordLowDWord(data), string + 8);
}

void EfiPrint::DWordToHex(UINT32 data, CHAR16* string)
{
	WordToHex(DWordHighWord(data), string);
	WordToHex(DWordLowWord(data), string + 4);
}

void EfiPrint::WordToHex(UINT16 data, CHAR16* string)
{
	ByteToHex(WordHighByte(data), string);
	ByteToHex(WordLowByte(data), string + 2);
}

void EfiPrint::ByteToHex(UINT8 data, CHAR16* string)
{
	string[0] = GetHexChar(ByteHighNibble(data));
	string[1] = GetHexChar(ByteLowNibble(data));
}