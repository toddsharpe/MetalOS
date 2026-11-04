#include "kernel/Api.h"
#include "Lib/StaticHeap.h"
#include "Lib/Arena.h"
#include "Lib/System.h"
#include "kernel/Arch.h"

namespace _HEAP
{
	/*
	 * Kernel Arenas.
	 */
	static constexpr size_t KernelArenaSize = Arch::PageSize << 9; //2MB Kernel Arena
	static constexpr size_t AcpiArenaSize = Arch::PageSize << 8; //1MB ACPI Arena

	KERNEL_PAGE_ALIGN static StaticHeap<KernelArenaSize> m_kernelAlloc;
	KERNEL_PAGE_ALIGN static StaticArena<AcpiArenaSize> m_acpiArena;
}

void* KeAlloc(const size_t size, const AllocType type)
{
	switch (type)
	{
		case AllocType::User:
		case AllocType::Kernel:
			return _HEAP::m_kernelAlloc.Allocate(size);

		case AllocType::Acpi:
			return _HEAP::m_acpiArena.Allocate(size);

		case AllocType::Malloc:
			return _HEAP::m_kernelAlloc.Allocate(size);

		default:
			NotImplemented();
			return nullptr;
	}
}

void KeFree(void* ptr, const AllocType type)
{
	switch (type)
	{
		case AllocType::User:
		case AllocType::Kernel:
			return _HEAP::m_kernelAlloc.Deallocate(ptr);

		case AllocType::Acpi:
			return _HEAP::m_acpiArena.Deallocate(ptr);

		case AllocType::Malloc:
			return _HEAP::m_kernelAlloc.Deallocate(ptr);

		default:
			NotImplemented();
	}
}
