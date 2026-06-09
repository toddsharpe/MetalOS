INCLUDE x64.s
INCLUDE Interrupt.s

; Defines
BootStackSize EQU (PageSize*256); 1MB

; Hooks into MetalOS
EXTERN KeMain: PROC
EXTERN KeSyscall: PROC
EXTERN KeInterrupt: PROC

; x64 Boot Stack
.data?
BootStack BYTE BootStackSize DUP (?)

.CODE

; void ArchMain();
ArchMain PROC
	; Setup stack to point to end of stack
	mov rsp, OFFSET BootStack
	add rsp, LENGTHOF BootStack

	; Register parameter area
	sub rsp, StackReserve 

	; Jump to KeMain
	jmp KeMain
ArchMain ENDP


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

; https://stackoverflow.com/questions/62546189/where-i-should-use-swapgs-instruction
; Routines to save/load user/kernel stacks don't use the stack themselves by design
x64_SYSTEMCALL PROC
	swapgs ; Swap to KThread
	PUSH_NONVOLATILE

	; Save user stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block (SelfPointer)
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+8] ; offsetof(KThread, UserThread)
	mov qword ptr [rbx+16], rsp ; offsetof(UThread, Stack)

	; Load kernel stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block (SelfPointer)
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx] ; offsetof(KThread, ContextPtr)
	mov rbx, qword ptr [rbx+64] ; Get Stack pointer (8th item)
	mov rsp, rbx ; Set stack

	; Create new frame
	PROLOG
	PUSH_SYSTEMCALL_FRAME
	
	; Save pointer for arg
	mov rcx, rsp

	sub rsp, StackReserve; Reserve 32bytes for register parameter area
	call KeSyscall; Call OS handler
	add rsp, StackReserve; Reclaim register parameter area

	; Restore frame
	POP_SYSTEMCALL_FRAME
	EPILOG

	; Save kernel stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block (SelfPointer)
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx] ; offsetof(KThread, ContextPtr)
	mov qword ptr [rbx+64], rsp ; Set stack

	; Restore stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block (SelfPointer)
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+8] ; offsetof(KThread, UserThread)
	mov rsp, qword ptr [rbx+16] ; offsetof(UThread, Stack)

	POP_NONVOLATILE
	swapgs ; Swap back to User TEB
	sysretq
x64_SYSTEMCALL ENDP

_ltr proc
	ltr cx
	ret
_ltr endp

_sti proc
	sti
	ret
_sti endp

_pause PROC
	pause
	ret
_pause ENDP

ArchEnableInterrupts PROC
	sti
	ret
ArchEnableInterrupts ENDP

ArchDisableInterrupts PROC
	pushfq
	cli
	pop rax
	ret
ArchDisableInterrupts ENDP

ArchRestoreFlags PROC
	push rcx
	popfq
	ret
ArchRestoreFlags ENDP

; void UpdateSegments(uint16_t data_selector, uint16_t code_selector);
UpdateSegments PROC
	; Populate segments with data selector
	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	mov ss, cx

	; Populate CS by using ret-far
	movzx rdx, dx ; copy code selector and extend it to 64 bits
	lea rcx, exit
	push rdx
	push rcx
	retfq
exit:
	ret
UpdateSegments ENDP

; Intel SDM Vol3A Table 6-1
x64_INTERRUPT_HANDLER 0, 0
x64_INTERRUPT_HANDLER 0, 1
x64_INTERRUPT_HANDLER 0, 2
x64_INTERRUPT_HANDLER 0, 3
x64_INTERRUPT_HANDLER 0, 4
x64_INTERRUPT_HANDLER 0, 5
x64_INTERRUPT_HANDLER 0, 6
x64_INTERRUPT_HANDLER 0, 7
x64_INTERRUPT_HANDLER 1, 8
x64_INTERRUPT_HANDLER 0, 9
x64_INTERRUPT_HANDLER 1, 10
x64_INTERRUPT_HANDLER 1, 11
x64_INTERRUPT_HANDLER 1, 12
x64_INTERRUPT_HANDLER 1, 13
x64_INTERRUPT_HANDLER 1, 14
x64_INTERRUPT_HANDLER 0, 15
x64_INTERRUPT_HANDLER 0, 16
x64_INTERRUPT_HANDLER 1, 17
x64_INTERRUPT_HANDLER 0, 18
x64_INTERRUPT_HANDLER 0, 19
x64_INTERRUPT_HANDLER 0, 20
x64_INTERRUPT_HANDLER_BLOCK 0, 21, 30
x64_INTERRUPT_HANDLER 1, 30
x64_INTERRUPT_HANDLER 0, 31
x64_INTERRUPT_HANDLER_BLOCK 0, 32, 256

END
