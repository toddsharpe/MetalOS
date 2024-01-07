#include <cstddef>
#include <efi.h>

extern EFI_BOOT_SERVICES* BS;

void* operator new(size_t n)
{
	void* addr = nullptr;
	BS->AllocatePool(EFI_MEMORY_TYPE::EfiLoaderData, n, &addr);
	return addr;
}

void operator delete(void* p, size_t count)
{
	BS->FreePool(p);
}
