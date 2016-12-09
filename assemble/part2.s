.global _start


.text
_start:
	movq	$1, %rax 
	movq	$1, %rdi
	movq	$sayhello, %rsi
	movq	$14, %rdx
	syscall

.data
sayhello:
	.ascii "Hello World!\n"
