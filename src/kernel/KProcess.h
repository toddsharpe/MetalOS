#pragma once

#include "Arch.h"
#include "Lib/String.h"
#include "Lib/LinkedList.h"
#include "x64/PageTables.h"
#include "kernel/Types.h"
#include "kernel/AddressSpace.h"
#include "MetalOS.Space.h"

//Manages resources associated with a process
//-Virtual allocations
//-Modules

class KModuleList : public LinkedList<KModule*>
{
public:
	constexpr KModuleList() :
		LinkedList<KModule*>()
	{

	}

	const KModule* AddModule(const CString& name, void* image)
	{
		KModule* created = KeAlloc<KModule>(AllocType::User, name, image);
		Assert(this->Add(created));
		return created;
	}

	const KModule* GetModule(const CString& name) const
	{
		for (const KModule* module : *this)
		{
			if (module->Name == name)
				return module;
		}

		return nullptr;
	}

	//NOTE: needs modules to be addressable at time of call
	const KModule* GetModule(const uintptr_t address) const
	{
		for (const KModule* module : *this)
		{
			const uintptr_t imageBase = (uintptr_t)module->ImageBase;
			const size_t imageSize = WinPE::GetSizeOfImage(module->ImageBase);
			const bool contains = ((address >= imageBase) && (address < imageBase + imageSize));

			if (contains)
				return module;
		}

		return nullptr;
	}

	void Display() const
	{
		Printf("  Modules:\n");
		for (const KModule* module : *this)
		{
			Printf("    0x%016X: %s\n", module->ImageBase, module->Name.c_str());
		}
	}

private:
};

enum class KProcessType
{
	Kernel,
	User
};

constexpr uintptr_t get_start(const KProcessType type)
{
	if (type == KProcessType::Kernel)
		return KernelStart;
	else
		return UserStart;
}

constexpr uintptr_t get_end(const KProcessType type)
{
	if (type == KProcessType::Kernel)
		return KernelEnd;
	else
		return UserEnd;
}

constexpr uintptr_t get_offset(const KProcessType type)
{
	constexpr uint64_t AllocationGranularity = 0x1'0000; //64K

	if (type == KProcessType::Kernel)
		return KernelAllocStart - KernelStart;
	else
		return AllocationGranularity;
}

struct KProcess
{
public:
	constexpr KProcess(const KProcessType type) :
		Modules(),
		//Kernel dynamic allocations start at KernelAllocStart so the watermark never
		//collides with the fixed low-memory regions (KernelBase, KdCom, KernelPdb, KernelHeap).
		Space(get_start(type), get_end(type), get_offset(type)),
		Tables(),
		IsGlobal(type == KProcessType::Kernel),
		Debug()
	{
	}

	void Display() const 
	{
		Printf("KProcess: %d\n", IsGlobal);
		Modules.Display();
		Space.Display();
	}

	//Modules 
	KModuleList Modules;

	//Address Space
	AddressSpace Space;

	//Page Tables
	KernelPageTables Tables;

	bool IsGlobal;
	bool Debug;

private:
};