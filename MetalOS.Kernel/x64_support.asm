.code

x64_ltr proc
	ltr cx
	ret
x64_ltr endp

x64_rflags proc
	pushfq
	pop rax
	ret
x64_rflags endp

x64_sti proc
	sti
	ret
x64_sti endp

x64_cli proc
	sti
	ret
x64_cli endp

; mov's into CS isn't allowed. Therefore
; https://stackoverflow.com/questions/34264752/change-gdt-and-update-cs-while-in-long-mode
; https://forum.osdev.org/viewtopic.php?f=1&t=29883&sid=b954100cc5c685faa4a7f958bad5291a&start=15
; http://os-adventure.blogspot.com/2014/12/local-gdt-and-idt.html?view=classic
; void x64_update_segments(UINT16 dataSelector - CX, UINT16 codeSelector - DX)
x64_update_segments PROC
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

	; Allocate stack space
	;sub rsp, 16
	;mov [rsp + 8], rcx
	;mov [rsp], rdx
	retf
exit:
	;pop rcx
	;pop rdx
	ret
x64_update_segments ENDP

; CS RW
x64_ReadCS PROC
	mov ax, cs
	ret
x64_ReadCS ENDP

; DS RW
x64_ReadDS PROC
	mov ax, ds
	ret
x64_ReadDS ENDP

x64_WriteDS PROC
	mov ds, cx
	ret
x64_WriteDS ENDP

; ES RW
x64_ReadES PROC
	mov ax, es
	ret
x64_ReadES ENDP

x64_WriteES PROC
	mov es, cx
	ret
x64_WriteES ENDP

; FS RW
x64_ReadFS PROC
	mov ax, fs
	ret
x64_ReadFS ENDP

x64_WriteFS PROC
	mov fs, cx
	ret
x64_WriteFS ENDP

; GS RW
x64_ReadGS PROC
	mov ax, gs
	ret
x64_ReadGS ENDP

x64_WriteGS PROC
	mov gs, cx
	ret
x64_WriteGS ENDP

; SS RW
x64_ReadSS PROC
	mov ax, ss
	ret
x64_ReadSS ENDP

x64_WriteSS PROC
	mov ss, cx
	ret
x64_WriteSS ENDP

; RSP RW
x64_ReadSP PROC
	mov rax, rsp
	ret
x64_ReadSP ENDP

; RCX R
x64_ReadRCX PROC
	mov rax, rcx
	ret
x64_ReadRCX ENDP

; RDX R
x64_ReadRDX PROC
	mov rax, rdx
	ret
x64_ReadRDX ENDP

end
