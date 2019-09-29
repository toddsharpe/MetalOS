#include "Loader.h"
#include "Common.h"
#include "Main.h"
#include "WindowsPE.h"

#define MakePtr( cast, ptr, addValue ) (cast)( (UINT64)(ptr) + (UINT64)(addValue))

UINT32 Loader::GetImageSize(void* baseAddress)
{
	Assert(baseAddress != nullptr);
	
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, baseAddress, dosHeader->e_lfanew);

	return ntHeader->OptionalHeader.SizeOfImage;
}
