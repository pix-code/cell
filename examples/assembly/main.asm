.intel_syntax noprefix

.section .data
    msg:
        .ascii "Hello, World!\n"
    msg_len = . - msg

.section .text
    .globl _start

_start:
    mov rax, 1          # sys_write
    mov rdi, 1          # stdout
    lea rsi, [msg]      # Load address of msg
    mov rdx, msg_len    # length
    syscall

    mov rax, 60         # sys_exit
    xor rdi, rdi        # status 0
    syscall
