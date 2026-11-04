#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "kernel/KProcess.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;
extern DirectUart m_uart;
extern StaticArena<TempArenaSize> m_tempArena;

namespace
{
	//A stack trace goes to the kernel console, or to a faulting process's stdout pipe
	void TracePrint(KPipe* const output, const char* const format, ...)
	{
		va_list args;
		va_start(args, format);
		if (output)
			output->VPrint(format, args);
		else
			VPrintf(format, args);
		va_end(args);
	}

	struct LoadedPdb
	{
		PdbFile* File;
		void* Image; //KeLoadFile copy the parser reads from, freed after the walk
	};
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
			//Get PDB name
			const char* fullPath = WinPE::GetPdbName(module->ImageBase);
			Assert(fullPath);
			const char* pdbName = GetFileName(fullPath);
			Assert(pdbName);

			//Load PDB into memory and parse it
			pdb.Image = KeLoadFile(pdbName);
			pdb.File = arena.Allocate<PdbFile>();
			pdb.File->Open(pdb.Image, module->ImageBase, arena);
			pdbs.Add(module, pdb);
		}

		//Calculate RVA
		const uint32_t rva = (uint32_t)(ctx.Rip - (uintptr_t)module->ImageBase);

		//Lookup in PDB
		PdbLookup lookup = {};
		if (pdb.File->Resolve(rva, lookup))
			TracePrint(output, "    %s (%d)\n", lookup.Function.c_str(), lookup.Line);
		else
			TracePrint(output, "    <resolve failed>\n");
		TracePrint(output, "        %s IP: 0x%016x Base: 0x%016x, RVA: 0x%08x\n", module->Name.c_str(), ctx.Rip, module->ImageBase, rva);
		sw.Next((uintptr_t)module->ImageBase);
	}

	for (const auto& entry : pdbs)
		KeVirtualFree(entry.Value.Image);
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

void Bugcheck(const char* file, const char* line, const char* format, va_list args)
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
	Bugcheck(file, line, format, args);
	va_end(args);
}
