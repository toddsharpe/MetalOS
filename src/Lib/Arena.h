#pragma once

#include <cstdint>
#include <cstring>
#include "Lib/Math.h"
#include "Lib/System.h"
#include "Lib/Buffer.h"
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
		if (m_count + size > m_size)
			return nullptr;

		void* const address = (void*)&m_storage[m_count];
		const size_t alignedSize = ByteAlign(size, Alignment);
		m_count += alignedSize;

		memset(address, 0x00, alignedSize);
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

	void* Pack(const size_t size)
	{
		if (m_count + size > m_size)
			return nullptr;

		void* const address = (void*)&m_storage[m_count];
		m_count += size;

		memset(address, 0x00, size);
		return address;
	}

	template <typename T>
	T* Pack()
	{
		//Acquire memory
		void* const allocated = Pack(sizeof(T));
		if (!allocated)
			return nullptr;

		//Call constructor
		T* created = new (allocated) T();
		return created;
	}

	template <typename T, typename ...A>
	T* Pack(A ...args)
	{
		//Acquire memory
		void* const allocated = Pack(sizeof(T));
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

	String Copy(const CString& source)
	{
		//Allocate for null terminator
		void* allocated = Allocate(source.Length + 1);
		memcpy(allocated, source.c_str(), source.Length);
		return {reinterpret_cast<char*>(allocated), source.Length};
	}

	void Reset()
	{
		m_count = 0;
	}

	void Display() const
	{
		Printf("Static Arena: Size: 0x%x, Count: 0x%x\n", m_size, m_count);
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
