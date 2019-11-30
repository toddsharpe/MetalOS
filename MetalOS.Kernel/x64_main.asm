.code

;extern KERNEL_STACK: qword
extern KERNEL_STACK_START: qword	
extern KERNEL_STACK_STOP: qword	
extern main: qword

x64_main PROC
	; Setup stack
	lea rbp, KERNEL_STACK_START
	lea rsp, KERNEL_STACK_STOP

	; Jump to main
	lea rax, main
	jmp rax
x64_main ENDP

end