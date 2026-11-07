#include "kernel/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Objects/KEvent.h"
#include "Assert.h"

namespace
{
	constexpr WaitStatus FromWaitResult(const KWaitResult result)
	{
		switch (result)
		{
			case KWaitResult::Signaled:
				return WaitStatus::Signaled;

			case KWaitResult::Timeout:
				return WaitStatus::Timeout;

			default:
				return WaitStatus::None;
		}
	}

	//Keeps an event alive while a thread waits on it, as another thread may close the handle meanwhile; the wait goes on, as on Windows
	class EventHold
	{
	public:
		EventHold(UThread& thread, KEvent& event) :
			m_thread(thread)
		{
			Assert(!thread.EventWait);
			event.Refs.Increment();
			thread.EventWait = &event;
		}

		//Never runs for a thread killed while waiting, so the reaper drops its hold instead
		~EventHold()
		{
			KEvent& event = *m_thread.EventWait;
			m_thread.EventWait = nullptr;
			KeEventUnref(event);
		}

	private:
		UThread& m_thread;
	};
}

SyscallResult WaitForSingleObject(const Handle handle, const milli_t time, WaitStatus* status)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!status || !proc.Space.IsValid(status))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::Event:
		{
			KEvent& event = *obj->Event;
			EventHold hold(Scheduler::GetUThread(), event);
			*status = FromWaitResult(KeWait(event, time));
		}
		break;

		default:
			return SyscallResult::InvalidObject;
	}

	return SyscallResult::Success;
}

SyscallResult CreateEvent(HEvent* event, const bool manual, const bool initial)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!event || !proc.Space.IsValid(event))
		return SyscallResult::InvalidPointer;

	//Create Event
	UObject* created = proc.CreateObject(UObjectType::Event);
	created->Event = KeAlloc<KEvent>(AllocType::User, manual, initial);
	created->Event->Refs.Increment(); //The handle's, dropped when it's closed

	*event = (HEvent)created->Handle;
	return SyscallResult::Success;
}

SyscallResult SetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, event, UObjectType::Event, obj);
	if (status != SyscallResult::Success)
		return status;

	Assert(obj->Event);
	obj->Event->Set();
	return SyscallResult::Success;
}

SyscallResult ResetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, event, UObjectType::Event, obj);
	if (status != SyscallResult::Success)
		return status;

	Assert(obj->Event);
	obj->Event->Reset();
	return SyscallResult::Success;
}
