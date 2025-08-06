#include "kernel/KProcess.h"
#include "WinPE.h"
#include "kernel/Api.h"

constexpr KProcess::KProcess(const KProcessType type) :
	IsGlobal(type == KProcessType::Kernel),
	Debug(),
	m_modules(),
	m_start(type == KProcessType::Kernel ? KernelStart : UserStart),
	m_end(type == KProcessType::Kernel ? KernelEnd : UserEnd),
	m_watermark(type == KProcessType::Kernel ? KernelAllocStart : UserAllocStart),
	m_reservations(),
	m_arena()
{
}

void KProcess::Initialize()
{
	//Initialize lists
	ListInitializeHead(m_modules);
	ListInitializeHead(m_reservations);
}

KModule* KProcess::AddModule(const CString& name, void* image)
{
	KModule* module = m_arena.Allocate<KModule>();
	module->Name = m_arena.Copy(name);
	module->ImageBase = image;
	ListInsertTail(m_modules, module->Link);
	return module;
}

KModule* KProcess::GetModule(const CString& name) const
{
	KModule* found = ListFirst<KModule, const CString&>(m_modules, [](const ListEntry& entry, const KModule& module, const CString& name)
	{
		return module.Name == name;
	}, name);

	return found;
}

KModule* KProcess::GetModule(const uintptr_t address) const
{
	KModule* found = ListFirst<KModule, const uintptr_t>(m_modules, [](const ListEntry& entry, const KModule& module, const uintptr_t& address)
	{
		const uintptr_t imageBase = (uintptr_t)module.ImageBase;
		const size_t imageSize = WinPE::GetSizeOfImage(module.ImageBase);
		const bool contains = ((address >= imageBase) && (address < imageBase + imageSize));
		return contains;
	}, address);

	return found;
}

bool KProcess::IsValidPointer(const void* const address)
{
	//Check if pointer is even in address space
	if (((uintptr_t)address < m_start) || ((uintptr_t)address >= m_end))
		return false;
	
	const uintptr_t page = (uintptr_t)address & ~PageMask;
	return !IsFree(page, 1);
}

bool KProcess::Reserve(const uintptr_t address, const size_t count)
{
	Assert(address);
	Assert(count);
	CPrintf(Debug, "Reserve: 0x%016x in [0x%016x, 0x%016x] Size: 0x%x\n", address, m_start, m_end, count);

	Assert((address & PageMask) == 0);
	if (!IsFree(address, count))
		return false;

	Reservation* res = m_arena.Allocate<Reservation>();
	Assert(res);
	res->Address = address;
	res->PageCount = count;
	ListInsertTail(m_reservations, res->Link);

	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return true;
}

uintptr_t KProcess::Reserve(const size_t count)
{
	Assert(count != 0);
	CPrintf(Debug, "Reserve: Any in [0x%016x, 0x%016x] Size: 0x%x\n", m_start, m_end, count);
	CPrintf(Debug, "  Watermark: 0x%016x Gran: 0x%016x\n", m_watermark, AllocationGranularity);

	while (!IsFree(m_watermark, count) && m_watermark < m_end)
	{
		AssertOp(m_watermark, <, m_end);
		m_watermark += AllocationGranularity;
	}

	//No suitable address found
	if (m_watermark >= m_end)
		return false;
	
	const uintptr_t address = m_watermark;
	m_watermark += ByteAlign((count << PageShift), AllocationGranularity);;
	CPrintf(Debug, "  NewWatermark: 0x%016x\n", m_watermark);

	Reservation* res = m_arena.Allocate<Reservation>();
	Assert(res);
	res->Address = address;
	res->PageCount = count;
	ListInsertTail(m_reservations, res->Link);

	CPrintf(Debug, "  Received: 0x%016x Count:0x%x\n", address, count);

	return address;
}

void KProcess::Display() const
{
	Printf("KProcess: %d\n", IsGlobal);
	Printf("  Modules:\n");
	ListForEach<KModule>(m_modules, [](const ListEntry& entry, const KModule& item)
	{
		Printf("    0x%016X: %s\n", item.ImageBase, item.Name.c_str());
	});
	Printf("  Reservations: %d\n", m_reservations.Count);
	ListForEach<Reservation>(m_reservations, [](const ListEntry& entry, const Reservation& item)
	{
		const uintptr_t high = item.Address + (item.PageCount << PageShift);
		Printf("    [0x%016X-0x%016X] (Count: 0x%x)\n", item.Address, high, item.PageCount);
	});
}

bool KProcess::IsFree(const uintptr_t address, const size_t count) const
{
	Assert((address & PageMask) == 0);

	//Check if address is inside out address space
	if ((address < m_start) || (address >= m_end))
		return false;

	struct Lookup
	{
		uintptr_t Low;
		uintptr_t High;
	};

	Lookup lookup = {};
	lookup.Low = address;
	lookup.High = address + (count << PageShift);

	const bool found = ListAny<Reservation, Lookup>(m_reservations, [](const ListEntry&, const Reservation& res, Lookup& lookup)
	{
		const uintptr_t end = res.Address + (res.PageCount << PageShift);

		//Check start address
		if ((lookup.Low >= res.Address) && (lookup.Low < end))
			return true;

		//Check ending address
		if ((lookup.High >= res.Address) && (lookup.High < end))
			return true;

		return false;
	}, lookup);

	return !found;
}
