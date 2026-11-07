#include "kernel/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Arch.h"
#include "kernel/Objects/KPipe.h"
#include "kernel/Objects/KPredicate.h"
#include "Lib/String.h"
#include "Assert.h"
#include "core_crt/ctype.h"

namespace
{
	constexpr KFileAccess ToAccess(const FileAccess access)
	{
		switch (access)
		{
			case FileAccess::Read:
				return KFileAccess::Read;

			case FileAccess::Write:
				return KFileAccess::Write;

			case FileAccess::ReadWrite:
				return KFileAccess::ReadWrite;

			default:
				return KFileAccess::None;
		}
	}

	//Keeps a pipe alive while a syscall can block on it, as another thread may close the handle meanwhile
	class PipeHold
	{
	public:
		PipeHold(UThread& thread, KPipe& pipe) :
			m_thread(thread)
		{
			Assert(!thread.PipeWait);
			pipe.Waiters.Increment();
			thread.PipeWait = &pipe;
		}

		//Never runs for a thread killed while blocked, so the reaper drops its hold instead
		~PipeHold()
		{
			KPipe& pipe = *m_thread.PipeWait;
			m_thread.PipeWait = nullptr;
			KePipeUnref(pipe, pipe.Waiters);
		}

	private:
		UThread& m_thread;
	};
}

HFile CreateFile(const char* path, const FileAccess access)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(path))
		return nullptr;

	UObject* uFile = proc.CreateObject(UObjectType::File);
	if (!uFile)
		return nullptr;

	//The pointer check covers only the first byte, so bound the copy by what's mapped
	char copy[128] = {};
	StrCopy(copy, CStr(path, UserStringLength(proc, path, sizeof(copy) - 1)));
	for (char* c = copy; *c != '\0'; c++)
		*c = (char)tolower(*c);

	const bool result = KeCreateFile(uFile->File, copy, ToAccess(access));
	if (!result)
	{
		//Don't leave an unopened file object behind
		proc.CloseObject(uFile->Handle);
		return nullptr;
	}
	return (HFile)uFile->Handle;
}

SyscallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	UProcess& proc = Scheduler::GetUProcess();

	//A zero-length read touches no buffer, as on Windows
	if ((bufferSize && !proc.Space.IsValidRange(buffer, bufferSize)) || (bytesRead && !proc.Space.IsValid(bytesRead)))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	if (!obj->CanRead())
		return SyscallResult::InvalidObject;

	switch (obj->Type)
	{
		case UObjectType::File:
		{
			const bool result = KeReadFile(obj->File, buffer, bufferSize, bytesRead);
			return result ? SyscallResult::Success : SyscallResult::Failed;
		}
		break;

		case UObjectType::Pipe:
		{
			//The handle can be closed during the wait, so use the pipe itself and hold it until done
			KPipe& pipe = *obj->Pipe;
			PipeHold hold(Scheduler::GetUThread(), pipe);

			//Wait for at least one byte or for every write end to close; readers woken by the same bytes take turns, so one that finds the pipe empty again waits again
			while (pipe.Count() == 0 && !pipe.Writers.IsEmpty())
			{
				KPipeSignal signal(pipe, KPipeOp::Read, 1);
				KPredicate wait(&KPipe::EventSignal, &signal);
				KWaitResult result = KeWait(wait);
				Assert(result == KWaitResult::Signaled);
			}

			//Another thread can also free the buffer or the count during the wait, so check both again before using either
			if ((bufferSize && !proc.Space.IsValidRange(buffer, bufferSize)) || (bytesRead && !proc.Space.IsValid(bytesRead)))
				return SyscallResult::InvalidPointer;

			//Bytes a writer left before closing are still returned; only an empty pipe with no write end left is the end
			if (pipe.Count() == 0)
			{
				if (bytesRead)
					*bytesRead = 0;
				return SyscallResult::BrokenPipe;
			}

			//Read what's there, up to the buffer; a zero-length read only waits for data
			size_t bytes = 0;
			Assert(pipe.Read(buffer, bufferSize, bytes));
			if (bytesRead)
				*bytesRead = bytes;
			return SyscallResult::Success;
		}
		break;

		default:
			Trace();
			return SyscallResult::Failed;
	}
}

SyscallResult WriteFile(const HFile handle, const void* buffer, const size_t bufferSize, size_t* bytesWritten)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!bufferSize)
		return SyscallResult::InvalidArg;
	if (!proc.Space.IsValidRange(buffer, bufferSize) || (bytesWritten && !proc.Space.IsValid(bytesWritten)))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;
	
	if (!obj->CanWrite())
		return SyscallResult::InvalidObject;

	switch (obj->Type)
	{
		case UObjectType::Pipe:
		{
			//The handle can be closed during the wait, so use the pipe itself and hold it until done
			KPipe& pipe = *obj->Pipe;
			UThread& writer = Scheduler::GetUThread();
			PipeHold hold(writer, pipe);
			const uint8_t* const data = static_cast<const uint8_t*>(buffer);

			//A write that fits in the pipe goes in whole, a larger one in pieces as the reader drains it
			const size_t needed = (bufferSize <= pipe.Capacity()) ? bufferSize : 1;

			//Writes are queued in arrival order and each finishes before the next starts, so writers' bytes never interleave
			pipe.QueueWriter(writer.PipeWriteLink);
			writer.PipeWriteQueue = &pipe;

			size_t written = 0;
			SyscallResult status = SyscallResult::Success;
			while (written < bufferSize)
			{
				//Wait for this write's turn and for room
				KPipeSignal signal(pipe, KPipeOp::Write, needed, &writer.PipeWriteLink);
				KPredicate wait(&KPipe::EventSignal, &signal);
				KWaitResult result = KeWait(wait);
				Assert(result == KWaitResult::Signaled);

				//The wait also ends when the reader is gone, with possibly no room to write
				if (pipe.IsBroken())
				{
					status = SyscallResult::BrokenPipe;
					break;
				}

				//Another thread of the process may have freed the rest of the buffer during the wait
				if (!proc.Space.IsValidRange(data + written, bufferSize - written))
				{
					status = SyscallResult::InvalidPointer;
					break;
				}

				size_t bytes = 0;
				const size_t piece = Min(bufferSize - written, pipe.Space());
				Assert(pipe.Write(data + written, piece, bytes));
				written += bytes;
			}

			pipe.DequeueWriter(writer.PipeWriteLink);
			writer.PipeWriteQueue = nullptr;

			//The count may have been freed during a wait as well
			if (bytesWritten)
			{
				if (!proc.Space.IsValid(bytesWritten))
					return SyscallResult::InvalidPointer;
				*bytesWritten = written;
			}
			return status;
		}
		break;

		case UObjectType::Debug:
		{
			//User text is data, never a format string. Buffer was validated for bufferSize bytes.
			const char* const s = (char*)buffer;
			const size_t length = (bufferSize < MaxDebugPrint) ? bufferSize : MaxDebugPrint;
			Printf("%.*s", (int)length, s);
			return SyscallResult::Success;
		}
		break;

		default:
			Assert(false);
			return SyscallResult::Failed;
	}
}

SyscallResult SetFilePointer(const HFile handle, const ssize_t position, const Seek seek, size_t* newPosition)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (newPosition && !proc.Space.IsValid(newPosition))
		return SyscallResult::InvalidPointer;

	UObject* uFile = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::File, uFile);
	if (status != SyscallResult::Success)
		return status;

	size_t newPos = 0;
	switch (seek)
	{
		case Seek::Begin:
			newPos = position;
			break;

		case Seek::Current:
			newPos = uFile->File.Position + position;
			break;

		case Seek::End:
			newPos = uFile->File.Length + position;
			break;

		default:
			return SyscallResult::InvalidArg;
	}

	const bool result = KeSetFilePosition(uFile->File, newPos);
	if (!result)
		return SyscallResult::Failed;

	if (newPosition)
		*newPosition = newPos;

	return SyscallResult::Success;
}

//The only storage is the read-only RAM drive, so nothing can be renamed, deleted or created
SyscallResult MoveFile(const char* existingFileName, const char* newFileName)
{
	return SyscallResult::NotImplemented;
}

SyscallResult DeleteFile(const char* fileName)
{
	return SyscallResult::NotImplemented;
}

SyscallResult CreateDirectory(const char* path)
{
	return SyscallResult::NotImplemented;
}

SyscallResult CreatePipe(HFile* readHandle, HFile* writeHandle)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!readHandle || !proc.Space.IsValid(readHandle))
		return SyscallResult::InvalidPointer;
	if (!writeHandle || !proc.Space.IsValid(writeHandle))
		return SyscallResult::InvalidPointer;

	static constexpr size_t pipeSize = Arch::PageSize;
	
	//Create kernel pipe; out of memory fails the call with nothing left allocated
	KPipe* pipe = KeAlloc<KPipe>(AllocType::Kernel, pipeSize);
	if (!pipe)
		return SyscallResult::Failed;
	if (!pipe->Init())
	{
		KeFree(pipe, AllocType::Kernel);
		return SyscallResult::Failed;
	}
	pipe->Readers.Increment();
	pipe->Writers.Increment();

	//Create reader
	{
		UObject* reader = proc.CreateObject(UObjectType::Pipe);
		reader->Pipe = pipe;
		reader->PipeOp = KPipeOp::Read;
		*readHandle = (HFile)reader->Handle;
	}

	//Create writer
	{
		UObject* writer = proc.CreateObject(UObjectType::Pipe);
		writer->Pipe = pipe;
		writer->PipeOp = KPipeOp::Write;
		*writeHandle = (HFile)writer->Handle;
	}

	return SyscallResult::Success;
}

SyscallResult PeekNamedPipe(const HFile file, size_t* bytesAvailable)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!bytesAvailable || !proc.Space.IsValid(bytesAvailable))
		return SyscallResult::InvalidPointer;

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, file, UObjectType::Pipe, obj);
	if (status != SyscallResult::Success)
		return status;

	*bytesAvailable = obj->Pipe->Count();
	return obj->Pipe->IsBroken() ? SyscallResult::BrokenPipe : SyscallResult::Success;
}
