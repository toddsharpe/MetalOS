#pragma once
#include <cstdint>

//Cheap water-mark allocator
class BootHeap
{
public:
	BootHeap(const void* address, const size_t length);

	void* Allocate(const size_t size);
	void Deallocate(void* const address);

private:
	static const size_t Alignment = 16;

	uintptr_t m_address;
	size_t m_length;
	uintptr_t m_watermark;
};

