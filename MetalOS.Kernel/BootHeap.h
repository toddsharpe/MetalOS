#pragma once
#include <cstdint>

//Cheap water-mark allocator
class BootHeap
{
public:
	BootHeap(const void* address, const size_t length);

	void* Allocate(size_t size);
	void Deallocate(void* address);
	size_t m_allocated;

private:
	uintptr_t m_address;
	size_t m_length;

	uintptr_t m_watermark;
};

