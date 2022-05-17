; Define variables in the data section
SECTION .DATA
	hello:     db 'Hello world!',10
	helloLen:  equ $-hello

; Code goes in the text section
section .text
	GLOBAL _start 

_start:
	mov rax,4            ; 'write' system call = 4
	mov rbx,1            ; file descriptor 1 = STDOUT
	mov rcx,hello        ; string to write
	mov rdx,helloLen     ; length of string to write
	int 80h              ; call the kernel
	; test imul
	mov rbx, 4
	mov rcx, 5
	imul rbx, rcx
	imul rcx, 2 
	; Terminate program
	mov rax, 60
	mov rdi, 0
	syscall
