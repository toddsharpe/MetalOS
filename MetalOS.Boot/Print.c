#include "Print.h"
#include "Common.h"
#include "String.h"
#include "Memory.h"

CHAR16 Char16_Zero = L'0';
CHAR16 Char16_CapA = L'A';

#define GetHexChar(x) ((x) < 10 ? (Char16_Zero + (x)) : (Char16_CapA + ((x) - 10)))
#define MAXBUFFER 255

EFI_STATUS Print(CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);
	EFI_STATUS status =  _print(ST->ConOut, format, args);
	va_end(args);
}

UINTN SPrint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	EFI_STATUS status = _sprint(Str, StrSize, fmt, args);
	va_end(args);
}

//TODO: Calculate string length?
EFI_STATUS _print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, CHAR16* format, va_list args)
{
	EFI_STATUS status;

	CHAR16 buffer[MAXBUFFER] = { 0 };
	_sprint(buffer, MAXBUFFER, format, args);

	//Write
	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer));

	return EFI_SUCCESS;
}

//TODO: we never check strsize
UINTN _sprint(OUT CHAR16* Str, IN UINTN StrSize, IN CONST CHAR16* format, va_list args)
{
	CHAR16* buffer_ptr = Str;

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

		BOOL zeroes = FALSE;
		if (*format == L'0')
		{
			zeroes = TRUE;
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
			buffer_ptr += efi_strcpy(va_arg(args, CHAR16*), buffer_ptr);
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
	efi_memcpy(start, buffer_ptr, (format - start) * sizeof(CHAR16));
}

EFI_STATUS PrintUint64Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT64 data)
{
	EFI_STATUS status;

	CHAR16 buffer[16];
	QWordToHex(data, buffer);

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

//TODO: rewrite without using length (subtract pointers)
UINT32 IntToString(int data, CHAR16* str)
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

	efi_strrev(start);
	return length;
}

void QWordToHex(UINT64 data, CHAR16* string)
{
	ASSERT(string != NULL);

	DWordToHex(QWordHighDWord(data), string);
	DWordToHex(QWordLowDWord(data), string + 8);
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