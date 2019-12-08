; x64 doesn't all for inline assembly and naked functions, so all ISRs need to be implemented in asm

EXTERN INTERRUPT_HANDLER: proc
EXTERN EXCEPTION_HANDLER: proc

ISR_PROLOG MACRO
	push rbp
	mov rbp, rsp
ENDM

ISR_EPILOG MACRO
	pop rbp
ENDM

PUSH_INTERRUPT_FRAME MACRO
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
	push rdx
	push rcx
	push rbx
	push rax
ENDM

POP_INTERRUPT_FRAME MACRO
	pop rax
	pop rbx
	pop rcx
	pop rdx
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
	pop rbp
ENDM

; TODO: i think these routines need to swap code/data selectors to kernel ones?

x64_INTERRUPT_HANDLER MACRO number, name
&name&_ISR_HANDLER PROC
	;ISR_PROLOG
	;PUSH_INTERRUPT_FRAME
	;push number
	;mov rcx, rsp ; set up argument for ISR_HANDLER
	;call INTERRUPT_HANDLER
	mov RAX, INTERRUPT_HANDLER
	call RAX
	;add rsp, 8 ; clean up stack from push of number
	;POP_INTERRUPT_FRAME
	;ISR_EPILOG
	iretq
&name&_ISR_HANDLER ENDP
ENDM

x64_EXCEPTION_HANDLER MACRO number, name
&name&_EXC_HANDLER PROC
	hlt
	ISR_PROLOG
	;PUSH_INTERRUPT_FRAME
	;push number
	;mov rcx, rsp ; set up argument for ISR_HANDLER
	;call EXCEPTION_HANDLER
	mov RAX, EXCEPTION_HANDLER
	call RAX
	;add rsp, 8 ; clean up stack from push of number
	;POP_INTERRUPT_FRAME
	ISR_EPILOG
&name&_EXC_HANDLER ENDP
ENDM

.code

; Intel SDM Vol3A Table 6-1
x64_INTERRUPT_HANDLER 0, <DE>
x64_INTERRUPT_HANDLER 1, <DB>
x64_INTERRUPT_HANDLER 2, <NMI>
x64_INTERRUPT_HANDLER 3, <BP>
x64_INTERRUPT_HANDLER 4, <OF>
x64_INTERRUPT_HANDLER 5, <BR>
x64_INTERRUPT_HANDLER 6, <UD>
x64_INTERRUPT_HANDLER 7, <NM>
x64_EXCEPTION_HANDLER 8, <DF>
x64_INTERRUPT_HANDLER 9, <CSO>
x64_EXCEPTION_HANDLER 10, <TS>
x64_EXCEPTION_HANDLER 11, <NP>
x64_EXCEPTION_HANDLER 12, <SS>
x64_EXCEPTION_HANDLER 13, <GP>
x64_EXCEPTION_HANDLER 14, <PF>
x64_INTERRUPT_HANDLER 16, <MF>
x64_EXCEPTION_HANDLER 17, <AC>
x64_INTERRUPT_HANDLER 18, <MC>
x64_INTERRUPT_HANDLER 19, <XM>
x64_INTERRUPT_HANDLER 20, <VE>
x64_EXCEPTION_HANDLER 30, <SX>
end
