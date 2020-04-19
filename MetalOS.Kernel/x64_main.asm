.code

extern KERNEL_STACK_STOP: qword
extern main: proc

x64_main PROC
	; Setup stack
	mov rsp, KERNEL_STACK_STOP ; Stack grows from deep to shallow (high address to low address)
	sub rsp, 32 ; Register parameter area

	; Jump to main
	jmp main
x64_main ENDP

end
