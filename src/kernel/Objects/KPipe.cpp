#include "kernel/Objects/KPipe.h"
#include "kernel/Arch.h"
#include "kernel/Pmem.h"
#include "kernel/Vmem.h"
#include "kernel/Core/KProcess.h"
#include "Lib/StaticVector.h"
#include "kernel/Kernel.h"
#include "core_crt/stdarg.h"
#include "core_crt/stdio.h"

bool KPipe::EventSignal(void* const arg)
{
	KPipeSignal* event = reinterpret_cast<KPipeSignal*>(arg);
	return event->Pipe.EventSignal(event->Op, event->Count, event->Writer);
}


bool KPipe::Init()
{
	Assert(m_size % Arch::PageSize == 0);
	const size_t count = Arch::SizeToPages(m_size);
	if (count > MaxPages)
		return false;

	//Acquire physical pages; a process creates pipes, so running out fails the call instead of the kernel
	StaticVector<paddr_t, MaxPages * 2> addresses;
	for (size_t i = 0; i < count; i++)
	{
		paddr_t addr = 0;
		if (!KePhysicalTryAlloc(addr))
		{
			for (const paddr_t page : addresses)
				KePhysicalFree(page);
			return false;
		}
		Assert(addresses.Add(addr));
	}

	//Add them to list twice (double map)
	for (size_t i = 0; i < count; i++)
	{
		const paddr_t addr = addresses[i];
		Assert(addresses.Add(addr));
	}

	//Map into kernel; the mapping only borrows the pages, so a failed map leaves them to free here
	void* const base = KeVirtualMap(addresses.begin(), addresses.Count() << Arch::PageShift);
	if (!base)
	{
		for (size_t i = 0; i < count; i++)
			KePhysicalFree(addresses[i]);
		return false;
	}

	m_buffer = reinterpret_cast<uint8_t*>(base);
	return true;
}

void KPipe::Free()
{
	//The first view holds each page once; read them back before the unmap drops the translations
	const size_t count = Arch::SizeToPages(m_size);
	StaticVector<paddr_t, MaxPages> addresses;
	for (size_t i = 0; i < count; i++)
		Assert(addresses.Add(m_process.Tables.ResolvePhysical(&m_buffer[i << Arch::PageShift])));

	//The mapping only borrows the pages, so they are freed separately
	Assert(KeVirtualFree(m_buffer));
	for (const paddr_t addr : addresses)
		KePhysicalFree(addr);
	m_buffer = nullptr;
}

bool KPipe::Read(void* const buffer, const size_t length, size_t& bytesRead)
{
	bytesRead = 0;
	if (IsEmpty())
		return true;

	//The second mapping makes a run from any index contiguous, so wrapped data is one copy
	bytesRead = Clamp<size_t>(m_count, 0, length);
	memcpy(buffer, &m_buffer[m_readIndex], bytesRead);
	m_readIndex = (m_readIndex + bytesRead) % m_size;
	m_count -= bytesRead;

	return true;
}

bool KPipe::Write(const void* buffer, const size_t length, size_t& bytesWritten)
{
	bytesWritten = 0;
	if (Space() < length)
		return false;

	bytesWritten = length;
	memcpy(&m_buffer[m_writeIndex], buffer, bytesWritten);
	m_writeIndex = (m_writeIndex + bytesWritten) % m_size;
	m_count += bytesWritten;

	return true;
}

//TODO(tsharpe): This method should be renamed printf, and kernel routine named KePrintf
void KPipe::Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	VPrint(format, args);
	va_end(args);
}

void KPipe::VPrint(const char* format, va_list args)
{
	char buffer[1024];
	const int length = vsnprintf(buffer, sizeof(buffer), format, args); //Truncates long output

	size_t bytesWritten = 0;
	Write(buffer, length, bytesWritten);
}

size_t KPipe::Count() const
{
	return m_count;
}

size_t KPipe::Space() const
{
	return m_size - m_count;
}

size_t KPipe::Capacity() const
{
	return m_size;
}

bool KPipe::IsBroken() const
{
	return Readers.IsEmpty() || Writers.IsEmpty();
}

void KPipe::QueueWriter(KPipeWriter& writer)
{
	m_writers.InsertTail(writer);
}

void KPipe::DequeueWriter(KPipeWriter& writer)
{
	m_writers.Remove(writer);
}

bool KPipe::IsUnused() const
{
	return Readers.IsEmpty() && Writers.IsEmpty() && Waiters.IsEmpty();
}

bool KPipe::EventSignal(const KPipeOp op, const size_t count, const KPipeWriter* const writer)
{
	switch (op)
	{
		//Only the write ends closing ends a read without data; a read end's handle closing under a blocked reader doesn't, as on Windows
		case KPipeOp::Read:
			return Writers.IsEmpty() || Count() >= count;

		case KPipeOp::Write:
			return IsBroken() || (!m_writers.IsEmpty() && &m_writers.First() == writer && Space() >= count);

		default:
			Fatal("Invalid pipe op");
			return false;
	}
}

bool KPipe::IsEmpty() const
{
	return m_count == 0;
}
