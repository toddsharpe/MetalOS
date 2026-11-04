#pragma once

#include "core_crt/stdint.h"
#include "core_crt/stddef.h"
#include "kernel/Objects/KRef.h"
#include "Lib/List.h"

enum class KPipeOp
{
	Read,
	Write
};

//Ring over a double-mapped buffer; m_count tells full from empty, so all m_size bytes are usable
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
	void VPrint(const char* format, va_list args); //Not a Print overload: va_list is char*, so it would capture Print("%s", buffer)

	size_t Count() const;
	size_t Space() const;
	size_t Capacity() const;
	bool IsBroken() const;

	//Writes are queued whole in arrival order, as on Windows: only the writer at the front may write
	void QueueWriter(ListEntry& writer);
	void DequeueWriter(ListEntry& writer);

	KRef Readers;
	KRef Writers;

private:
	bool EventSignal(const KPipeOp op, const size_t count, const ListEntry* const writer);
	bool IsEmpty() const;

	uint8_t* m_buffer;
	size_t m_count;
	size_t m_size;
	size_t m_readIndex;
	size_t m_writeIndex;
	ListHead m_writers;
};

struct KPipeSignal
{
	KPipeSignal(KPipe& pipe, const KPipeOp op, const size_t count, const ListEntry* const writer = nullptr) :
		Pipe(pipe),
		Op(op),
		Count(count),
		Writer(writer)
	{

	}

	KPipe& Pipe;
	const KPipeOp Op;
	const size_t Count;
	const ListEntry* const Writer; //A write waits for its turn as well as for room
};
