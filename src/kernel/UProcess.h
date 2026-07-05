#pragma once

#include "kernel/KProcess.h"
#include "Lib/Arena.h"
#include "user/MetalOS.Types.h"
#include "kernel/UObject.h"
#include "Lib/List.h"
#include "Lib/LinkedList.h"
#include "kernel/Types.h"
#include "kernel/Objects/KPipe.h"

enum class UProcessState
{
	Created,
	Running,
	Terminated,
	Last
};


class KThread;
class Scheduler;
class UThread;
class UProcess : public KProcess
{
	friend Scheduler;

public:
	UProcess(const CString& name);

	void Initialize();

	//This has to occur when this process is active (page tables are in use)
	//TODO(tsharpe): Remove this limitation (map into kernel also?)
	void InContextInit(void* const image, const CString& cmd);

	UThread* CreateThread(KThread& backing, const size_t stackSize, const UThreadStart userStart = nullptr, void* const arg = nullptr);
	UObject* CreateObject(const UObjectType type);
	UObject* CreateObject(const UObjectType type, const handle_t handle);
	UObject* GetObject();
	UObject* GetObject(const handle_t handle);
	bool CloseObject(const handle_t handle);

	const KModule* AddModule(const CString& name, void* image);

	void Display() const;

	//Lifecycle: set on teardown so consumers (e.g. the WM liveness check) can tell
	//a process is gone even though its UProcess slot may still be allocated.
	void MarkTerminated() { m_state = UProcessState::Terminated; }
	bool IsAlive() const { return m_state != UProcessState::Terminated; }

	const uint32_t Id;

	//Runtime start addresses
	void* InitProcess;
	void* InitThread;
	bool IsConsole;

	//PageTables Tables;

	 //TODO(tsharpe): Shouldnt be a static string, just string
	StaticString<32> Name;

	LinkedList<UObject*> Objects;

private:
	static uint32_t LastId;

	static constexpr size_t KernelReserve = Arch::PageSize * 4;
	static constexpr size_t UThreadStackSize = (Arch::PageSize << 4); //64K

	//UThreads
	ListHead m_threads;

	//Scheduler
	UProcessState m_state;

	//Userspace process storage (PEB/TEB)
	ProcessEnvironmentBlock* m_peb;
	PreallocatedArena m_userArena;
};
