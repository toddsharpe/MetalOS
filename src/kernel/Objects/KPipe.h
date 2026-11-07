#pragma once

#include "core_crt/stdint.h"
#include "core_crt/stddef.h"
#include "kernel/Objects/KRef.h"
#include "Lib/IntrusiveList.h"
#include "core_crt/stdarg.h"

enum class KPipeOp
{
	Read,
	Write
};

//Ring over a double-mapped buffer; m_count tells full from empty, so all m_size bytes are usable
//A WriteFile waiting its turn at a pipe; the writing thread embeds one
struct KPipeWriter : ListLink
{
};

class KPipe
{
public:
	static bool EventSignal(void* const arg);

	constexpr KPipe(const size_t size);

	bool Init(); //False when out of memory, with nothing left allocated
	void Free(); //Undoes Init: unmaps both views and frees the pages behind them

	bool Read(void* const buffer, const size_t length, size_t& bytesRead);
	bool Write(const void* buffer, const size_t length, size_t& bytesWritten);

	//Convenience for kernel
	void Print(const char* format, ...);
	void VPrint(const char* format, va_list args); //Not a Print overload: va_list is char*, so it would capture Print("%s", buffer)

	size_t Count() const;
	size_t Space() const;
	size_t Capacity() const;
	bool IsBroken() const;
	bool IsUnused() const;

	//Writes are queued whole in arrival order, as on Windows: only the writer at the front may write
	void QueueWriter(KPipeWriter& writer);
	void DequeueWriter(KPipeWriter& writer);

	KRef Readers;
	KRef Writers;
	KRef Waiters; //Threads in a syscall that can block on it, which keep it alive after its last handle closes

private:
	static constexpr size_t MaxPages = 16; //Buffer size limit; Init lists each page twice for the double map

	bool EventSignal(const KPipeOp op, const size_t count, const KPipeWriter* const writer);
	bool IsEmpty() const;

	uint8_t* m_buffer;
	size_t m_count;
	size_t m_size;
	size_t m_readIndex;
	size_t m_writeIndex;
	IntrusiveList<KPipeWriter> m_writers;
};

constexpr KPipe::KPipe(const size_t size) :
	Readers(),
	Writers(),
	Waiters(),
	m_buffer(),
	m_count(),
	m_size(size),
	m_readIndex(),
	m_writeIndex(),
	m_writers()
{

}

struct KPipeSignal
{
	KPipeSignal(KPipe& pipe, const KPipeOp op, const size_t count, const KPipeWriter* const writer = nullptr) :
		Pipe(pipe),
		Op(op),
		Count(count),
		Writer(writer)
	{

	}

	KPipe& Pipe;
	const KPipeOp Op;
	const size_t Count;
	const KPipeWriter* const Writer; //A write waits for its turn as well as for room
};
