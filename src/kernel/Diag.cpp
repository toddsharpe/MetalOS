#include "kernel/System.h"
#include "kernel/Vmem.h"
#include "kernel/Process.h"
#include "kernel/File.h"
#include "kernel/Diag.h"
#include "kernel/Arch.h"
#include "kernel/Core/KProcess.h"
#include "kernel/Core/UProcess.h"
#include "Assert.h"
#include "kernel/Kernel.h"
#include "kernel/Scheduler.h"
#include "kernel/Pdb/PdbFile.h"
#include "kernel/Types.h"
#include "Drivers/DirectUart.h"
#include "MetalOS.Loader.h"
#include "MetalOS.Space.h"
#include "WinPE.h"
#include "Lib/Arena.h"
#include "Lib/Path.h"
#include "Lib/String.h"
#include "core_crt/stdarg.h"
#include "core_crt/stdio.h"
#include "core_crt/ctype.h"
#include "Lib/StaticMap.h"

static constexpr size_t TempArenaSize = Arch::PageSize << 10; //4MB Temp Arena
static KERNEL_PAGE_ALIGN uint8_t m_tempBacking[TempArenaSize];
static constinit PreallocatedArena m_tempArena(m_tempBacking);
static DirectUart m_uart(DirectUart::ComPort::Com1);

namespace
{
	//A user process has at most MaxLoadedModules (8) modules and the kernel two, so a trace doesn't fill the PDB map today
	static constexpr size_t MaxTracePdbs = 32;
	static constexpr size_t MaxTraceFrames = 64; //Bounds a corrupt stack that unwinds in a loop

	struct LoadedPdb
	{
		PdbFile* File; //Null when the module has no PDB to load, or one that doesn't match it, so its frames print without symbols
		void* Image; //What the parser reads: the boot-mapped kernel PDB, or a KeLoadFile copy freed after the walk
	};

	//The bootloader maps the kernel's PDB, so kernel frames resolve without the RAM drive, the storage driver or an allocation
	void* LoadPdb(const KProcess& process, const KModule& module)
	{
		if (module.ImageBase == (void*)KernelBase && BootParams.PdbSize != 0)
			return (void*)KernelPdb;

		//A user image is its process's memory to scribble on or free, so it's only read while all of it is mapped (and, below, within its size)
		if (!process.IsGlobal && !process.Space.IsValidRange(module.ImageBase, module.ImageSize))
			return nullptr;

		//Get PDB name (null without a CodeView debug entry, or one that runs outside the image), then load it if it's on the drive
		const char* const pdbName = GetFileName(WinPE::GetPdbName(module.ImageBase, module.ImageSize));
		return pdbName ? KeLoadFile(pdbName) : nullptr;
	}
}

void TracePrint(KPipe* const output, const char* const format, ...)
{
	va_list args;
	va_start(args, format);
	VPrintf(format, args);
	va_end(args);

	//The pipe write never blocks and drops a line that doesn't fit, which is why the console always gets it too
	if (!output)
		return;

	va_start(args, format);
	output->VPrint(format, args);
	va_end(args);
}

void PrintStack(const Arch::Context* context, const KProcess& process, KPipe* const output)
{

	Arena& arena = m_tempArena;
	arena.Reset();
	StaticMap<uintptr_t, LoadedPdb, MaxTracePdbs> pdbs;

	TracePrint(output, "Call Stack\n");

	//A user stack (and image) is the process's memory, so a smashed stack or bad RSP must end the walk rather than fault the kernel
	StackWalk sw(*context, process.IsGlobal ? nullptr : &process.Space);
	size_t frames = 0;
	while (sw.HasNext())
	{
		if (frames++ == MaxTraceFrames)
		{
			TracePrint(output, "    <stopped after %d frames>\n", MaxTraceFrames);
			break;
		}

		//Kernel frames resolve through the kernel's own modules; a user process's come from its PEB, read as process memory
		const uintptr_t ip = sw.LookupIp();
		KModule module;
		const bool found = process.IsGlobal ? KeGetModule(ip, module) : static_cast<const UProcess&>(process).GetModule(ip, module);
		if (!found)
		{
			//No module info for the IP, print it and exit
			TracePrint(output, "    IP: 0x%016x\n", sw.Ip());
			break;
		}

		//Once the map is full, a module not in it prints without symbols, rather than loading (and leaking) its PDB again every frame
		LoadedPdb pdb = {};
		const uintptr_t imageBase = (uintptr_t)module.ImageBase;
		if (!pdbs.Get(imageBase, pdb) && pdbs.Count() < MaxTracePdbs)
		{
			//Parse the module's PDB, if it has one
			pdb.Image = LoadPdb(process, module);
			if (pdb.Image)
			{
				pdb.File = arena.Allocate<PdbFile>();
				if (pdb.File && !pdb.File->Open(pdb.Image, module.ImageBase, module.ImageSize, arena))
					pdb.File = nullptr; //From another build of the module (its names would be wrong), or the image's headers are bad
			}
			pdbs.Add(imageBase, pdb);
		}

		//Calculate RVA
		const uint32_t rva = (uint32_t)(sw.Ip() - imageBase);

		//Lookup in PDB
		PdbLookup lookup = {};
		if (!pdb.File)
			TracePrint(output, "    <no symbols>\n");
		else if (pdb.File->Resolve((uint32_t)(ip - imageBase), lookup))
			TracePrint(output, "    %s (%d)\n", lookup.Function, lookup.Line);
		else
			TracePrint(output, "    <resolve failed>\n");
		TracePrint(output, "        %s IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", module.Name, sw.Ip(), module.ImageBase, rva);

		//The stack or the module's unwind data isn't readable (or usable), so there's no caller to go to
		if (!sw.Next(imageBase))
		{
			TracePrint(output, "    <unwind failed>\n");
			break;
		}
	}

	for (const auto& entry : pdbs)
	{
		if (entry.Value.Image && entry.Value.Image != (void*)KernelPdb)
			KeVirtualFree(entry.Value.Image);
	}
}

void PrintStack()
{
	Arch::Context context = {};
	ArchSaveContext(&context);
	PrintStack(&context, m_process);
}

void VPrintf(const char* format, va_list args)
{
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), format, args); //Truncates long output; always NUL-terminated
	m_uart.Write(buffer);
}

void Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	VPrintf(format, args);

	va_end(args);
}

void PrintBytes(const void* data, const size_t length)
{
	static constexpr uint32_t width = 16;
	const uint8_t* pData = reinterpret_cast<const uint8_t*>(data);

	//Print headers
	Printf("A: 0x%016x S: 0x%016x\n", data, length);
	Printf("---- ");
	for (size_t i = 0; i < width; i++)
	{
		Printf("%02x ", (unsigned char)i);
	}
	Printf("\n");

	char line[width] = { 0 };
	for (size_t i = 0; i < length; i++)
	{
		if (i != 0 && i % width == 0)
		{
			//Print characters
			Printf(" | ");
			for (const auto c : line)
			{
				Printf("%c", isprint(c) ? c : '.');
			}

			memset(line, 0, width);
			Printf("\n");
		}

		if (i % width == 0)
			Printf("%02x - ", (i / width) << 4);

		Printf("%02x ", (unsigned char)pData[i]);
		line[i % width] = (unsigned char)pData[i];
	}

	//Print the rest of the line
	if ((length % width) != 0)
		for (size_t i = 0; i < width - (length % width); i++)
		{
			Printf("   ");
		}

	//Print characters
	Printf(" | ");
	for (size_t i = 0; i < (length % width); i++)
	{
		char c = line[i];
		Printf("%c", isprint(c) ? c : '.');
	}

	Printf("\n");
}

static void VBugcheck(const char* file, const char* line, const char* format, va_list args)
{
	static bool inBugcheck = false;
	KePauseSystem();

	if (inBugcheck)
	{
		//Bugcheck during bugcheck, print what's available and bail
		Printf("Double bug check!\n");
		Printf("\n%s\n%s\n", file, line);
		VPrintf(format, args);
		Printf("\n");

		while (true)
			ArchWait();
	}
	inBugcheck = true;

	Printf("Bugcheck\r\n");
	Printf("    %s\n", file);
	Printf("    %s\n", line);

	VPrintf(format, args);
	Printf("\n");

	Arch::Context context = {};
	ArchSaveContext(&context);
	PrintStack(&context, m_process);

	while(true);
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	VBugcheck(file, line, format, args);
	va_end(args);
}

//Called through a vtable slot whose pure virtual was never overridden, which is always a bug
int __cdecl _purecall(void)
{
	Assert(false);
	return 0;
}
