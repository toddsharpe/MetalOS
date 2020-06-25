.code

x64SysCall MACRO name, id
&name& PROC
	mov r10, rcx
	mov eax, id
	syscall
	ret
&name& ENDP
ENDM

; This needs to be kept consistent with SystemCall in MetalOS.Internal.h. Figure out how
x64SysCall <GetSystemInfo>, 1
x64SysCall <GetProcessInfo>, 2
x64SysCall <ExitProcess>, 3
x64SysCall <CreateWindow>, 4
x64SysCall <WaitForMessages>, 5
x64SysCall <GetMessage>, 6
x64SysCall <CreateFile>, 7
x64SysCall <ReadFile>, 8
x64SysCall <SetFilePosition>, 9

end

