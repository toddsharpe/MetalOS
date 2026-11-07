#pragma once

#include "Lib/System.h"
#include "core_crt/stdlib.h"
#include "new.h"
#include "Assert.h"

enum class AllocType
{
	Kernel,
	Acpi,
	Malloc,
	User
};

void* KeAlloc(size_t size, AllocType type);
void KeFree(void* ptr, AllocType type);

//core_crt's malloc and free, on the kernel heap
inline void* malloc(size_t size) { return KeAlloc(size, AllocType::Malloc); }
inline void free(void* ptr) { KeFree(ptr, AllocType::Malloc); }

template <typename T, typename... Args>
T* KeAlloc(AllocType type, Args&&... args)
{
	void* const mem = KeAlloc(sizeof(T), type);
	Assert(mem);
	return mem ? new (mem) T(static_cast<Args&&>(args)...) : nullptr;
}
