#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"
#include "kernel/UProcess.h"
#include "kernel/Loader.h"
#include "WinPE.h"
#include "Assert.h"

static constexpr CString RuntimeDLL = "mosrt.dll";

const KModule* KeLoadLibrary(const CString& name)
{
	//Check if module exists in process
	const KModule* search = m_process.Modules.GetModule(name);
	if (search != nullptr)
		return search;

	//If it doesnt exists attempt to load it
	void* address = Loader::Load(m_process, name);
	if (!address)
		return nullptr;

	const KModule* created = m_process.Modules.AddModule(name, address);
	return created;
}

//Since this method changes page tables, make sure every return restores original
UProcess* KeCreateProcess(const CString& cmd)
{
	//TODO(tsharpe): Make this not necessary
	KernelPageTables current = KernelPageTables::Current();

	//Get path
	size_t i = 0;
	while (i < cmd.Length && cmd[i] != '\0' && cmd[i] !=' ')
		i++;
	Assert(i > 0);

	StaticString<64> path;
	path.Append(CString(cmd.c_str(), i));

	//Create UProcess
	UProcess* created = m_procArena.Allocate(path);
	Assert(created);
	created->Initialize();

	//Initialize and switch to new page tables. Interrupts are disabled for the
	//duration of the switch: the running thread belongs to the kernel process, so
	//any reschedule would reload CR3 from it (Scheduler) and discard this override.
	//Nothing in the load path blocks (RamDrive reads are a synchronous memcpy), so
	//disabling interrupts cannot deadlock.
	created->Tables = KernelPageTables::CreateNew();
	Assert(created->Tables.IsValid());
	created->Tables.LoadKernelMappings();
	const cpu_flags_t flags = ArchDisableInterrupts();
	ArchSetPagingRoot(created->Tables.Root);

	//Load exe and runtime into process
	void* address = Loader::Load(*created, path);
	if (!address)
	{
		ArchSetPagingRoot(current.Root);
		ArchRestoreFlags(flags);
		return nullptr;
	}

	void* runtime = Loader::Load(*created, RuntimeDLL);
	if (!runtime)
	{
		ArchSetPagingRoot(current.Root);
		ArchRestoreFlags(flags);
		return nullptr;
	}

	//Resolve runtime imports
	Loader::ResolveImports(address, runtime, RuntimeDLL);

	//Init pointers
	created->InitProcess = WinPE::GetProcAddress(runtime, "InitProcess");
	Assert(created->InitProcess);
	created->InitThread = WinPE::GetProcAddress(runtime, "InitThread");
	Assert(created->InitThread);

	//Update process structures
	created->InContextInit(address, cmd);

	Assert(created->AddModule(path, address));
	Assert(created->AddModule(RuntimeDLL, runtime));

	created->IsConsole = WinPE::IsConsole(address);

	//Create main process thread
	const size_t stackSize = WinPE::GetStackSize(address);
	KeCreateUThread(*created, stackSize);
	ArchSetPagingRoot(current.Root);
	ArchRestoreFlags(flags);

	//Add debug output as stdout for now
	Assert(created->CreateObject(UObjectType::Debug, Handles::StdOut));
	Assert(created->CreateObject(UObjectType::Debug, Handles::StdErr));
	return created;
}

void KeTerminateProcess(UProcess& process, const uint32_t exitCode)
{
	Printf("Process: %s exited with code 0x%x\n", process.Name.c_str(), exitCode);

	//Close all objects
	UObject* top = process.GetObject();
	while (top != nullptr)
	{
		//TODO(tsharpe): Should this call internal method?
		CloseHandle((Handle)top->Handle);

		//Get next object
		top = process.GetObject();
	}

	//Kill all KThreads in process
	m_scheduler.KillProcess(process);
}
