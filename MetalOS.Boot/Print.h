#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS Print(CHAR16* format, ...);
EFI_STATUS _print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, CHAR16* format, va_list args);

EFI_STATUS PrintUint32Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT32 data);
EFI_STATUS PrintUint16Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT16 data);
EFI_STATUS PrintUint8Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT8 data);

UINT32 IntToString(int data, CHAR16* str);
void DWordToHex(UINT32 data, CHAR16* string);
void WordToHex(UINT16 data, CHAR16* string);
void ByteToHex(UINT8 data, CHAR16* string);