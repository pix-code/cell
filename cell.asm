.intel_syntax noprefix
.section .text
.global _start

_start:
  push 42
  pop rbx
  pop rax
  add rax, rbx
  push rax
  push 48
  mov rax, 60
  pop rdi
  syscall
