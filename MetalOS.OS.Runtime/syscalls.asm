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

x64SysCall <GetCurrentThread>, 200h
x64SysCall <CreateThread>, 201h
x64SysCall <Sleep>, 202h
x64SysCall <SwitchToThread>, 203h
x64SysCall <SuspendThread>, 204h
x64SysCall <ResumeThread>, 205h
x64SysCall <ExitProcess>, 206h
x64SysCall <ExitThread>, 207h

x64SysCall <CreateWindow>, 300h
x64SysCall <GetWindowRect>, 301h
x64SysCall <GetMessage>, 302h
x64SysCall <PeekMessage>, 303h
x64SysCall <SetScreenBuffer>, 304h

x64SysCall <CreateFile>, 400h
x64SysCall <ReadFile>, 401h
x64SysCall <WriteFile>, 402h
x64SysCall <SetFilePointer>, 403h
x64SysCall <CloseFile>, 404h
x64SysCall <MoveFile>, 405h
x64SysCall <DeleteFile>, 406h
x64SysCall <CreateDirectory>, 407h

x64SysCall <VirtualAlloc>, 500h

x64SysCall <DebugPrint>, 600h

end
