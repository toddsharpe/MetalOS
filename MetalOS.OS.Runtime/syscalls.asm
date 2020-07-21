.code

x64SysCall MACRO name, id
&name& PROC
	mov r10, rcx
	mov rax, id
	syscall
	ret
&name& ENDP
ENDM

; This needs to be kept consistent with SystemCall in MetalOS.Internal.h. Figure out how
x64SysCall <GetSystemInfo>, 1
; GetProcessInfo (usermode), 2
x64SysCall <GetTickCount>, 3

x64SysCall <Sleep>, 4
x64SysCall <ExitProcess>, 5
x64SysCall <ExitThread>, 6

x64SysCall <CreateWindow>, 7
x64SysCall <GetWindowRect>, 8
x64SysCall <GetMessage>, 9
x64SysCall <PeekMessage>, 10
x64SysCall <SetScreenBuffer>, 11

x64SysCall <CreateFile>, 12
x64SysCall <ReadFile>, 13
x64SysCall <WriteFile>, 14
x64SysCall <SetFilePointer>, 15
x64SysCall <CloseFile>, 16
x64SysCall <MoveFile>, 17
x64SysCall <DeleteFile>, 18
x64SysCall <CreateDirectory>, 19

x64SysCall <VirtualAlloc>, 20

x64SysCall <DebugPrint>, 21

; LoadLibrary (usermode), 22
; GetProcAddress (usermode), 23
end


