#pragma once

#include <cstdint>
#include <cstring>
#include "Lib/Math.h"
#include "Lib/System.h"
#include "Lib/String.h"
#include "Assert.h"
#include "new.h"

class Arena
{
public:
	constexpr Arena(uint8_t* const storage, const size_t size) :
		m_storage(storage),
		m_size(size),
		m_count()
	{

	}

	void* Allocate(const size_t size)
	{
		void* const address = Reserve(size);
		if (!address)
			return nullptr;

		memset(address, 0x00, ByteAlign(size, Alignment));
		return address;
	}

	//Bump-allocate without writing (no memset). For memory the caller fills through some other
	//path (e.g. a different address space), so the arena must not touch it here.
	void* Reserve(const size_t size)
	{
		//The aligned size is what gets consumed; it wraps to a smaller value for sizes near SIZE_MAX
		const size_t alignedSize = ByteAlign(size, Alignment);
		if (alignedSize < size || alignedSize > m_size - m_count)
			return nullptr;

		void* const address = (void*)&m_storage[m_count];
		m_count += alignedSize;
		return address;
	}

	template <typename T>
	T* Allocate()
	{
		//Acquire memory
		void* const allocated = Allocate(sizeof(T));
		if (!allocated)
			return nullptr;

		//Call constructor
		T* created = new (allocated) T();
		return created;
	}

	template <typename T, typename ...A>
	T* Allocate(A&& ...args)
	{
		//Acquire memory
		void* const allocated = Allocate(sizeof(T));
		if (!allocated)
			return nullptr;

		//Call constructor
		T* created = new (allocated) T(args...);
		return created;
	}

	void Deallocate(const void* const pointer)
	{
		//TODO(tsharpe): Implement?
	}

	CString Copy(const CString& source)
	{
		//Allocate for null terminator; a full arena gives back an empty string rather than a copy to null
		void* allocated = Allocate(source.Length + 1);
		if (!allocated)
			return {};

		memcpy(allocated, source.c_str(), source.Length);
		return {reinterpret_cast<const char*>(allocated), source.Length};
	}

	void Reset()
	{
		m_count = 0;
	}

private:
	static constexpr size_t Alignment = 16;

	uint8_t *m_storage;
	size_t m_size;
	size_t m_count;
};

template <size_t N>
class StaticArena : public Arena
{
public:
	constexpr StaticArena() :
		Arena(m_backing, N)
	{

	}

private:
	uint8_t m_backing[N];
};

class PreallocatedArena : public Arena
{
public:
	constexpr PreallocatedArena() : Arena(nullptr, 0)
	{

	}

	constexpr PreallocatedArena(uint8_t* const storage, const size_t size) :
		Arena(storage, size)
	{

	}

	template <size_t N>
	constexpr PreallocatedArena(uint8_t (&storage)[N]) :
		Arena(storage, N)
	{

	}
};
