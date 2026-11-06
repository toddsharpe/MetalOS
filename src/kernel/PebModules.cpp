#include "kernel/PebModules.h"
#include "Lib/Math.h"
#include "WinPE.h"

bool GetPebModule(const ProcessEnvironmentBlock* const peb, const AddressSpace& space, const uintptr_t address, KModule& module)
{
	if (!space.IsValid(&peb->ModuleIndex))
		return false;

	const size_t count = Min(peb->ModuleIndex, MaxLoadedModules);
	for (size_t i = 0; i < count; i++)
	{
		//One copy, as the process can rewrite the entry meanwhile
		if (!space.IsValid(&peb->LoadedModules[i]))
			return false;
		const Module entry = peb->LoadedModules[i];

		//A freed module's entry, or a bogus one, doesn't start a reservation
		const uintptr_t base = (uintptr_t)entry.ImageBase;
		const size_t pages = space.GetCount(base);
		if (pages == 0)
			continue;

		const size_t imageSize = WinPE::GetMappedImageSize(entry.ImageBase, pages << Arch::PageShift);
		if (imageSize == 0 || address < base || address - base >= imageSize)
			continue;

		size_t length = 0;
		while (length < MaxModuleName && entry.Name[length] != '\0')
			length++;
		if (length == 0 || length == MaxModuleName)
			continue;

		StrCopy(module.Name, CStr(entry.Name, length));
		module.ImageBase = entry.ImageBase;
		module.ImageSize = imageSize;
		return true;
	}

	return false;
}
