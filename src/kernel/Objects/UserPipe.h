#pragma once

#include "Kernel/Objects/UObject.h"

#include <memory>

enum class PipeOp
{
	Read,
	Write
};

//Ring buffer implementation using double-mapped physical pages in callers address space.
class UserPipe
{
public:
	static bool EventSignal(void* const arg);

	UserPipe();
	~UserPipe();

	void Init();

	bool Read(void* const buffer, const size_t length, size_t& read);
	bool Write(const void* buffer, const size_t length);

	size_t FreeSpace() const;
	size_t Count() const;
	bool IsBroken() const;

	//Number of readers and writers
	//TODO(tsharpe): Find a better way to do this
	size_t Readers;
	size_t Writers;

private:
	static constexpr size_t PageCount = 1;

	bool EventSignal(const PipeOp op, const size_t count);

	//Ring buffer indexes
	size_t m_readIndex;
	size_t m_writeIndex;

	//Bytes used and total size
	size_t m_count;
	size_t m_size;

	//Buffer
	void* m_buffer;
};

struct PipeEvent
{
	PipeEvent(UserPipe& pipe, const PipeOp op, const size_t count) :
		Pipe(pipe),
		Op(op),
		Count(count)
	{}

	UserPipe& Pipe;
	const PipeOp Op;
	const size_t Count;
};
