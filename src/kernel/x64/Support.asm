.CODE

_pause PROC
	pause
	ret
_pause ENDP

_ltr proc
	ltr cx
	ret
_ltr endp

_sti proc
	sti
	ret
_sti endp

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

END

