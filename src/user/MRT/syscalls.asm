.code

x64SysCall MACRO name, id
&name& PROC
	mov r10, rcx
	mov rax, id
	syscall
	ret
&name& ENDP
ENDM

; This needs to be kept consistent with SystemCall in MetalOS.Internal.h. Figure out how
x64SysCall <GetTickCount>, 100h
x64SysCall <GetSystemTime>, 102h

x64SysCall <GetCurrentThread>, 200h
x64SysCall <CreateProcess>, 201h
x64SysCall <CreateThread>, 202h
x64SysCall <GetThreadId>, 203h
x64SysCall <Sleep>, 204h
x64SysCall <SwitchToThread>, 205h
x64SysCall <SuspendThread>, 206h
x64SysCall <ResumeThread>, 207h
x64SysCall <ExitProcess>, 208h
x64SysCall <ExitThread>, 209h

x64SysCall <AllocWindow>, 300h
x64SysCall <PaintWindow>, 301h
x64SysCall <MoveWindow>, 302h
x64SysCall <GetWindowRect>, 303h
x64SysCall <GetMessage>, 304h
x64SysCall <PeekMessage>, 305h
x64SysCall <GetScreenRect>, 306h

x64SysCall <CreateFile>, 400h
x64SysCall <CreatePipe>, 401h
x64SysCall <ReadFile>, 402h
x64SysCall <WriteFile>, 403h
x64SysCall <SetFilePointer>, 404h
x64SysCall <CloseFile>, 405h
x64SysCall <MoveFile>, 406h
x64SysCall <DeleteFile>, 407h
x64SysCall <CreateDirectory>, 408h
x64SysCall <WaitForSingleObject>, 409h
x64SysCall <GetPipeInfo>, 40Ah
x64SysCall <CloseHandle>, 40Bh
x64SysCall <CreateEvent>, 40Ch
x64SysCall <ResetEvent>, 40Dh
x64SysCall <SetEvent>, 40Eh

x64SysCall <VirtualAlloc>, 500h
x64SysCall <CreateRingBuffer>, 501h
x64SysCall <CreateSharedMemory>, 502h
x64SysCall <MapObject>, 503h
x64SysCall <MapSharedObject>, 504h

x64SysCall <DebugPrint>, 600h
x64SysCall <DebugPrintBytes>, 601h

end
