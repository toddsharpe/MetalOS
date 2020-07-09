EXTERN SYSTEMCALL_HANDLER: proc

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

.code

x64_SYSTEMCALL PROC
	; Push context
	push RSP

	; Push frame
	PROLOG
	PUSH_SYSTEMCALL_FRAME
	
	; Save pointer for arg
	mov rcx, rsp

	sub rsp, 20h; Reserve 32bytes for register parameter area
	call SYSTEMCALL_HANDLER; Call OS handler
	add rsp, 20h; Reclaim register parameter area

	; Restore frame
	POP_SYSTEMCALL_FRAME
	EPILOG

	; Restore context
	pop RSP
	sysretq
x64_SYSTEMCALL ENDP

END
