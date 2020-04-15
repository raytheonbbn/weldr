# Copyright (c) 2019 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration

.text
    .global fake_longjmp
    .global fake_setjmp

# Code copied directly from x86_64 libc.so.  Assume fragile as heck.

fake_longjmp:
    mov    0x30(%rdi),%r8
    mov    0x8(%rdi),%r9
    mov    0x38(%rdi),%rdx
    ror    $0x11,%r8
    xor    %fs:0x30,%r8
    ror    $0x11,%r9
    xor    %fs:0x30,%r9
    ror    $0x11,%rdx
    xor    %fs:0x30,%rdx
    mov    (%rdi),%rbx
    mov    0x10(%rdi),%r12
    mov    0x18(%rdi),%r13
    mov    0x20(%rdi),%r14
    mov    0x28(%rdi),%r15
    mov    %esi,%eax
    mov    %r8,%rsp
    mov    %r9,%rbp
    jmpq   *%rdx

fake_setjmp:
    xor    %esi,%esi
    mov    %rbx,(%rdi)
    mov    %rbp,%rax
    xor    %fs:0x30,%rax
    rol    $0x11,%rax
    mov    %rax,0x8(%rdi)
    mov    %r12,0x10(%rdi)
    mov    %r13,0x18(%rdi)
    mov    %r14,0x20(%rdi)
    mov    %r15,0x28(%rdi)
    lea    0x8(%rsp),%rdx
    xor    %fs:0x30,%rdx
    rol    $0x11,%rdx
    mov    %rdx,0x30(%rdi)
    mov    (%rsp),%rax
    xor    %fs:0x30,%rax
    rol    $0x11,%rax
    mov    %rax,0x38(%rdi)
    jmp    fake_sigjmp_save

fake_sigjmp_save:
    test   %esi,%esi
    push   %rbx
    mov    %rdi,%rbx
    je     fake_sigjmp_save+0x1d
    lea    0x48(%rdi),%rdx
    xor    %esi,%esi
    xor    %edi,%edi
    callq  sigprocmask
    xor    %esi,%esi
    test   %eax,%eax
    sete   %sil
    mov    %esi,0x40(%rbx)
    xor    %eax,%eax
    pop    %rbx
    retq   
