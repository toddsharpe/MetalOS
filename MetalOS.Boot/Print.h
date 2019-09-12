#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS Print(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, CHAR16* format, ...);

EFI_STATUS PrintUint32Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT32 data);
EFI_STATUS PrintUint16Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT16 data);
EFI_STATUS PrintUint8Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT8 data);

void IntToString(int data, CHAR16* str);
void DWordToString(UINT32 data, CHAR16* string);
void WordToString(UINT16 data, CHAR16* string);
void ByteToString(UINT8 data, CHAR16* string);