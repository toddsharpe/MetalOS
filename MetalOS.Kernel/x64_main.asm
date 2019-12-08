.code

;extern KERNEL_STACK: qword
extern KERNEL_STACK_START: qword	
extern KERNEL_STACK_STOP: qword	
extern main: qword

x64_main PROC
	; Setup stack
	mov rbp, KERNEL_STACK_STOP ; This doesn't seem totally right, but main never returns anyways
	mov rsp, KERNEL_STACK_STOP ; Stack grows from deep to shallow (high address to low address)

	; Jump to main
	lea rax, main
	jmp rax
x64_main ENDP

end
