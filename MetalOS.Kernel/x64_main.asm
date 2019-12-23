.code

extern KERNEL_STACK_STOP: qword
extern main_thunk: proc

x64_main PROC
	; Setup stack
	mov rsp, KERNEL_STACK_STOP ; Stack grows from deep to shallow (high address to low address)

	; Jump to main
	jmp main_thunk
x64_main ENDP

end
