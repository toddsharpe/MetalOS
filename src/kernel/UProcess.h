#pragma once

#include "kernel/KProcess.h"
#include "Lib/Arena.h"
#include "user/MetalOS.Types.h"
#include "kernel/UObject.h"
#include "Lib/List.h"
#include "Lib/LinkedList.h"
#include "kernel/Types.h"
#include "kernel/Objects/KPipe.h"

class KThread;
class UThread;
class UProcess : public KProcess
{
public:
	UProcess(const CString& name);

	void Initialize(void* const image, const CString& cmd);

	UThread* CreateThread(KThread& backing, const size_t stackSize, const UThreadStart userStart = nullptr, void* const arg = nullptr);
	UObject* CreateObject(const UObjectType type);
	UObject* CreateObject(const UObjectType type, const handle_t handle);
	UObject* GetObject();
	UObject* GetObject(const handle_t handle);
	bool CloseObject(const handle_t handle);

	const KModule* AddModule(const CString& name, void* image);

	void Display() const;

	const uint32_t Id;

	//Runtime start addresses
	void* InitProcess;
	void* InitThread;
	bool IsConsole;

	//TODO(tsharpe): Shouldnt be a static string, just string
	StaticString<32> Name;

	LinkedList<UObject*> Objects;
	LinkedList<UThread*> Threads;

	//Lifecycle: Created at construction, Running once fully built and schedulable, Terminated on
	//teardown (consumers like the WM liveness check read this directly).
	ProcessState State;

private:
	static uint32_t LastId;

	static constexpr size_t KernelReserve = Arch::PageSize * 4;
	static constexpr size_t UThreadStackSize = (Arch::PageSize << 4); //64K

	//Userspace process storage (PEB/TEB)
	ProcessEnvironmentBlock* m_peb;
	PreallocatedArena m_userArena;
	size_t m_moduleIndex; //kernel-side PEB->LoadedModules count (avoids reading user memory)
};
