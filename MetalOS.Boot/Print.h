#pragma once

#include <efi.h>
#include <efilib.h>

EFI_STATUS PrintEfiTime(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, EFI_TIME* timeNow);

EFI_STATUS PrintUint32Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT32 data);
EFI_STATUS PrintUint16Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT16 data);
EFI_STATUS PrintUint8Hex(SIMPLE_TEXT_OUTPUT_INTERFACE* conOut, UINT8 data);
