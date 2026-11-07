.code

x64Syscall MACRO name, id
&name& PROC
	mov r10, rcx
	mov rax, id
	syscall
	ret
&name& ENDP
ENDM

;
; Keep in sync with MetalOS.Syscalls.h
;

; 0x100: System
x64Syscall <GetTickCount>, 100h
x64Syscall <GetSystemTime>, 101h

; 0x200: Processes
x64Syscall <CreateProcess>, 200h
x64Syscall <TerminateProcess>, 201h
x64Syscall <ExitProcess>, 202h
x64Syscall <IsProcessAlive>, 203h
x64Syscall <GetProcesses>, 204h

; 0x300: Threads
x64Syscall <GetCurrentThread>, 300h
x64Syscall <CreateThread>, 301h
x64Syscall <GetThreadId>, 302h
x64Syscall <Sleep>, 303h
x64Syscall <SwitchToThread>, 304h
x64Syscall <TerminateThread>, 305h
x64Syscall <ExitThread>, 306h

; 0x400: Files and pipes
x64Syscall <CreateFile>, 400h
x64Syscall <ReadFile>, 401h
x64Syscall <WriteFile>, 402h
x64Syscall <SetFilePointer>, 403h
x64Syscall <MoveFile>, 404h
x64Syscall <DeleteFile>, 405h
x64Syscall <CreateDirectory>, 406h
x64Syscall <CreatePipe>, 407h
x64Syscall <PeekNamedPipe>, 408h

; 0x500: Handles (closing, and passing one to another process)
x64Syscall <CloseHandle>, 500h
x64Syscall <ShareHandle>, 501h
x64Syscall <ClaimHandle>, 502h

; 0x600: Synchronization
x64Syscall <WaitForSingleObject>, 600h
x64Syscall <CreateEvent>, 601h
x64Syscall <SetEvent>, 602h
x64Syscall <ResetEvent>, 603h

; 0x700: Virtual memory
x64Syscall <VirtualAlloc>, 700h
x64Syscall <VirtualFree>, 701h

; 0x800: Shared memory
x64Syscall <CreateSharedMemory>, 800h
x64Syscall <MapSharedMemory>, 801h
x64Syscall <DeleteSharedMemory>, 802h

; 0x900: Display
x64Syscall <MapFramebuffer>, 900h
x64Syscall <FlushFramebuffer>, 901h

; 0xA00: Network interfaces (TCP/IP is the usermode netstack)
x64Syscall <NetSend>, 0A00h
x64Syscall <GetInterfaces>, 0A01h

; 0xB00: IPC endpoints (name to capability token rendezvous)
x64Syscall <RegisterEndpoint>, 0B00h
x64Syscall <LookupEndpoint>, 0B01h
x64Syscall <PostEndpoint>, 0B02h
x64Syscall <PollEndpoint>, 0B03h

; 0xC00: Debug
x64Syscall <DebugPrint>, 0C00h
x64Syscall <DebugPrintBytes>, 0C01h
x64Syscall <DebugPrintStack>, 0C02h

end
