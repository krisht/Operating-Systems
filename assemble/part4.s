.global _start

.text
_start:
    movq $123, %rax
    movq $sayhello, %rbx
    syscall

    movq $60, %rax
    movq $0, %rdi
    syscall

.data
sayhello:
    .ascii "Hello world!\n"
