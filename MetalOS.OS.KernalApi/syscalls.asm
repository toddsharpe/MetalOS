.code

x64SysCall MACRO name, id
&name& PROC
	mov r10, rcx
	mov eax, id
	syscall
	ret
&name& ENDP
ENDM

; This needs to be kept consistent with SystemCall in MetalOS.h. Figure out how
x64SysCall <GetSystemInfo>, 1
x64SysCall <GetProcessInfo>, 2

end