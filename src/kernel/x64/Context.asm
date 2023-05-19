INCLUDE x64.s

.CODE

; void ArchInitContext(void* context, void* const entry, void* const stack)
ArchInitContext PROC
	;Setup stack and entry
	mov [rcx + CONTEXT._rip], rdx ; Copy entry to instruction pointer
	mov [rcx + CONTEXT._rsp], r8 ; Copy stack to stack pointer

	;Setup flags with interrupts enabled (so its ready to go upon restore)
	pushfq
	pop rax
	or rax, 200h
	mov [rcx + CONTEXT._rflags], rax
	ret
ArchInitContext ENDP

; void ArchSaveContext(void* context)
ArchSaveContext PROC
	; General purpose
	mov [rcx + CONTEXT._r12], r12
	mov [rcx + CONTEXT._r13], r13
	mov [rcx + CONTEXT._r14], r14
	mov [rcx + CONTEXT._r15], r15
	mov [rcx + CONTEXT._rdi], rdi
	mov [rcx + CONTEXT._rsi], rsi
	mov [rcx + CONTEXT._rbx], rbx
	mov [rcx + CONTEXT._rbp], rbp

	; Flags
	pushfq
	pop rax
	mov [rcx + CONTEXT._rflags], rax

	; Return Address
	pop rdx
	mov [rcx + CONTEXT._rip], rdx
	mov [rcx + CONTEXT._rsp], rsp

	; Return 0 from function
	xor rax, rax
	jmp rdx
ArchSaveContext ENDP

; void ArchLoadContext(void* context);
ArchLoadContext proc
	; General purpose
	mov r12, [rcx + CONTEXT._r12]
	mov r13, [rcx + CONTEXT._r13]
	mov r14, [rcx + CONTEXT._r14]
	mov r15, [rcx + CONTEXT._r15]
	mov rdi, [rcx + CONTEXT._rdi]
	mov rsi, [rcx + CONTEXT._rsi]
	mov rbx, [rcx + CONTEXT._rbx]
	mov rbp, [rcx + CONTEXT._rbp]
	mov rsp, [rcx + CONTEXT._rsp]

	; Return 1 from function
	mov rax, 1

	;Restore flags right befor jump
	mov r8, [rcx + CONTEXT._rflags]
	mov r9, [rcx + CONTEXT._rip]
	push r8
	popfq
	jmp r9
ArchLoadContext endp

; void ArchUserThreadStart(void* context, void* teb);
ArchUserThreadStart proc
	; Load registers
	mov r12, [rcx + CONTEXT._r12]
	mov r13, [rcx + CONTEXT._r13]
	mov r14, [rcx + CONTEXT._r14]
	mov r15, [rcx + CONTEXT._r15]
	mov rdi, [rcx + CONTEXT._rdi]
	mov rsi, [rcx + CONTEXT._rsi]
	mov rbx, [rcx + CONTEXT._rbx]
	mov rbp, [rcx + CONTEXT._rbp]
	mov rsp, [rcx + CONTEXT._rsp]
	
	; RFlags
	mov r11, [rcx + CONTEXT._rflags]
	mov rcx, [rcx + CONTEXT._rip]
	swapgs
	wrgsbase rdx
	sysretq
ArchUserThreadStart endp

END
