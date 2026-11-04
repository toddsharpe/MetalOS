#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern DirectUart m_uart;
extern StaticArena<TempArenaSize> m_tempArena;
extern LoaderParams BootParams;

namespace
{
	struct LoadedPdb
	{
		PdbFile* File; //Null when the module has no PDB to load, so its frames print without symbols
		void* Image; //What the parser reads: the boot-mapped kernel PDB, or a KeLoadFile copy freed after the walk
	};

	//The bootloader maps the kernel's PDB, so kernel frames resolve without the RAM drive, the storage driver or an allocation
	void* LoadPdb(const KModule& module)
	{
		if (module.ImageBase == (void*)KernelBase && BootParams.PdbSize != 0)
			return (void*)KernelPdb;

		//Get PDB name (null without a CodeView debug entry), then load it if it's on the drive
		const char* const pdbName = GetFileName(WinPE::GetPdbName(module.ImageBase));
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
	//Convert to unwind context
	//NOTE(tsharpe): Convert unwind code to work on X64_CONTEXT
	CONTEXT ctx = {};
	ctx.Rip = context->Rip;
	ctx.Rsp = context->Rsp;
	ctx.Rbp = context->Rbp;

	Arena& arena = m_tempArena;
	arena.Reset();
	StaticMap<const KModule*, LoadedPdb, 32> pdbs;

	TracePrint(output, "Call Stack\n");
	StackWalk sw(&ctx);
	while (sw.HasNext())
	{
		//Get module
		const KModule* module = process.Modules.GetModule(ctx.Rip);
		if (!module)
		{
			//No module info for the IP, print it and exit
			TracePrint(output, "    IP: 0x%016x\n", ctx.Rip);
			break;
		}

		LoadedPdb pdb = {};
		if (!pdbs.Get(module, pdb))
		{
			//Parse the module's PDB, if it has one
			pdb.Image = LoadPdb(*module);
			if (pdb.Image)
			{
				pdb.File = arena.Allocate<PdbFile>();
				pdb.File->Open(pdb.Image, module->ImageBase, arena);
			}
			pdbs.Add(module, pdb);
		}

		//Calculate RVA
		const uint32_t rva = (uint32_t)(ctx.Rip - (uintptr_t)module->ImageBase);

		//Lookup in PDB
		PdbLookup lookup = {};
		if (!pdb.File)
			TracePrint(output, "    <no symbols>\n");
		else if (pdb.File->Resolve(rva, lookup))
			TracePrint(output, "    %s (%d)\n", lookup.Function.c_str(), lookup.Line);
		else
			TracePrint(output, "    <resolve failed>\n");
		TracePrint(output, "        %s IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", module->Name.c_str(), ctx.Rip, module->ImageBase, rva);
		sw.Next((uintptr_t)module->ImageBase);
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
	const uint32_t width = 16;
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

void VBugcheck(const char* file, const char* line, const char* format, va_list args)
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
