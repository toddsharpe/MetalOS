#pragma once

//Keep in sync with MetalOS.KernalApi syscalls.asm
enum class Syscall : uint64_t
{
	//0x100: System
	GetTickCount = 0x100,
	GetSystemTime,

	//0x200: Threads/Processes
	GetCurrentThread = 0x200,
	CreateProcess,
	CreateThread,
	GetThreadId,
	Sleep,
	SwitchToThread,
	SuspendThread,
	ResumeThread,
	TerminateProcess,
	ExitProcess,
	TerminateThread,
	ExitThread,
	IsProcessAlive,
	GetProcesses,

	//0x300: Devices (framebuffer + network interface enumeration).
	//IPv4 config (Get/SetInterfaceIp, SetGateway) is netstack IPC, not syscalls.
	MapFramebuffer = 0x300,
	GetInterfaces,
	NetSend,

	//0x400: Files/pipes
	CreateFile = 0x400,
	ReadFile,
	WriteFile,
	SetFilePointer,
	MoveFile,
	DeleteFile,
	CreateDirectory,
	CreatePipe,
	PeekNamedPipe,
	CloseHandle,

	//0x500: Syncronization
	WaitForSingleObject = 0x500,
	CreateEvent,
	SetEvent,
	ResetEvent,

	//0x600: Memory
	VirtualAlloc = 0x600,
	VirtualFree,
	CreateSharedMemory,
	MapSharedMemory,

	//0x700: Debug
	DebugPrint = 0x700,
	DebugPrintBytes,
	DebugPrintStack,

	//0x900: IPC endpoints (name -> capability-token rendezvous)
	RegisterEndpoint = 0x900,
	LookupEndpoint,
	PostEndpoint,   //0x902
	PollEndpoint,   //0x903

	//0xA00: Handle capabilities (pass a handle to another process)
	ShareHandle = 0xA00,
	ClaimHandle,
};
