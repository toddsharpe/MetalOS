.code

x64_ltr proc
	ltr cx
	ret
x64_ltr endp

; CS RW
x64_ReadCS PROC
	mov ax, cs
	ret
x64_ReadCS ENDP

x64_WriteCS PROC
	mov cs, cx
	ret
x64_WriteCS ENDP

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

end
