#pragma once

#include "kernel/Arch.h"
#include "Lib/String.h"

#include "kernel/Types.h"
#include "kernel/Core/AddressSpace.h"
#include "MetalOS.Space.h"
#include "kernel/Vmem.h"

//Manages resources associated with a process
//-Virtual allocations

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
		//Kernel dynamic allocations start at KernelAllocStart so the watermark never
		//collides with the fixed low-memory regions (KernelBase, KdCom, KernelPdb, KernelHeap).
		Space(get_start(type), get_end(type), get_offset(type)),
		Tables(),
		IsGlobal(type == KProcessType::Kernel)
	{
	}

	void Display() const 
	{
		Printf("KProcess: %d\n", IsGlobal);
		Space.Display();
	}

	//Address Space
	AddressSpace Space;

	//Page Tables
	KernelPageTables Tables;

	bool IsGlobal;

private:
};