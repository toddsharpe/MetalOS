#pragma once

#include "kernel/Objects/KSignalObject.h"
#include "kernel/KProcess.h"
#include "Lib/Arena.h"
#include "user/MetalOS.Types.h"
#include "kernel/UObject.h"
#include "Lib/List.h"
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
class UProcess : public KProcess, public KSignalObject
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
	UObject* CreateEvent(const bool manual, const bool initial);
	UObject* CreatePipe(KPipe& pipe, const KPipeOp op);
	UObject* CreatePipe(KPipe& pipe, const KPipeOp op, const handle_t handle);
	UObject* GetObject();
	UObject* GetObject(const handle_t handle);
	bool CloseObject(const handle_t handle);

	KModule* AddModule(const CString& name, void* image);

	virtual bool IsSignalled() const;

	void Display() const;

	const uint32_t Id;

	//Runtime start addresses
	void* InitProcess;
	void* InitThread;
	bool IsConsole;

	PageTables Tables;

	 //TODO(tsharpe): Shouldnt be a static string, just string
	StaticString<32> Name;

private:
	static uint32_t LastId;

	static constexpr size_t KernelReserve = PageSize * 4;
	static constexpr size_t UThreadStackSize = (PageSize << 4); //64K

	//UThreads
	ListHead m_threads;

	//Scheduler
	UProcessState m_state;

	//UObjects
	ListHead m_objects;

	//Userspace process storage (PEB/TEB)
	ProcessEnvironmentBlock* m_peb;
	PreallocatedArena m_userArena;
};
