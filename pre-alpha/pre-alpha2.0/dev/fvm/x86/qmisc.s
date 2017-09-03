/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               This is a definition of the virtual machine.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170826
Updated:    20170903+
Version:    pre-alpha-0.0.0.29+ for FVM 2.0

                              This Edition:
                     x86 Assembly Language for Linux
                           using GNU Assembler

                               ( ) [ ] { }

TODO NEXT:

  1. Consider if 31-bit immediates is a smell and just make them 32.
  2. Consider I/O.
  3. Then implement for ARM (important POC).
  4. Then fix self-virtualization.

==============================================================================
                            BUILDING FOR i386
==============================================================================

You may need gcc-multilib installed.

When linking with gcc for debugging, build with:

  as -g --gstabs -o qmisc.o qmisc.s --32
  gcc -static -o qmisc qmisc.o -m32

When linking with gcc for release, build with:

  as -o qmisc.o qmisc.s --32
  gcc -static -o qmisc qmisc.o -m32

When LINKING_WITH_LD_ON_LINUX without any imports, build with:

  as -o qmisc.o qmisc.s --32
  ld -o qmisc qmisc.o -m elf_i386

Or for convenience, build with:

  ./build.sh qmisc

Or for convenience, build and run with:

  ./go.sh qmisc

This initial implementation is mainly for i386 but you can modify
the above commands appropriately (by removing --32, -m32, -m elf_i386)
and set the build flag x86_64 to YES to build for x86-64.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
# ============================================================================
#                              BUILD FLAGS
# ============================================================================
.equ YES, 0
.equ NO, 1
.equ TRACING_ENABLED, YES
.equ LINKING_WITH_LD_ON_LINUX, NO
.equ x86_64, NO
.equ NO_PROGRAM, NO
# ============================================================================
#                               CONSTANTS
# ============================================================================
.equ WD_BYTES, 4
.equ ONES,          0xffffffff
.equ MSb,           0x80000000  # Bit mask for most significant bit
.equ METADATA_MASK, 0x7fffffff  # 31 bits
.equ BYTE_MASK,     0x000000ff
.equ SHIFT_MASK,    0x0000001f
.equ SUCCESS, 0
.equ FAILURE, 1
.equ ILLEGAL, 2
.equ MAX_DM_WORDS,  0x10000000  # Must be 2^(WD_BITS-4) due to C limitations.
.equ DM_WORDS, MAX_DM_WORDS     # Must be some power of 2 <= MAX_DM_WORDS.
.equ DM_BYTES, DM_WORDS*WD_BYTES
.equ DM_MASK, DM_WORDS-1
# ============================================================================
#                           ACCESSIBLE REGISTERS
#     These are the accessible registers of the QMISC virtual machine.
# ============================================================================
.equ vA, %eax; # accumulator
.equ vB, %ecx; # operand register (here using ecx to simplify shl, shr)
.equ vT, %edx; # temporary register
.equ vR, %esi; # repeat register
# ============================================================================
#                          INACCESSIBLE REGISTERS
#     These are the inaccessible registers of the QMISC virtual machine.
# ============================================================================
.ifeq x86_64  # link register
  .equ vL, %rdi;
.else
  .equ vL, %edi;
.endif
.equ vD, %ebx; # address register
# ============================================================================
#                         IMPLEMENTATION REGISTERS
#     These are registers internal to this particular implementation.
#     These are not part of the QMISC virtual machine itself.
# ============================================================================
.equ rShift, %cl; # shift register (see note at vB above)
# ============================================================================
#                            INSTRUCTION SET
# ============================================================================
.macro i x
  movl $\x, vB
  andl $METADATA_MASK, vB
.endm
.macro add
  addl vB, vA
.endm
.macro sub
  subl vB, vA
.endm
.macro or
  orl vB, vA
.endm
.macro and
  andl vB, vA
.endm
.macro xor
  xorl vB, vA
.endm
.macro shl
  shll rShift, vA
.endm
.macro shr
  shrl rShift, vA
.endm
.macro get
  andl $DM_MASK, vB
  movl data_memory(,vB,WD_BYTES), vA
.endm
.macro put
  andl $DM_MASK, vB
  movl vA, data_memory(,vB,WD_BYTES)
.endm
.macro at
  andl $DM_MASK, vB
  movl vB, vD
  movl data_memory(,vB,WD_BYTES), vB
.endm
.macro asav
  movl vA, data_memory(,vD,WD_BYTES)
.endm
.macro bsav
  movl vB, data_memory(,vD,WD_BYTES)
.endm
.macro copy
  movl vB, vD
  andl $DM_MASK, vD
  movl data_memory(,vD,WD_BYTES), vB
  addl vA, vD
  andl $DM_MASK, vD
  movl vB, data_memory(,vD,WD_BYTES)
.endm
.macro inc
  incl vB
.endm
.macro dec
  decl vB
.endm
.macro set
  orl $MSb, vB
.endm
.macro swap
.ifeq x86_64
  movl vA, %r8d
  movl vB, vA
  movl %r8d, vB
.else
  xorl vA, vB
  xorl vB, vA
  xorl vA, vB
.endif
.endm
.macro tob
  movl vA, vB
.endm
.macro tot
  movl vA, vT
.endm
.macro tor
  movl vA, vR
.endm
.macro fromb
  movl vB, vA
.endm
.macro fromt
  movl vT, vA
.endm
.macro fromr
  movl vR, vA
.endm
.macro mdm
  movl $DM_WORDS, vA
.endm
.macro noop
  nop
.endm
.macro halt
  andl $BYTE_MASK, vA
  do_exit vA
.endm
.macro jump label
  jmp \label
.endm
.macro jmpe label
  cmpl vB, vA
  jne 1f
    jmp \label
  1:
.endm
.macro jmpb label
  cmpl $0, vB
  jnz 1f
    jmp \label
  1:
.endm
.macro br label
.ifeq x86_64
  leaq 1f, vL
.else
  leal 1f, vL
.endif
  jmp \label
  1:
.endm
.macro link label
  jmp *vL
.endm
.macro rpt label
  cmpl $0, vR
  jz 1f
  decl vR
  jmp \label
  1:
.endm

##############################################################################
.section .data ###############################################################
##############################################################################

# ============================================================================
#                                 TRACING
# ============================================================================
.ifeq TRACING_ENABLED
  .extern printf

  .ifeq x86_64
    .macro SAVE_REGS
      # Save physical CPU registers
      pushq %rax
      pushq %rbx
      pushq %rcx
      pushq %rdx
      pushq %rsi
      pushq %rdi
      pushq %r8
      pushq %r9
    .endm
    .macro RESTORE_REGS
      # Restore physical CPU registers
      popq %r9
      popq %r8
      popq %rdi
      popq %rsi
      popq %rdx
      popq %rcx
      popq %rbx
      popq %rax
    .endm
    # x86-64: not fully tested and might not be entirely correct
    pntfmt: .asciz "\nvA:%08x vB:%08x vT:%08x vR:%08x vD:%08x vL:%016x"
    .macro TRACE_INSTR
      SAVE_REGS
      # Some juggling to prepare to supply printf arguments
      pushq %rdi # vL (this will be popped by printf)
      pushq %rbx # vD (this will be read from %r9  by printf)
      pushq %rsi # vR (this will be read from %r8  by printf)
      pushq %rdx # vT (this will be read from %rcx by printf)
      pushq %rcx # vB (this will be read from %rdx by printf)
      pushq %rax # vA (this will be read from %rsi by printf)
      pushq $pntfmt # (this will be read from %rdi by printf)
      movq $6, %rax # number of vector registers used
      popq %rdi # arg1 in %rdi = $pntfmt
      popq %rsi # arg2 in %rsi = vA from %eax
      popq %rdx # arg3 in %rdx = vB from %ecx
      popq %rcx # arg4 in %rcx = vT from %edx
      popq %r8  # arg5 in %r8  = vR from %esi
      popq %r9  # arg6 in %r9  = vD from %ebx
      call printf
      addq $8, %rsp # only need to adjust for pushq %rdi # vL
      RESTORE_REGS
    .endm
  .else
    .macro SAVE_REGS
      # Save physical CPU registers
      pushal
    .endm
    .macro RESTORE_REGS
      # Restore physical CPU registers
      popal
    .endm
    # x86-32: appears to be ok
    pntfmt: .asciz "\nvA:%08x vB:%08x vT:%08x vR:%08x vD:%08x vL:%08x"
    .macro TRACE_INSTR
      SAVE_REGS
      pushl vL
      pushl vD
      pushl vR
      pushl vT
      pushl vB
      pushl vA
      pushl $pntfmt
      call printf
      addl $28, %esp
      RESTORE_REGS
    .endm
  .endif

.endif

##############################################################################
.section .bss ################################################################
##############################################################################
saved_word: .lcomm sw, WD_BYTES
data_memory: .lcomm dm, DM_BYTES

##############################################################################
.section .text ###############################################################
##############################################################################

# ============================================================================
#                              VM EXIT POINTS
# ============================================================================
.macro do_exit reg_vA
  .ifeq LINKING_WITH_LD_ON_LINUX
    movl \reg_vA, %ebx          # Exit code (status)
    movl $0x1, %eax             # Linux call ID for exit
    int $0x80                   # Linux interrupt for system call
  .else
    movl \reg_vA, %eax          # Exit code (status)
    ret
  .endif
.endm
vm_success:
  movl $SUCCESS, vA
  do_exit vA
vm_failure:
  movl $FAILURE, vA
  do_exit vA
vm_exit:
  andl $BYTE_MASK, vA
  do_exit vA
# ============================================================================
#                             VM INITIALIZATION
# ============================================================================
.macro do_init
  xorl vA, vA
  xorl vB, vB
  xorl vT, vT
  xorl vR, vR
.ifeq x86_64
  xorq vL, vL #FIXME 64-bit zero all regs
.else
  xorl vL, vL
.endif
  xorl vD, vD
.endm
# ============================================================================
#                               VM ENTRY POINT
# ============================================================================
.ifeq LINKING_WITH_LD_ON_LINUX
  .global _start
  _start:
.else
  .global main
  main:
.endif

vm_pre_init:
  do_init
# ============================================================================
#                                   USAGE
#
#     Your program would begin here. Rather than adding your program
#     to this file, you should simply include this 'qmisc.s' as
#     the first line of your program by:
#
#       .include "qmisc.s"
#
#     For an example of doing so please see 'perftest.s'.
#
#     You must also set the build flag NO_PROGRAM to NO, otherwise
#     your program will not run (instead the VM will just immediately
#     exit via vm_success rather than running your program).
#
# ============================================================================
.ifeq NO_PROGRAM
  jmp vm_success
.endif
