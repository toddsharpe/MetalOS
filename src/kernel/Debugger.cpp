#include "Kernel.h"
#include "Assert.h"

#include <algorithm>
#include <windows/types.h>
#include <sal.h>
#include <ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/amd64/ke.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kernel/kddll.h>
namespace Kd64
{
	#include <kernel.kd64/kd64.h>
	extern "C" KdDll m_dll;
}
#include "Debugger.h"
#include "Loader.h"
#include "PortableExecutable.h"

size_t Debugger::ThreadLoop(void* arg)
{
	return static_cast<Debugger*>(arg)->ThreadLoop();
};

Debugger::Debugger()
{
	/* Fill out the KD Version Block */
	Kd64::KdVersionBlock.MajorVersion = DBGKD_MAJOR_EFI;
	Kd64::KdVersionBlock.MinorVersion = 7601;
	
	/* Set protocol limits */
	Kd64::KdVersionBlock.MaxStateChange = DbgKdMaximumStateChange -
		DbgKdMinimumStateChange;
	Kd64::KdVersionBlock.MaxManipulate = DbgKdMaximumManipulate -
		DbgKdMinimumManipulate;
	Kd64::KdVersionBlock.Unused[0] = 0;

	Kd64::KdVersionBlock.KernBase = KernelBaseAddress;
	Kd64::KdVersionBlock.PsLoadedModuleList = (ULONG64)(LONG_PTR)&PsLoadedModuleList;

	InitializeListHead(&PsLoadedModuleList);
}

void Debugger::Initialize()
{
	KeLibrary& kddll = kernel.KeLoadLibrary("kdcom.dll");

	//Load pointers
	Kd64::m_dll.KdInitialize = static_cast<OnKdInitialize>(PortableExecutable::GetProcAddress(kddll.ImageBase, "KdInitialize"));
	Kd64::m_dll.KdReceivePacket = static_cast<OnKdReceivePacket>(PortableExecutable::GetProcAddress(kddll.ImageBase, "KdReceivePacket"));
	Kd64::m_dll.KdSendPacket = static_cast<OnKdSendPacket>(PortableExecutable::GetProcAddress(kddll.ImageBase, "KdSendPacket"));

	kernel.Printf("KdInitialize loaded at 0x%016x\n", Kd64::m_dll.KdInitialize);
	kernel.Printf("KdReceivePacket loaded at 0x%016x\n", Kd64::m_dll.KdReceivePacket);
	kernel.Printf("KdSendPacket loaded at 0x%016x\n", Kd64::m_dll.KdSendPacket);

	NTSTATUS result = Kd64::m_dll.KdInitialize(2);
	kernel.Printf("KdInitialized %d\n", result);

	KThread* thread = kernel.KeCreateThread(Debugger::ThreadLoop, this);
	thread->Name = "Debugger::ThreadLoop";

	Kd64::KdInitSystem();
	Kd64::KdDebuggerEnabled = TRUE;
	Kd64::KdDebuggerNotPresent = FALSE;
	Kd64::KdpDprintf("MetalOS::KdCom initialized!\n");
}

void Debugger::AddModule(KeLibrary& library)
{
	LDR_DATA_TABLE_ENTRY* entry = new LDR_DATA_TABLE_ENTRY();

	entry->DllBase = (void*)library.ImageBase;
	entry->EntryPoint = (void*)((uintptr_t)library.ImageBase + PortableExecutable::GetEntryPoint(library.ImageBase));
	entry->SizeOfImage = PortableExecutable::GetSizeOfImage(library.ImageBase);
	entry->LoadCount = 1;

	//Allocate name unicode strings
	size_t length = library.Name.length();
	size_t wideLength = (length + 1) * 2;
	wchar_t* s = new wchar_t[wideLength];
	mbstowcs(s, library.Name.c_str(), wideLength);
	s[length] = '\0';

	entry->FullDllName = { (USHORT)wcslen(s), 0, s };
	entry->BaseDllName = { (USHORT)wcslen(s), 0, s };

	InsertHeadList(&PsLoadedModuleList, &entry->InLoadOrderLinks);
}

void Debugger::DebuggerEvent(InterruptVector vector, PINTERRUPT_FRAME pFrame)
{
	KTRAP_FRAME TrapFrame = { 0 }; //Unused?
	KEXCEPTION_FRAME ExceptionFrame = { 0 }; //Unused?
	EXCEPTION_RECORD ExceptionRecord = { 0 };
	CONTEXT ContextRecord = { 0 };
	KPROCESSOR_MODE PreviousMode = KernelMode;
	BOOLEAN SecondChanceException = false;

	Assert(vector == InterruptVector::Breakpoint);
	ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
	ExceptionRecord.NumberParameters = 3;
	ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
	ExceptionRecord.ExceptionInformation[1] = (ULONG64)(LONG_PTR)kernel.m_scheduler->GetCurrentThread();
	ExceptionRecord.ExceptionInformation[2] = 0;

	ConvertToContext(pFrame, &ContextRecord);

	BOOLEAN handled = Kd64::KdpTrap(&TrapFrame, &ExceptionFrame, &ExceptionRecord, &ContextRecord, PreviousMode, SecondChanceException);
	kernel.Printf("Debugger Event: %d\n", handled);
}

void Debugger::KdpDprintf(const char* format, va_list args)
{
	Kd64::VaKdpDprintf(format, args);
}

bool Debugger::Enabled()
{
	return Kd64::KdDebuggerEnabled;
}

size_t Debugger::ThreadLoop()
{
	while (true)
	{
		if (Kd64::KdPollBreakIn())
		{
			__debugbreak();
		}
		kernel.Sleep(50);
	}

	return 0;
}

void Debugger::ConvertToContext(PINTERRUPT_FRAME frame, PCONTEXT context)
{
	context->SegCs = (WORD)frame->CS;
	context->SegFs = (WORD)frame->FS;
	context->SegGs = (WORD)frame->GS;
	context->SegSs = (WORD)frame->SS;
	context->EFlags = (DWORD)frame->RFlags;
	
	context->Rip = frame->RIP;

	context->Rax = frame->RAX;
	context->Rcx = frame->RCX;
	context->Rdx = frame->RDX;
	context->Rbx = frame->RBX;
	context->Rsp = frame->RSP;
	context->Rbp = frame->RBP;
	context->Rsi = frame->RSI;
	context->Rdi = frame->RDI;
	context->R8 = frame->R8;
	context->R9 = frame->R9;
	context->R10 = frame->R10;
	context->R11 = frame->R11;
	context->R12 = frame->R12;
	context->R13 = frame->R13;
	context->R14 = frame->R14;
	context->R15 = frame->R15;
}
