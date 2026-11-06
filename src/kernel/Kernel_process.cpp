#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"
#include "kernel/UProcess.h"
#include "kernel/UThread.h"
#include "kernel/KThread.h"
#include "kernel/Scheduler.h"
#include "kernel/Objects/KEvent.h"
#include "kernel/Loader.h"
#include "WinPE.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern LinkedList<UProcess*> m_processes;

static constexpr CString RuntimeDLL = "MetalOS-RT.dll";

//The kernel's own modules (moskrnl and kdcom), added at boot and never unloaded; filled in place, as a KModule's name can't be copied
static constexpr size_t MaxKernelModules = 2;
static KModule m_modules[MaxKernelModules];
static size_t m_moduleCount;
static constexpr size_t MaxPathLength = decltype(UProcess::Name)::Size - 1; //The path also names the process

//An image installs at its fixed ImageBase, so it must be page-aligned and end inside user space
static bool FitsUserSpace(const Loader::LoadedImage& image)
{
	const uintptr_t start = reinterpret_cast<uintptr_t>(image.UserBase);
	if ((start & Arch::PageMask) != 0 || start >= UserEnd)
		return false;
	return x64::SizeToPages(image.Size) <= ((UserEnd - start) >> Arch::PageShift);
}

//Checked before the process exists, so KeAliasInto's fixed-address reserve can't fail
static bool CanInstall(const Loader::LoadedImage& exe, const Loader::LoadedImage& runtime)
{
	if (!FitsUserSpace(exe) || !FitsUserSpace(runtime))
		return false;

	const uintptr_t exeStart = reinterpret_cast<uintptr_t>(exe.UserBase);
	const uintptr_t runtimeStart = reinterpret_cast<uintptr_t>(runtime.UserBase);
	const uintptr_t exeEnd = exeStart + (x64::SizeToPages(exe.Size) << Arch::PageShift);
	const uintptr_t runtimeEnd = runtimeStart + (x64::SizeToPages(runtime.Size) << Arch::PageShift);
	return exeEnd <= runtimeStart || runtimeEnd <= exeStart;
}

const KModule* KeAddModule(const CString& name, void* const image, const size_t imageSize)
{
	if (m_moduleCount == MaxKernelModules)
		return nullptr;

	KModule& module = m_modules[m_moduleCount++];
	module.Name.Set(name);
	module.ImageBase = image;
	module.ImageSize = imageSize;
	return &module;
}

bool KeGetModule(const uintptr_t address, KModule& module)
{
	for (size_t i = 0; i < m_moduleCount; i++)
	{
		const KModule& candidate = m_modules[i];
		const uintptr_t base = (uintptr_t)candidate.ImageBase;
		if (address < base || address - base >= candidate.ImageSize)
			continue;

		module.Name.Set(CString(candidate.Name.c_str()));
		module.ImageBase = candidate.ImageBase;
		module.ImageSize = candidate.ImageSize;
		return true;
	}

	return false;
}

const KModule* KeLoadLibrary(const char* const name)
{
	const CString moduleName(name);

	//Check if module is already loaded
	for (size_t i = 0; i < m_moduleCount; i++)
	{
		if (m_modules[i].Name == moduleName)
			return &m_modules[i];
	}

	if (m_moduleCount == MaxKernelModules)
		return nullptr;

	//If it doesn't exist, load it in the kernel, install it at its ImageBase (a kernel VA for a
	//kernel module), then drop the temporary load alias.
	Loader::LoadedImage image = Loader::Load(moduleName);
	if (!image.UserBase)
		return nullptr;
	KeAliasInto(m_process, image.Alias, image.UserBase, image.Size);
	KeVirtualFree(m_process, image.Alias);

	return KeAddModule(moduleName, image.UserBase, image.Size);
}

UProcess* KeCreateProcess(const char* const cmd)
{
	Printf("Creating process: %s\n", cmd);

	//No room to schedule the main thread: fail before anything is built
	if (!Scheduler::HasRoom())
	{
		Printf("Failed to create process: no room to schedule its main thread\n");
		return nullptr;
	}

	//Get path (up to the first space or end of string)
	size_t i = 0;
	while (cmd[i] != '\0' && cmd[i] != ' ')
		i++;
	if (i == 0)
	{
		Printf("Failed to create process: empty command line\n");
		return nullptr;
	}
	if (i > MaxPathLength)
	{
		Printf("Failed to create process: path is %u characters, the limit is %u\n", (uint32_t)i, (uint32_t)MaxPathLength);
		return nullptr;
	}

	StaticString<64> path;
	path.Append(CString(cmd, i));

	//Load exe and runtime into the kernel (each returns a temporary kernel alias for patching).
	Loader::LoadedImage exe = Loader::Load(path);
	if (!exe.UserBase)
	{
		Printf("Failed to load %s\n", path.c_str());
		return nullptr;
	}

	Loader::LoadedImage runtime = Loader::Load(RuntimeDLL);
	if (!runtime.UserBase)
	{
		Printf("Failed to load %s for %s\n", RuntimeDLL.c_str(), path.c_str());
		KeVirtualFree(m_process, exe.Alias);
		return nullptr;
	}

	//Resolve runtime imports (stamps the exe's IAT with the runtime's user addresses) and the init entry points
	void* const initProcess = WinPE::GetProcAddress(runtime.Alias, "InitProcess");
	void* const initThread = WinPE::GetProcAddress(runtime.Alias, "InitThread");
	if (!initProcess)
		Printf("%s does not export InitProcess\n", RuntimeDLL.c_str());
	if (!initThread)
		Printf("%s does not export InitThread\n", RuntimeDLL.c_str());
	if (!initProcess || !initThread || !Loader::ResolveImports(exe, runtime, RuntimeDLL))
	{
		KeVirtualFree(m_process, exe.Alias);
		KeVirtualFree(m_process, runtime.Alias);
		return nullptr;
	}

	if (!CanInstall(exe, runtime))
	{
		Printf("%s: ImageBase 0x%016x is outside user space or overlaps %s\n", path.c_str(), exe.UserBase, RuntimeDLL.c_str());
		KeVirtualFree(m_process, exe.Alias);
		KeVirtualFree(m_process, runtime.Alias);
		return nullptr;
	}

	//Bad images are rejected above, so a failed load never leaves a half-built process in the list
	UProcess* created = KeAlloc<UProcess>(AllocType::User, path);
	Assert(created);
	Assert(m_processes.Add(created));

	//Fresh page tables for the process, with the kernel mapped in. No CR3 switch and no
	//interrupts-off window: the loader aliases each image into the kernel and patches it there,
	//and process structures are written through the physical-memory window (KeWriteProcessMemory).
	created->Tables = KernelPageTables::CreateNew();
	Assert(created->Tables.IsValid());
	created->Tables.LoadKernelMappings();

	//Init pointers: store the process (user) address of each runtime entry point
	created->InitProcess = runtime.ToUser(initProcess);
	created->InitThread = runtime.ToUser(initThread);

	//Install the images into the process at their ImageBase (aliasing the kernel-loaded pages)
	KeAliasInto(*created, exe.Alias, exe.UserBase, exe.Size);
	KeAliasInto(*created, runtime.Alias, runtime.UserBase, runtime.Size);

	//Update process structures
	created->Initialize(exe.UserBase, CString(cmd));

	created->AddModule(path, exe.UserBase);
	created->AddModule(RuntimeDLL, runtime.UserBase);

	created->IsConsole = WinPE::IsConsole(exe.Alias);
	const size_t stackSize = WinPE::GetStackSize(exe.Alias);

	//Done loading; release the temporary kernel aliases
	KeVirtualFree(m_process, exe.Alias);
	KeVirtualFree(m_process, runtime.Alias);

	//Add debug output as stdout for now
	Assert(created->CreateObject(UObjectType::Debug, Handles::StdOut));
	Assert(created->CreateObject(UObjectType::Debug, Handles::StdErr));

	//Fully built; create the main thread last since it's schedulable as soon as it exists
	created->State = ProcessState::Running;
	if (!KeCreateUThread(*created, stackSize))
	{
		//No memory for the main thread's stack: tear down instead of leaving a running process with no threads
		Printf("%s: could not create the main thread (no TEB slot or stack memory)\n", path.c_str());
		KeTerminateProcess(*created, 1);
		return nullptr;
	}
	return created;
}

void KeTerminateProcess(UProcess& process, const uint32_t exitCode)
{
	//Already torn down, e.g. TerminateProcess through a handle to a process that has exited
	if (process.State == ProcessState::Terminated)
		return;

	Printf("Process: %s exited with code 0x%x\n", process.Name.c_str(), exitCode);

	//Mark dead first: KillProcess() does not return when a process terminates
	//itself (ExitProcess), so anything after it would be skipped. This also lets
	//liveness queries (KeIsProcessAlive) report the process as gone.
	process.State = ProcessState::Terminated;

	//Close all objects. This also releases the process's shared-memory mappings
	//(CloseObject unmaps each SharedMemory handle), freeing regions whose last
	//holder is now gone.
	UObject* top = process.GetObject();
	while (top != nullptr)
	{
		Assert(process.CloseObject(top->Handle)); //On the target, not the caller (TerminateProcess runs in the parent)

		//Get next object
		top = process.GetObject();
	}

	//Free the names it served so a restarted server can register them
	KeEndpointRelease(process.Id);

	//Revoke grants nobody claimed, which would otherwise keep their objects alive forever
	KeGrantRevoke(process.Id);

	//The reaper frees the rest once its threads are off the CPU (a process with none is reapable now)
	KeSignalReaper();

	//Kill all KThreads in process (does not return for a self-terminating process)
	Scheduler::KillProcess(process);
}

void KeExitUThread(UThread& thread, const uint32_t exitCode)
{
	//Already exited
	if (!thread.Thread)
		return;

	//Another live thread keeps the process running; as on Windows, the last one ending ends the process with its exit code, else nothing would terminate (or reap) it
	for (const UThread* const other : thread.Process.Threads)
	{
		if (other != &thread && other->Thread && other->Thread->Sched.State != KThreadState::Terminated)
		{
			KeExitThread(*thread.Thread);
			return;
		}
	}

	KeTerminateProcess(thread.Process, exitCode);
}

//Reclaims what exited threads and processes leave, from the reaper's own stack under the kernel's page tables
namespace _REAP
{
	KEvent m_event(false, false);

	//Nothing runs on an exited thread's user stack, so it goes now even if handles keep the record
	void ReleaseUserStack(UThread& thread)
	{
		const uintptr_t stack = reinterpret_cast<uintptr_t>(thread.StackBase);
		if (stack && thread.Process.Space.GetCount(stack))
			Assert(KeVirtualFree(thread.Process, thread.StackBase));
		thread.StackBase = nullptr;
	}

	//A thread killed while queued to write a pipe never finishes its write, so it leaves the queue here and the next writer goes
	void LeavePipeWriteQueue(UThread& thread)
	{
		if (!thread.PipeWriteQueue)
			return;

		thread.PipeWriteQueue->DequeueWriter(thread.PipeWriteLink);
		thread.PipeWriteQueue = nullptr;
	}

	//A thread killed while blocked on a pipe never finished its syscall, so its hold on the pipe is dropped here
	void ReleasePipeWait(UThread& thread)
	{
		if (!thread.PipeWait)
			return;

		KPipe& pipe = *thread.PipeWait;
		thread.PipeWait = nullptr;
		KePipeUnref(pipe, pipe.Waiters);
	}

	//A thread killed while waiting on an event never finished its wait, so its hold on the event is dropped here
	void ReleaseEventWait(UThread& thread)
	{
		if (!thread.EventWait)
			return;

		KEvent& event = *thread.EventWait;
		thread.EventWait = nullptr;
		KeEventUnref(event);
	}

	//A terminated process is reapable once its last thread record is gone
	UProcess* FindReapable()
	{
		for (UProcess* const process : m_processes)
		{
			if (process->State == ProcessState::Terminated && !process->Reaped && process->Threads.IsEmpty())
				return process;
		}
		return nullptr;
	}

	//Frees a terminated process's memory, page tables and modules; the record lives on while handles to it remain
	void ReapProcess(UProcess& process)
	{
		//One reservation at a time with interrupts off: the heap and page allocator have no locks
		while (true)
		{
			const cpu_flags_t flags = ArchDisableInterrupts();
			const uintptr_t reservation = process.Space.FirstReservation();
			if (reservation)
				Assert(KeVirtualFree(process, reinterpret_cast<void*>(reservation)));
			ArchRestoreFlags(flags);

			if (!reservation)
				break;
		}

		const cpu_flags_t flags = ArchDisableInterrupts();
		KeFreePageTables(process);
		process.Reaped = true;
		KeReleaseUProcess(process);
		ArchRestoreFlags(flags);
	}
}

void KeSignalReaper()
{
	_REAP::m_event.Set();
}

uint32_t KeReaperThread(void* const arg)
{
	UNUSED(arg);

	while (true)
	{
		KeWait(_REAP::m_event);

		//Threads first: releasing a process's last thread record is what makes the process reapable
		while (KThread* const thread = Scheduler::TakeDeadThread())
		{
			const cpu_flags_t flags = ArchDisableInterrupts();
			thread->ReleaseStack();
			if (thread->UserThread)
			{
				_REAP::ReleaseUserStack(*thread->UserThread);
				_REAP::LeavePipeWriteQueue(*thread->UserThread);
				_REAP::ReleasePipeWait(*thread->UserThread);
				_REAP::ReleaseEventWait(*thread->UserThread);
				KeReleaseUThread(*thread->UserThread);
			}
			KeFree(thread, AllocType::Kernel);
			ArchRestoreFlags(flags);
		}

		while (true)
		{
			const cpu_flags_t flags = ArchDisableInterrupts();
			UProcess* const process = _REAP::FindReapable();
			ArchRestoreFlags(flags);

			if (!process)
				break;
			_REAP::ReapProcess(*process);
		}
	}
}

void KeReleaseUThread(UThread& thread)
{
	Assert(thread.Refs);
	if (--thread.Refs != 0)
		return;

	//Last reference: its KThread (and with it the user stack) is gone and no handle names it
	UProcess& process = thread.Process;
	Assert(process.Threads.Remove(&thread));
	if (thread.Name.Length)
		KeFree(const_cast<char*>(thread.Name.c_str()), AllocType::User);
	KeFree(&thread, AllocType::User);
}

void KeReleaseUProcess(UProcess& process)
{
	Assert(process.Refs);
	if (--process.Refs != 0)
		return;

	//Reaping drops the process's own reference, so at zero only the record is left
	Assert(process.Reaped);
	Assert(m_processes.Remove(&process));
	KeFree(&process, AllocType::User);
}

bool KeIsProcessAlive(const uint32_t id)
{
	for (UProcess* const process : m_processes)
	{
		if (process->Id == id && process->State != ProcessState::Terminated)
			return true;
	}
	return false;
}

size_t KeGetProcessList(ProcessListEntry* const buffer, const size_t maxCount)
{
	size_t n = 0;
	for (UProcess* const process : m_processes)
	{
		if (n >= maxCount)
			break;

		ProcessListEntry& entry = buffer[n];
		entry.Id = process->Id;
		entry.VirtualSize = process->Space.ReservedBytes();
		entry.State = process->State;

		strncpy(entry.Name, process->Name.c_str(), MaxProcessName - 1);
		entry.Name[MaxProcessName - 1] = '\0';

		n++;
	}
	return n;
}
