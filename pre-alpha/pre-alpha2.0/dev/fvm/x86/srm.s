/*
                      SINGLE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170709
Updated:    20170712+
Version:    pre-alpha-0.0.0.2 for FVM 2.0


                              This Edition:
                           32-bit i386 native
                          x86 Assembly Language
                           using GNU Assembler

                               ( ) [ ] { }

This experimental version uses macros and is compiled to native x86.
Therefore this can easily be ported to other architectures such as ARM.
Once experimentation is complete an interpreted VM will also be implemented.
Note: it perhaps makes sense to make this a Harvard architecture.

==============================================================================
                            BUILDING FOR i386
==============================================================================

For debugging build with:

  as -g --gstabs -o fvm.o fvm.s --32
  gcc -o fvm fvm.o -m32

For release build with:

  as -o fvm.o fvm.s --32
  gcc -o fvm fvm.o -m32

Alternative if no imports:

  as -o fvm.o fvm.s --32
  ld -o fvm fvm.o -m elf_i386

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
# ============================================================================
#                                IMPORTS
# ============================================================================
.extern printf

# ============================================================================
#                                SYMBOLS
# ============================================================================
.equ SUCCESS, 0x00
.equ FAILURE, 0x01
.equ MM_BYTES, 0x01000000
.equ vA, %ebx
.equ vC, %edx
.equ rA, %eax
.equ rC, %ecx

# ============================================================================
.section .data #                CONSTANTS
# ============================================================================
version: .asciz "SRM 0.0.0.0\n"
success: .asciz "SRM success\n"
failure: .asciz "SRM failure\n"
illegal: .asciz "SRM illegal opcode: "
format_hex8: .asciz "%08x"
newline: .asciz "\n"
space: .asciz " "

# ============================================================================
#                             INSTRUCTION SET
# ============================================================================
.macro HALT
  jmp vm_success
.endm

.macro FAIL
  jmp vm_failure
.endm

.macro lit imm
  movl $\imm, vA
.endm

.macro from imm
  movl $\imm, rA
  movl memory(,rA,1), vA
.endm

.macro to imm
  movl $\imm, rA
  movl vA, memory(,rA,1)
.endm

.macro subm imm
  movl $\imm, rA
  subl vA, memory(,rA,1)
.endm

.macro sub imm
  subl $\imm, vA
.endm

.macro jnz label
  xorl $0, vA
  jz 1f
    jmp \label
  1:
.endm

.macro repeats imm
  movl $\imm, vC
.endm

.macro again label
  decl vC
  jle 1f
    jmp \label
  1:
.endm

.macro pop imm
  movl $\imm, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl memory(,rC,1), vA
.endm

.macro push imm
  movl $\imm, rC
  movl memory(,rC,1), rA
  movl vA, memory(,rA,1)
  addl $4, memory(,rC,1)
.endm

/*
.macro

.endm
*/

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
memory: .lcomm mm, MM_BYTES

# ============================================================================
#                                 TRACING
# ============================================================================
.macro TRACE_STR strz
  SAVE_REGS
  pushl \strz
  call printf
  addl $4, %esp
  RESTORE_REGS
.endm

.macro TRACE_HEX8 rSrc
  SAVE_REGS
  pushl %eax
  pushl \rSrc
  pushl $format_hex8
  call printf
  addl $8, %esp
  popl %eax
  RESTORE_REGS
.endm

.macro SAVE_REGS
  pushal
.endm

.macro RESTORE_REGS
  popal
.endm

# ============================================================================
#                            EXAMPLE VARIABLES
# ============================================================================
.equ a, 0x00
.equ b, 0x04
.equ c, 0x08
.equ counter, 0x0c
.equ ptr, 0x10

# ============================================================================
.section .text #                ENTRY POINT
# ============================================================================
.global main
main:

  # Initialize pointer
  lit 0x14
  to ptr

  # Inbuilt looping for gigantic stack = 0.20 sec
  lit 9
  repeats 0x3fff00
  loop:
    push ptr
    pop ptr
    again loop

  HALT


vm_failure:

  # PRINT FAILURE MESSAGE AND EXIT ====
  TRACE_STR $failure
  movl $FAILURE, %eax
  ret
  # ===================================

vm_success:

  #  PRINT SUCCESS MESSAGE AND EXIT ===
  TRACE_STR $success
  movl $SUCCESS, %eax
  ret
  #  ==================================

# ============================================================================
