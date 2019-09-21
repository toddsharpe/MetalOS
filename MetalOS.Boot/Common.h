#pragma once

#define ReturnIfNotSuccess(x) status = x; if (EFI_ERROR(status)) { DisplayError(L#x, status); return status; }

#define ByteHighNibble(x) (((UINT8)x) >> 4)
#define ByteLowNibble(x) (((UINT8)x) & 0x0F)

#define WordHighByte(x) (((UINT16)x) >> 8)
#define WordLowByte(x) (((UINT16)x) & 0xFF)

#define DWordHighWord(x) (((UINT32)x) >> 16)
#define DWordLowWord(x) (((UINT32)x) & 0xFFFF)

#define QWordHighDWord(x) (((UINT64)x) >> 32)
#define QWordLowDWord(x) (((UINT64)x) & 0xFFFFFFFF)

#define LibraryFunction(x) lib_ ## x

#define BOOL UINT8
#define TRUE 1
#define FALSE 0

extern EFI_SYSTEM_TABLE* ST;
extern EFI_RUNTIME_SERVICES* RT;
extern EFI_BOOT_SERVICES* BS;
extern EFI_MEMORY_TYPE AllocationType;
