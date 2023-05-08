INCLUDE x64.s

; Hooks into MetalOS
extern KERNEL_STACK_STOP: qword
extern main: proc

.CODE

; void ArchMain();
ArchMain PROC
	; Setup stack
	mov rsp, KERNEL_STACK_STOP ; Stack grows from high to low
	sub rsp, StackReserve ; Register parameter area

	; Jump to main
	jmp main
ArchMain ENDP

END
