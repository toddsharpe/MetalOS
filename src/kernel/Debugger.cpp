#include "Assert.h"


#include <windows/types.h>
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/amd64/ke.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include "kddll.h"
#include "kernel/Kd64/kd64.h"
extern "C" KdDll m_dll;
#include "Debugger.h"
#include "Loader.h"
#include "WinPE.h"
#include "MetalOS.Space.h"

uint32_t Debugger::ThreadLoop(void* arg)
{
	return static_cast<Debugger*>(arg)->ThreadLoop();
};

Debugger::Debugger()
{
	/* Fill out the KD Version Block */
	KdVersionBlock.MajorVersion = DBGKD_MAJOR_EFI;
	KdVersionBlock.MinorVersion = 7601;
	
	/* Set protocol limits */
	KdVersionBlock.MaxStateChange = DbgKdMaximumStateChange -
		DbgKdMinimumStateChange;
	KdVersionBlock.MaxManipulate = DbgKdMaximumManipulate -
		DbgKdMinimumManipulate;
	KdVersionBlock.Unused[0] = 0;

	KdVersionBlock.KernBase = KernelBase;
	KdVersionBlock.PsLoadedModuleList = (ULONG64)(LONG_PTR)&PsLoadedModuleList;

	InitializeListHead(&PsLoadedModuleList);
}

void Debugger::Initialize()
{
	const KModule* kddll = KeLoadLibrary("kdcom.dll");

	//Load pointers
	m_dll.KdInitialize = static_cast<OnKdInitialize>(WinPE::GetProcAddress(kddll->ImageBase, "KdInitialize"));
	m_dll.KdReceivePacket = static_cast<OnKdReceivePacket>(WinPE::GetProcAddress(kddll->ImageBase, "KdReceivePacket"));
	m_dll.KdSendPacket = static_cast<OnKdSendPacket>(WinPE::GetProcAddress(kddll->ImageBase, "KdSendPacket"));

	Printf("KdInitialize loaded at 0x%016x\n", m_dll.KdInitialize);
	Printf("KdReceivePacket loaded at 0x%016x\n", m_dll.KdReceivePacket);
	Printf("KdSendPacket loaded at 0x%016x\n", m_dll.KdSendPacket);

	NTSTATUS result = m_dll.KdInitialize(2);
	Printf("KdInitialized %d\n", result);

	KeCreateThread(Debugger::ThreadLoop, this, "Debugger::ThreadLoop");

	KdInitSystem();
	KdDebuggerEnabled = TRUE;
	KdDebuggerNotPresent = FALSE;
}

void Debugger::AddModule(const KModule& library)
{
	LDR_DATA_TABLE_ENTRY* entry = m_arena.Allocate<LDR_DATA_TABLE_ENTRY>();

	entry->DllBase = (void*)library.ImageBase;
	entry->EntryPoint = (void*)((uintptr_t)library.ImageBase + WinPE::GetEntryPoint(library.ImageBase));
	entry->SizeOfImage = WinPE::GetSizeOfImage(library.ImageBase);
	entry->LoadCount = 1;

	//Allocate name unicode strings
	size_t length = library.Name.Count();
	size_t wideLength = (length + 1) * 2;
	wchar_t* s = (wchar_t*)m_arena.Allocate(sizeof(wchar_t) * wideLength);
	mbstowcs(s, library.Name.c_str(), wideLength);
	s[length] = '\0';

	entry->FullDllName = { (USHORT)wcslen(s), 0, s };
	entry->BaseDllName = { (USHORT)wcslen(s), 0, s };

	InsertHeadList(&PsLoadedModuleList, &entry->InLoadOrderLinks);
}

void Debugger::DebuggerEvent(Arch::InterruptVector vector, Arch::InterruptFrame& frame)
{
	KTRAP_FRAME TrapFrame = { 0 }; //Unused?
	KEXCEPTION_FRAME ExceptionFrame = { 0 }; //Unused?
	EXCEPTION_RECORD ExceptionRecord = { 0 };
	CONTEXT ContextRecord = { 0 };
	KPROCESSOR_MODE PreviousMode = KernelMode;
	BOOLEAN SecondChanceException = false;

	ExceptionRecord.ExceptionCode = STATUS_BREAKPOINT;
	ExceptionRecord.NumberParameters = 3;
	ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
	ExceptionRecord.ExceptionInformation[1] = (ULONG64)(LONG_PTR)&Scheduler::GetThread();
	ExceptionRecord.ExceptionInformation[2] = 0;

	ConvertToContext(&frame, &ContextRecord);

	BOOLEAN handled = KdpTrap(&TrapFrame, &ExceptionFrame, &ExceptionRecord, &ContextRecord, PreviousMode, SecondChanceException);
	Printf("Debugger Event: %d\n", handled);
}

void Debugger::KdpDprintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	KdpDprintf(format, args);

	va_end(args);
}

void Debugger::KdpDprintf(const char* format, va_list args)
{
	VaKdpDprintf(format, args);
}

bool Debugger::Enabled()
{
	return KdDebuggerEnabled;
}

uint32_t Debugger::ThreadLoop()
{
	while (true)
	{
		if (KdPollBreakIn())
		{
			__debugbreak();
		}
		Sleep(50);
	}

	return 0;
}

void Debugger::ConvertToContext(Arch::InterruptFrame* frame, PCONTEXT context)
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
