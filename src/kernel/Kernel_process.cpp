#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"
#include "kernel/UProcess.h"
#include "kernel/Loader.h"
#include "WinPE.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern LinkedList<UProcess*> m_processes;

static constexpr CString RuntimeDLL = "MetalOS-RT.dll";

const KModule* KeLoadLibrary(const char* const name)
{
	const CString moduleName(name);

	//Check if module exists in process
	const KModule* search = m_process.Modules.GetModule(moduleName);
	if (search != nullptr)
		return search;

	//If it doesn't exist, load it in the kernel, install it at its ImageBase (a kernel VA for a
	//kernel module), then drop the temporary load alias.
	Loader::LoadedImage image = Loader::Load(moduleName);
	if (!image.UserBase)
		return nullptr;
	KeAliasInto(m_process, image.Alias, image.UserBase, image.Size);
	KeVirtualFree(m_process, image.Alias);

	const KModule* created = m_process.Modules.AddModule(moduleName, image.UserBase);
	return created;
}

UProcess* KeCreateProcess(const char* const cmd)
{
	Printf("Creating process: %s\n", cmd);

	//Get path (up to the first space or end of string)
	size_t i = 0;
	while (cmd[i] != '\0' && cmd[i] != ' ')
		i++;
	if (i == 0)
		return nullptr; //Empty command line (reachable from the CreateProcess syscall)

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
		KeVirtualFree(m_process, exe.Alias);
		return nullptr;
	}

	//Resolve runtime imports (stamps the exe's IAT with the runtime's user addresses) and the init entry points
	void* const initProcess = WinPE::GetProcAddress(runtime.Alias, "InitProcess");
	void* const initThread = WinPE::GetProcAddress(runtime.Alias, "InitThread");
	if (!Loader::ResolveImports(exe, runtime, RuntimeDLL) || !initProcess || !initThread)
	{
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

	Assert(created->AddModule(path, exe.UserBase));
	Assert(created->AddModule(RuntimeDLL, runtime.UserBase));

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
	KeCreateUThread(*created, stackSize);
	return created;
}

void KeTerminateProcess(UProcess& process, const uint32_t exitCode)
{
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

	//Kill all KThreads in process (does not return for a self-terminating process)
	Scheduler::KillProcess(process);
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
