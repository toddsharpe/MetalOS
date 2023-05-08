INCLUDE x64.s

; Hooks into MetalOS
EXTERN SYSTEMCALL_HANDLER: proc

.CODE

PROLOG MACRO
	push rbp
	mov rbp, rsp
ENDM

EPILOG MACRO
	pop rbp
ENDM

PUSH_SYSTEMCALL_FRAME MACRO
	push R9 ; Arg3
	push R8 ; Arg2
	push RDX ; Arg1
	push R10 ; Arg0
	push R11 ; Flags
	push RCX ; User IP to return to
	push RAX ; Systemcall number
ENDM

POP_SYSTEMCALL_FRAME MACRO
	add RSP, 8 ; Pop RAX without saving (that would clobber handler's return result)
	pop RCX
	pop R11
	pop R10
	pop RDX
	pop R8
	pop R9
ENDM

PUSH_NONVOLATILE MACRO
	push rbx
ENDM

POP_NONVOLATILE MACRO
	pop rbx
ENDM

; https://stackoverflow.com/questions/62546189/where-i-should-use-swapgs-instruction
; Routines to save/load user/kernel stacks don't use the stack themselves by design
x64_SYSTEMCALL PROC
	swapgs ; Swap to KThread
	PUSH_NONVOLATILE

	; Save user stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+16] ; offsetof(KThread, UserThread)
	mov qword ptr [rbx], rsp ; offsetof(UserThread, Stack)

	; Load kernel stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+8] ; offsetof(KThread, Context)
	mov rbx, qword ptr [rbx+64] ; Get Stack pointer (8th item)
	mov rsp, rbx ; Set stack

	; Create new frame
	PROLOG
	PUSH_SYSTEMCALL_FRAME
	
	; Save pointer for arg
	mov rcx, rsp

	sub rsp, StackReserve; Reserve 32bytes for register parameter area
	call SYSTEMCALL_HANDLER; Call OS handler
	add rsp, StackReserve; Reclaim register parameter area

	; Restore frame
	POP_SYSTEMCALL_FRAME
	EPILOG

	; Save kernel stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+8] ; offsetof(KThread, Context)
	mov qword ptr [rbx+64], rsp ; Set stack

	; Restore stack
	mov rbx, qword ptr gs:[0] ; Get CpuContext block
	mov rbx, qword ptr [rbx+8] ; offsetof(CpuContext, Thread)
	mov rbx, qword ptr [rbx+16] ; offsetof(KThread, UserThread)
	mov rsp, qword ptr [rbx] ; offsetof(UserThread, Stack)

	POP_NONVOLATILE
	swapgs ; Swap back to UserThread
	sysretq
x64_SYSTEMCALL ENDP

END
