extern print

segment .note.GNU-stack

segment .text
	global main 

main:
	push rbp
	mov rdi, hello
	call print
	mov rdi, exit
	call print
	pop rbp
	mov rax, 0
	ret

segment .data
	hello: db 'Hello world!',10,0
	exit: db 'bye',10,0