#include "kernel/PebModules.h"
#include <string.h>
#include <malloc.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

//Looks modules up in a PEB the way a trace of a user process does, with this test's own image as the process's module
void PebModules_test()
{
	const uintptr_t imageBase = (uintptr_t)&__ImageBase;
	const size_t imageSize = WinPE::GetSizeOfImage(&__ImageBase);
	const size_t imagePages = (imageSize + Arch::PageMask) >> Arch::PageShift;
	//In a page of its own outside this image, so a space can hold the PEB or not
	ProcessEnvironmentBlock* const peb = static_cast<ProcessEnvironmentBlock*>(_aligned_malloc(Arch::PageSize, Arch::PageSize));
	Assert(peb);
	const uintptr_t pebPage = (uintptr_t)peb;

	AddressSpace space(0, 0x00007FFF'FFFF0000);
	Assert(space.Reserve(pebPage, 1));
	Assert(space.Reserve(imageBase, imagePages));

	//This image, listed after the entry of a module whose image is gone, so no reservation starts at it
	memset(peb, 0, sizeof(*peb));
	peb->LoadedModules[0].ImageBase = (void*)0x0000'7000'0000'0000;
	strcpy(peb->LoadedModules[0].Name, "gone.dll");
	peb->LoadedModules[1].ImageBase = &__ImageBase;
	strcpy(peb->LoadedModules[1].Name, "test.exe");
	peb->ModuleIndex = 2;

	KModule module;
	Assert(GetPebModule(peb, space, imageBase + 0x1000, module));
	Assert(module.Name == "test.exe");
	AssertEqual((uintptr_t)module.ImageBase, imageBase);
	AssertEqual(module.ImageSize, imageSize);
	Assert(GetPebModule(peb, space, imageBase + imageSize - 1, module));
	Assert(!GetPebModule(peb, space, imageBase + imageSize, module));
	Assert(!GetPebModule(peb, space, imageBase - 1, module));

	//A name that runs to the end of the entry, or an empty one
	memset(peb->LoadedModules[1].Name, 'x', MaxModuleName);
	Assert(!GetPebModule(peb, space, imageBase, module));
	peb->LoadedModules[1].Name[0] = '\0';
	Assert(!GetPebModule(peb, space, imageBase, module));
	strcpy(peb->LoadedModules[1].Name, "test.exe");

	//A base inside a reservation rather than at its start
	peb->LoadedModules[1].ImageBase = (void*)(imageBase + Arch::PageSize);
	Assert(!GetPebModule(peb, space, imageBase + Arch::PageSize, module));
	peb->LoadedModules[1].ImageBase = &__ImageBase;

	//A count past the PEB's entries reads only those
	peb->ModuleIndex = (size_t)-1;
	Assert(GetPebModule(peb, space, imageBase, module));
	peb->ModuleIndex = 2;

	//An image larger than the reservation it starts
	AddressSpace small(0, 0x00007FFF'FFFF0000);
	Assert(small.Reserve(pebPage, 1));
	Assert(small.Reserve(imageBase, imagePages - 1));
	Assert(!GetPebModule(peb, small, imageBase, module));

	//A PEB the space doesn't hold
	AddressSpace noPeb(0, 0x00007FFF'FFFF0000);
	Assert(noPeb.Reserve(imageBase, imagePages));
	Assert(!GetPebModule(peb, noPeb, imageBase, module));

	_aligned_free(peb);
}
