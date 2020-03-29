#include "RuntimeSupport.h"

#include "Main.h"

PIMAGE_SECTION_HEADER RuntimeSupport::GetPESection(const std::string& name, uint64_t* ImageBase)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)((uint64_t)ImageBase + dosHeader->e_lfanew);

	//Find section
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if ((char*)& section[i].Name == name)
			return &section[i];
	}

	Assert(false);
	return nullptr;
}

//TODO: History Table?
PRUNTIME_FUNCTION RuntimeSupport::LookupFunctionEntry(uint64_t ControlPC, uint64_t* ImageBase, void* HistoryTable)
{
	const uint32_t pageNumber = (ControlPC - (uint64_t)ImageBase) >> PAGE_SHIFT;
	
	PIMAGE_SECTION_HEADER pdataHeader = RuntimeSupport::GetPESection(".pdata", ImageBase);
	Print("0x%016x\n", pdataHeader);
	Assert(pdataHeader);

	PRUNTIME_FUNCTION current = (PRUNTIME_FUNCTION)((uint64_t)ImageBase + pdataHeader->VirtualAddress);
	while (current != nullptr && !(current->BeginAddress <= pageNumber && current->EndAddress >= pageNumber))
		current++;

	return current;
}
