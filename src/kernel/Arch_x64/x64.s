.DATA

; https://docs.microsoft.com/en-us/cpp/build/x64-software-conventions?view=vs-2019
; Keep in sync with X64_CONTEXT
Context STRUCT
	_r12 QWORD ?
	_r13 QWORD ?
	_r14 QWORD ?
	_r15 QWORD ?
	_rdi QWORD ?
	_rsi QWORD ?
	_rbx QWORD ?
	_rbp QWORD ?
	_rsp QWORD ?
	_rip QWORD ?
	_rflags QWORD ?
Context ENDS

StackReserve QWORD 20h ; Register Parameter Area
PageSize EQU 1000h ; 4k pages

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
