#pragma once

#include "core_crt/stdint.h"
#include "core_crt/string.h"
#include "Assert.h"

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PageSize))

typedef uintptr_t paddr_t;

template<typename T>
constexpr T MakePointer(const void* base, const ptrdiff_t offset = 0)
{
	return reinterpret_cast<T>((char*)base + offset);
}

constexpr size_t ByteAlign(const size_t size, const size_t alignment)
{
	const size_t mask = alignment - 1;
	return ((size + mask) & ~(mask));
}

template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N]) {
	return N;
}
