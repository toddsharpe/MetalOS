INCLUDE x64.s

;Interrupt handler
EXTERN INTERRUPT_HANDLER: proc

ISR_PROLOG MACRO
	push rbp
	mov rbp, rsp
ENDM

ISR_EPILOG MACRO
	pop rbp
ENDM

; Pushed in the opposite order as INTERRUPT_FRAME since stack grows shallow in x64
PUSH_INTERRUPT_FRAME MACRO
	push GS
	push FS
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rbx
	push rdx
	push rcx
	push rax
ENDM

; Popped in opposite order as PUSH_INTERRUPT_FRAME, same order as INTERRUPT_FRAME
POP_INTERRUPT_FRAME MACRO
	pop rax
	pop rcx
	pop rdx
	pop rbx
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	add rsp, 16; Pop GS/FS
ENDM

x64_INTERRUPT_HANDLER MACRO hasCode, number
x64_interrupt_handler_&number& PROC
	; Disable interrupts
	cli

	; Push dummy error code if interrupt doesn't already
	IF hasCode EQ 0
	push 0
	ENDIF

	; Compare CS to 8 (KernelCode), jumping over swapgs if equal
	cmp qword ptr [rsp+10h], 8h
	je noswap_start
	swapgs

noswap_start:
	; Push Complete frame
	ISR_PROLOG
	PUSH_INTERRUPT_FRAME

	; INTERRUPT_HANDLER arguments
	mov rcx, number
	mov rdx, rsp

	sub rsp, StackReserve; Reserve 32bytes for register parameter area
	call INTERRUPT_HANDLER; Call OS handler
	add rsp, StackReserve; Reclaim register parameter area

	; Restore previous frame
	POP_INTERRUPT_FRAME
	ISR_EPILOG

	; Compare CS to 8 (KernelCode), jumping over swapgs if equal
	cmp qword ptr [rsp+10h], 8h
	je noswap_stop
	swapgs

noswap_stop:
	; Remove error code (Intel SDM, Vol 3A Section 6.13)
	add rsp, 8

	sti
	iretq
x64_interrupt_handler_&number& ENDP
ENDM

x64_INTERRUPT_HANDLER_BLOCK MACRO hasCode:REQ, startNumber:REQ, stopNumber:REQ
count = startNumber
WHILE count LT stopNumber
	x64_INTERRUPT_HANDLER %hasCode, %count
	count = count + 1
ENDM
ENDM

.code

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
