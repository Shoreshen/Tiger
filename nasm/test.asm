; Define variables in the data section
SECTION .DATA
	hello:     db 'Hello world!',10
	helloLen:  equ $-hello

; Code goes in the text section
SECTION .TEXT
	GLOBAL _start 

_start:
    mov rax, 0
    add rax, 2             ; Add instruction test
	; mov rax,4            ; 'write' system call = 4
	; mov rbx,1            ; file descriptor 1 = STDOUT
	; mov rcx,hello        ; string to write
	; mov rdx,helloLen     ; length of string to write
	; int 80h              ; call the kernel

	; ; Terminate program
	; mov rax,1            ; 'exit' system call
	; mov rbx,0            ; exit with error code 0
	; int 80h              ; call the kernel
