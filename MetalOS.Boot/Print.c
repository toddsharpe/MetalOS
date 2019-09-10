#include "Print.h"
#include "Common.h"

CHAR16 Char16_Zero = L'0';
CHAR16 Char16_CapA = L'A';

#define GetHexChar(x) ((x) < 10 ? (Char16_Zero + (x)) : (Char16_CapA + ((x) - 10)))

EFI_STATUS PrintEfiTime(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, EFI_TIME* timeNow)
{

}

EFI_STATUS PrintUint32Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT32 data)
{
	EFI_STATUS status;

	CHAR16 buffer[8];

	buffer[0] = GetHexChar(ByteHighNibble(WordHighByte(DWordHighWord(data))));
	buffer[1] = GetHexChar(ByteLowNibble(WordHighByte(DWordHighWord(data))));
	buffer[2] = GetHexChar(ByteHighNibble(WordLowByte(DWordHighWord(data))));
	buffer[3] = GetHexChar(ByteLowNibble(WordLowByte(DWordHighWord(data))));
	buffer[4] = GetHexChar(ByteHighNibble(WordHighByte(DWordLowWord(data))));
	buffer[5] = GetHexChar(ByteLowNibble(WordHighByte(DWordLowWord(data))));
	buffer[6] = GetHexChar(ByteHighNibble(WordLowByte(DWordLowWord(data))));
	buffer[7] = GetHexChar(ByteLowNibble(WordLowByte(DWordLowWord(data))));

	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer))
}

EFI_STATUS PrintUint16Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT16 data)
{
	EFI_STATUS status;

	CHAR16 buffer[4];

	UINT8 d = WordHighByte(data);
	UINT8 d1 = WordLowByte(data);

	buffer[0] = GetHexChar(ByteHighNibble(WordHighByte(data)));
	buffer[1] = GetHexChar(ByteLowNibble(WordHighByte(data)));
	buffer[2] = GetHexChar(ByteHighNibble(WordLowByte(data)));
	buffer[3] = GetHexChar(ByteLowNibble(WordLowByte(data)));

	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer))
}

EFI_STATUS PrintUint8Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT8 data)
{
	EFI_STATUS status;

	CHAR16 buffer[2];

	buffer[0] = GetHexChar(ByteHighNibble(data));
	buffer[1] = GetHexChar(ByteLowNibble(data));
	
	ReturnIfNotSuccess(conOut->OutputString(conOut, buffer))
}