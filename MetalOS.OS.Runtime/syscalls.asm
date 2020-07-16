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
x64SysCall <GetSystemInfo>, 1
; GetProcessInfo (usermode)
x64SysCall <Sleep>, 3
x64SysCall <ExitProcess>, 4
x64SysCall <ExitThread>, 5
x64SysCall <CreateWindow>, 6
x64SysCall <GetWindowRect>, 7
x64SysCall <GetMessage>, 8
x64SysCall <PeekMessage>, 9
x64SysCall <SetScreenBuffer>, 10
x64SysCall <CreateFile>, 11
x64SysCall <ReadFile>, 12
x64SysCall <SetFilePosition>, 13
x64SysCall <VirtualAlloc>, 14
x64SysCall <DebugPrint>, 15

end

