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
x64SysCall <GetSystemInfo>, 100h
x64SysCall <GetTickCount>, 101h
x64SysCall <GetSystemTime>, 101h

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
x64SysCall <ReadFile>, 401h
x64SysCall <WriteFile>, 402h
x64SysCall <SetFilePointer>, 403h
x64SysCall <CloseFile>, 404h
x64SysCall <MoveFile>, 405h
x64SysCall <DeleteFile>, 406h
x64SysCall <CreateDirectory>, 407h

x64SysCall <VirtualAlloc>, 500h
x64SysCall <CreateRingBuffer>, 501h
x64SysCall <CreateSharedMemory>, 502h
x64SysCall <MapObject>, 503h
x64SysCall <MapSharedObject>, 504h

x64SysCall <DebugPrint>, 600h

end
