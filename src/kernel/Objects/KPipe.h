#pragma once

#include "core_crt/stdint.h"
#include "core_crt/stddef.h"
#include "kernel/Objects/KRef.h"

enum class KPipeOp
{
	Read,
	Write
};

//Full when indexes are equal
class KPipe
{
public:
	static bool EventSignal(void* const arg);

	constexpr KPipe(const size_t size);

	void Init();

	bool Read(void* const buffer, const size_t length, size_t& bytesRead);
	bool Write(const void* buffer, const size_t length, size_t& bytesWritten);

	//Convenience for kernel
	void Print(const char* format, ...);

	size_t Count() const;
	size_t Space() const;
	bool IsBroken() const;

	void Display() const;

	KRef Readers;
	KRef Writers;

private:
	bool EventSignal(const KPipeOp op, const size_t count);
	bool IsEmpty() const;
	bool IsFull() const;

	uint8_t* m_buffer;
	size_t m_count;
	size_t m_size;
	size_t m_readIndex;
	size_t m_writeIndex;
};

struct KPipeSignal
{
	KPipeSignal(KPipe& pipe, const KPipeOp op, const size_t count) :
		Pipe(pipe),
		Op(op),
		Count(count)
	{

	}

	KPipe& Pipe;
	const KPipeOp Op;
	const size_t Count;
};
