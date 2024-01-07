#pragma once
#include <stdint.h>

//Shared header between loader and kernel

//User space starts at   0x00000000 00000000
//User space stops at    0x00007FFF FFFFFFFF
enum UserAddress : uintptr_t
{
	UserStart = 0x0,
	UserStop = 0x0000800000000000,
};

//Kernel space starts at 0xFFFF8000 00000000
//Kernel space stops at  0xFFFFFFFF FFFFFFFF
static constexpr size_t KernelSectionLength = 0x10'0000'0000; //64GB
enum KernelAddress : uintptr_t
{
	KernelStart = 0xFFFF'8000'0000'0000,

	//Kernel libraries 0xFFFF'8000'0000'0000 (256MB)
	KernelLibraryStart     = KernelStart,
	KernelBaseAddress      = 0xFFFF'8000'0100'0000,
	//KernelKdComBaseAddress = 0xFFFF'8000'0200'0000,
	KernelLibraryEnd       = 0xFFFF'8000'1000'0000,

	//Kernel pdbs 0xFFFF'8000'1000'0000 (256MB)
	KernelPdbStart  = KernelLibraryEnd,
	KernelKernelPdb = 0xFFFF'8000'1100'0000,
	//KernelKdComPdb  = 0xFFFF'8000'1200'0000,
	KernelPdbEnd    = 0xFFFF'8000'2000'0000,

	//Kernel stacks 0xFFFF'8000'2000'0000 (256MB)
	KernelStackStart = KernelPdbEnd,
	KernelStackEnd   = 0xFFFF'8000'3000'0000,

	//Kernel runtime 0xFFFF'8000'3000'0000 (256MB) TODO: condense with IO?
	KernelRuntimeStart = KernelStackEnd,
	KernelRuntimeEnd = 0xFFFF'8000'4000'0000,

	//Window frames 0xFFFF'8000'4000'0000 (256MB)
	KernelWindowsStart = KernelRuntimeEnd,
	KernelWindowsEnd = 0xFFFF'8000'5000'0000,

	//Hardware 0xFFFF'8010'0000'0000
	KernelHardwareStart = KernelStart + KernelSectionLength,
	KernelPageTablesPool = KernelHardwareStart, //32MB (512 pages used)
	KernelGraphicsDevice = KernelHardwareStart + 0x200'0000, //32MB
	KernelRamDrive = KernelHardwareStart + 0x400'0000, //64MB
	KernelHardwareEnd = KernelHardwareStart + 0x800'0000,
	KernelPageFrameDBStart = KernelHardwareStart + 0x1'0000'0000,//4GB

	//Heap 0xFFFF'8020'0000'0000
	KernelHeapStart = KernelHardwareStart + KernelSectionLength,
	KernelHeapEnd = KernelHeapStart + KernelSectionLength,

	//UEFI 0xFFFF'8030'0000'0000
	KernelUefiStart = KernelHeapEnd,
	KernelUefiEnd = KernelUefiStart + KernelSectionLength,

	//ACPI 0xFFFF'8040'0000'0000
	KernelAcpiStart = KernelUefiEnd,
	KernelAcpiEnd = KernelAcpiStart + KernelSectionLength,

	//IO 0xFFFF'8050'0000'0000 (physical page)
	KernelIoStart = KernelAcpiEnd,
	KernelIoEnd = KernelIoStart + KernelSectionLength,

	//Windows
	KernelSharedPageStart = 0xFFFF'F780'0000'0000,
	KernelSharedPageStop  = 0xFFFF'F780'0000'1000,

	KernelEnd = 0xFFFF'FFFF'FFFF'FFFF
};
static constexpr size_t RamDriveSize = KernelHardwareEnd - KernelRamDrive;

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PageSize))

//TODO(tsharpe): Move in kernel only header once x64 library is integrated
#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))

//Kernel defines
typedef size_t cpu_flags_t;

//Keep in sync with MetalOS.KernalApi syscalls.asm
enum class SystemCall : size_t
{
	GetTickCount = 0x100,
	GetSystemTime,

	GetCurrentThread = 0x200,
	CreateProcess,
	CreateThread,
	GetThreadId,
	Sleep,
	SwitchToThread,
	SuspendThread,
	ResumeThread,
	ExitProcess,
	ExitThread,

	AllocWindow = 0x300,
	PaintWindow,
	MoveWindow,
	GetWindowRect,
	GetMessage,
	PeekMessage,
	GetScreenRect,

	CreateFile = 0x400,
	CreatePipe,
	ReadFile,
	WriteFile,
	SetFilePointer,
	CloseFile,
	MoveFile,
	DeleteFile,
	CreateDirectory,
	WaitForSingleObject,
	GetPipeInfo,
	CloseHandle,
	CreateEvent,
	ResetEvent,
	SetEvent,

	VirtualAlloc = 0x500,
	CreateRingBuffer,
	CreateSharedMemory,
	MapObject,
	MapSharedObject,

	DebugPrint = 0x600,
	DebugPrintBytes = 0x601,
};
