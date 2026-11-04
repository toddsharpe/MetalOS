#include "kernel/Api.h"
#include "Lib/StaticHeap.h"
#include "Lib/System.h"
#include "kernel/Arch.h"

namespace _HEAP
{
	/*
	 * Kernel Arenas.
	 */
	static constexpr size_t KernelArenaSize = Arch::PageSize << 9; //2MB Kernel Arena

	KERNEL_PAGE_ALIGN static StaticHeap<KernelArenaSize> m_kernelAlloc;
}

void* KeAlloc(const size_t size, const AllocType type)
{
	switch (type)
	{
		case AllocType::User:
		case AllocType::Kernel:
			return _HEAP::m_kernelAlloc.Allocate(size);

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

		case AllocType::Malloc:
			return _HEAP::m_kernelAlloc.Deallocate(ptr);

		default:
			NotImplemented();
	}
}
