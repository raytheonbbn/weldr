# Copyright (c) 2019 Raytheon BBN Technologies, Inc.  All Rights Reserved.
# This document does not contain technology or Technical Data controlled under either
# the  U.S. International Traffic in Arms Regulations or the U.S. Export Administration

.text
    .global pthread_launch

pthread_launch:
#Save off some callee-saved registers.
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

#Get me the arguments
    movq %rdi,%r12
    movq %rsi,%r13
    movq %rdx,%r14
    movq %rcx,%r15

#Save current state to buf
    movq %r13,%rdi
    movq $0,%rax
    callq setjmp

#Check setjmp's output; if not zero, we've been woken.
    test %rax,%rax
    jnz __pthread_launch_nocall

#Reset the stack, and pray very, very fast.
    movq %r12,%rsp

#Call start_routine(arg)
    movq %r12,%rdi
    movq %r13,%rsi
    movq %r14,%rdx
    movq %r15,%rcx
    call pthread_launch_helper

__pthread_launch_nocall:

#Return
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    ret
    
