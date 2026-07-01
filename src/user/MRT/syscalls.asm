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

; 0x200: Threads/Processes
x64Syscall <GetCurrentThread>, 200h
x64Syscall <CreateProcess>, 201h
x64Syscall <CreateThread>, 202h
x64Syscall <GetThreadId>, 203h
x64Syscall <Sleep>, 204h
x64Syscall <SwitchToThread>, 205h
x64Syscall <SuspendThread>, 206h
x64Syscall <ResumeThread>, 207h
x64Syscall <TerminateProcess>, 208h
x64Syscall <ExitProcess>, 209h
x64Syscall <TerminateThread>, 20Ah
x64Syscall <ExitThread>, 20Bh

; 0x300: Windowing
x64Syscall <AllocWindow>, 300h
x64Syscall <PaintWindow>, 301h
x64Syscall <MoveWindow>, 302h
x64Syscall <GetWindowRect>, 303h
x64Syscall <GetMessage>, 304h
x64Syscall <PeekMessage>, 305h
x64Syscall <GetScreenRect>, 306h

; 0x400: Files/pipes
x64Syscall <CreateFile>, 400h
x64Syscall <ReadFile>, 401h
x64Syscall <WriteFile>, 402h
x64Syscall <SetFilePointer>, 403h
x64Syscall <MoveFile>, 404h
x64Syscall <DeleteFile>, 405h
x64Syscall <CreateDirectory>, 406h
x64Syscall <CreatePipe>, 407h
x64Syscall <PeekNamedPipe>, 408h
x64Syscall <CloseHandle>, 409h

; 0x500: Syncronization
x64Syscall <WaitForSingleObject>, 500h
x64Syscall <CreateEvent>, 501h
x64Syscall <SetEvent>, 502h
x64Syscall <ResetEvent>, 503h

; 0x600: Memory
x64Syscall <VirtualAlloc>, 600h
;x64Syscall <CreateRingBuffer>, 601h
;x64Syscall <CreateSharedMemory>, 602h
;x64Syscall <MapObject>, 603h
;x64Syscall <MapSharedObject>, 604h

; 0x700: Debug
x64Syscall <DebugPrint>, 700h
x64Syscall <DebugPrintBytes>, 701h
x64Syscall <DebugPrintStack>, 702h

; 0x800: Network
; SocketRecvFrom is exposed via a C wrapper (SocketRecvFromSys) because it has >4 args.
x64Syscall <SocketCreate>, 800h
x64Syscall <SocketBind>, 801h
x64Syscall <SocketConnect>, 802h
x64Syscall <SocketSendTo>, 803h
x64Syscall <SocketRecvFromSys>, 804h
x64Syscall <SocketSend>, 805h
x64Syscall <SocketClose>, 806h
x64Syscall <GetInterfaces>, 807h
x64Syscall <GetInterfaceIp>, 808h
x64Syscall <SetInterfaceIp>, 809h
x64Syscall <SetGateway>, 80Ah

end
