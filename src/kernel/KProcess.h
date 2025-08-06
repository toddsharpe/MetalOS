#pragma once

#include "Arch.h"
#include "Lib/String.h"
#include "Lib/Arena.h"
#include "x64/PageTables.h"
#include "kernel/Types.h"
#include "MetalOS.Space.h"

enum class KProcessType
{
	Kernel,
	User
};

//Manages resources associated with a process
//-Virtual allocations
//-Modules

class KProcess
{
public:
	constexpr KProcess(const KProcessType type);

	void Initialize();

	KModule* AddModule(const CString& name, void* image);
	KModule* GetModule(const CString& name) const;
	KModule* GetModule(const uintptr_t address) const;

	bool IsValidPointer(const void* const address);
	bool Reserve(const uintptr_t address, const size_t count);
	uintptr_t Reserve(const size_t count);

	void Display() const;

	bool IsGlobal;
	bool Debug;

private:
	static constexpr uint64_t AllocationGranularity = 0x1'0000; //64K
	static constexpr uintptr_t UserAllocStart = UserStart + AllocationGranularity;
	static constexpr uintptr_t KernelAllocStart = 0xFFFF'8000'2000'0000;

	struct Reservation
	{
		ListEntry Link;
		uintptr_t Address;
		size_t PageCount;
	};

	bool IsFree(const uintptr_t address, const size_t count) const;

	//Modules 
	ListHead m_modules;

	//Address Space
	uintptr_t m_start;
	uintptr_t m_end;
	uintptr_t m_watermark;
	ListHead m_reservations;

protected:
	StaticArena<PageSize * 2> m_arena;
};