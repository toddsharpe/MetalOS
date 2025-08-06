#pragma once

#include "Arch.h"
#include "Lib/List.h"
#include "Lib/Arena.h"
#include "kernel/Types.h"

class ModuleList
{
public:
	ModuleList() : m_modules()
	{

	}

	void Initialize()
	{
		ListInitializeHead(m_modules);
	}

	KModule* AddModule(Arena& arena, const CString& name, void* image)
	{
		KModule* module = arena.Allocate<KModule>();
		module->Name = arena.Copy(name);
		module->ImageBase = image;
		ListInsertTail(m_modules, module->Link);
		return module;
	}

	KModule* GetModule(const CString& name) const
	{
		KModule* found = ListFirst<KModule, const CString&>(m_modules, [](const ListEntry& entry, const KModule& module, const CString& name)
		{
			return module.Name == name;
		}, name);
		Assert(found);

		return found;
	}

	KModule* GetModule(const uintptr_t address) const
	{
		KModule* found = ListFirst<KModule, const uintptr_t>(m_modules, [](const ListEntry& entry, const KModule& module, const uintptr_t& address)
		{
			const uintptr_t imageBase = (uintptr_t)module.ImageBase;
			const size_t imageSize = WinPE::GetSizeOfImage(module.ImageBase);
			const bool contains = ((address >= imageBase) && (address < imageBase + imageSize));
			return contains;
		}, address);
		Assert(found);

		return found;
	}

private:
	ListHead m_modules;
};
