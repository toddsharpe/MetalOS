#pragma once

#include <cstdint>
#include <cctype>
#include "Lib/Buffer.h"

int constexpr length(const char* str)
{
	return *str ? 1 + length(str + 1) : 0;
}

struct CString
{
	static CString Clip(const char *const data, const size_t maxLength)
	{
		const size_t len = strnlen_s(data, maxLength);
		return CString(data, len);
	}

	constexpr CString() : Length(), m_data()
	{

	}
	
	//When constructing from char array, null terminator is included. Adjust length accordingly
	template <size_t N>
	constexpr CString(const char (&data)[N]) : m_data(data), Length(N - 1)
	{
		
	}

	//When length is known, length should not include null terminator
	constexpr CString(const char *const data, const size_t length) : m_data(data), Length(length)
	{
		
	}

	//Standard C strings that have no known length
	constexpr explicit CString(const char *const data) : m_data(data), Length(length(data))
	{
		
	}

	bool operator==(const CString& rhs) const
	{
		return this->Length == rhs.Length && memcmp(this->m_data, rhs.m_data, this->Length) == 0;
	}

	explicit operator bool() const
	{
		return Length != 0;
	}

	const char *begin() const
	{
		return m_data;
	}

	const char *end() const
	{
		return &m_data[Length];
	}

	const char* c_str() const
	{
		return m_data;
	}

	const char& operator[](const size_t index) const
	{
		Assert(index < Length);
		return begin()[index];
	}

	size_t Length;

private:
	const char* m_data;
};

//Buffer of fixed length who's contents can be modified
struct String
{
	constexpr String() : Data(), Length()
	{

	}
	
	template <size_t N>
	constexpr String(char (&data)[N]) : Data(data), Length(N)
	{
		
	}

	constexpr String(char *const data, const size_t length) : Data(data), Length(length)
	{
		
	}

	bool operator==(const String& rhs) const
	{
		return this->Length == rhs.Length && memcmp(this->Data, rhs.Data, this->Length) == 0;
	}

	bool operator!=(const String& rhs) const
	{
		return !(*this == rhs);
	}

	bool operator==(const CString& rhs) const
	{
		return this->Length == rhs.Length && memcmp(this->c_str(), rhs.c_str(), this->Length) == 0;
	}

	bool operator!=(const CString& rhs) const
	{
		return !(*this == rhs);
	}

	const char *begin() const
	{
		return reinterpret_cast<const char*>(Data);
	}

	const char *end() const
	{
		return reinterpret_cast<const char*>(&Data[Length]);
	}

	const char* c_str() const
	{
		return Data;
	}

	char *begin()
	{
		return reinterpret_cast<char*>(Data);
	}

	char *end()
	{
		return reinterpret_cast<char*>(&Data[Length]);
	}

	char& operator[](const size_t index)
	{
		Assert(index < Length);
		return begin()[index];
	}

	operator CString() const
	{
		return {Data, Length};
	}

	const CString Slice(const size_t length)
	{
		Assert(length <= Length);
		return {Data, length};
	}

	char* Data;
	size_t Length;
};

struct StringStorage
{
protected:
	template <size_t N>
	constexpr StringStorage(char (&data)[N]) :
		Data(data),
		m_count(0),
		Capacity(N)
	{
		
	}

public:
	const StringStorage& operator=(const CString& buffer) 
	{
		Assert(buffer.Length < Capacity);
		Trace();
		memcpy(Data, buffer.c_str(), buffer.Length);
		m_count = buffer.Length;

		return *this;
	}

	void ToLower()
	{
		for (size_t i = 0; i < m_count; i++)
		{
			Data[i] = tolower(Data[i]);
		}
	}

	void Pop()
	{
		m_count--;

		//Null terminate
		*end() = '\0';
	}

	char *begin()
	{
		return reinterpret_cast<char*>(Data);
	}

	char *end()
	{
		return reinterpret_cast<char*>(&Data[m_count]);
	}

	const char* c_str() const
	{
		return Data;
	}

	void Append(const char c)
	{
		//Assert there is room
		Assert(m_count < Capacity - 1);

		//Write char
		*end() = c;
		m_count++;

		//Null terminate
		*end() = '\0';
	}

	void Clear()
	{
		m_count = 0;
		*end() = '\0';
	}

	void Set(const CString& str)
	{
		Clear();
		Append(str);
	}

	void Append(const CString& str)
	{
		AssertOp(m_count + str.Length, <, Capacity - 1);

		memcpy(end(), str.c_str(), str.Length);
		m_count += str.Length;

		//Null terminate
		*end() = '\0';
	}

	void AppendClip(const char* const c_str, const size_t maxLength = 0)
	{
		const CString clip = CString::Clip(c_str, maxLength ? maxLength : Capacity);
		Assert(m_count + clip.Length < Capacity - 1);

		memcpy(end(), clip.c_str(), clip.Length);
		m_count += clip.Length;

		//Null terminate
		*end() = '\0';
	}

	bool operator==(const CString& rhs) const
	{
		return CString(*this) == rhs;
	}

	bool operator!=(const CString& rhs) const
	{
		return !(*this == rhs);
	}

	operator CString() const
	{
		return {Data, m_count};
	}

	const CString Ref() const
	{
		return { Data, m_count};
	}

	size_t Count() const
	{
		return m_count;
	}

	char* const Data;
	const size_t Capacity;

protected:
	size_t m_count;
};

template <size_t N>
struct StaticString : public StringStorage
{
public:
	constexpr StaticString() :
		StringStorage(m_storage),
		m_storage()
	{

	}

	constexpr StaticString(const CString& other) : StaticString()
	{
		//TODO(tsharpe): Is memcpy in the constructor a good idea?
		Assert(other.Length < Capacity);
		memcpy(m_storage, other.c_str(), other.Length);
		m_count = other.Length;
	}

	static constexpr size_t Size = N;

private:
	char m_storage[N];
};
