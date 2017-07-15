/*
                      SINGLE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170709
Updated:    20170715+
Version:    pre-alpha-0.0.0.4 for FVM 2.0


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
exit: .asciz "SRM exit code: "
illegal: .asciz "SRM illegal opcode with metadata: "
format_hex8: .asciz "%08x"
newline: .asciz "\n"
space: .asciz " "

# ============================================================================
#                             INSTRUCTION SET
# ============================================================================
/*

  LATEST THOUGHTS:

  - FW32
  - word-addressing
  - 1 bit: jmp-format instruction
  - 2 bits: mode (imm,@,@++,--@)
  - 4 bits: opcode:
      - lit, from, to = 3
      - add, sub, mul, div = 4
      - shr, shl, and, or, xor = 5
      - reserved1, reserved1, nop, halt = 3
  - jumps: always absolute
      - jmp, jz, jnz, jlz, jgz, jo = 6
      - skip, skz, sknz, sklz, skgz, sko, skeq, skne, skle, skge = 10
  - 24 bits: metadata

  PREFERRED:

  - Keep it simple, have it in 1 direction only: M->R
  - bit-30 overflow solutions and 15-bit multiply, not sure div

*/

# ----------------------------------------------------------------------------
#                                STORE MACROS
# ----------------------------------------------------------------------------
# M = x
.macro dst_imm x
  movl $\x, rA
  movl vA, memory(,rA,1)
.endm

# M = @x
.macro dst_at x
  movl $\x, rA
  movl memory(,rA,1), rA
  movl vA, memory(,rA,1)
.endm

# M = @x++
.macro dst_pp x
  movl $\x, rC
  movl memory(,rC,1), rA
  movl vA, memory(,rA,1)
  addl $4, memory(,rC,1)
.endm

# M = --@x
.macro mm_dst x
  movl $\x, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl vA, memory(,rC,1)
.endm

# ----------------------------------------------------------------------------
#                                 LOAD MACROS
# ----------------------------------------------------------------------------
# R = x
.macro src_imm x
  movl $\x, rA
  movl memory(,rA,1), vA
.endm

# R = @x
.macro src_at x
  movl $\x, rA
  movl memory(,rA,1), rA
  movl vA, memory(,rA,1)
.endm

# R = @x++
.macro src_pp x
  movl $\x, rC
  movl memory(,rC,1), rA
  movl memory(,rA,1), vA
  addl $4, memory(,rC,1)
.endm

# R = --@x
.macro mm_src x
  movl $\x, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl memory(,rC,1), vA
.endm

# ----------------------------------------------------------------------------
#                             MOVE INSTRUCTIONS
# ----------------------------------------------------------------------------
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

# ----------------------------------------------------------------------------
#                           ARITHMETIC INSTRUCTIONS
# ----------------------------------------------------------------------------
# TODO carry? overflow?
.macro add imm
  addl $\imm, vA
.endm

.macro sub imm
  subl $\imm, vA
.endm

.macro mul imm
  imull $\imm, vA
.endm

.macro div imm
  movl vA, %eax
  movl $\imm, %ebx

  test %ebx, %ebx
  je 1f

  cdq             # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx      # %edx:eax is the implied dividend
  jmp 2f
  1: # Division by zero
    movl $0, vA
  2:
.endm

.macro subm imm
  movl $\imm, rA
  subl vA, memory(,rA,1)
.endm

# ----------------------------------------------------------------------------
#                               BITWISE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro or imm
  orl $\imm, vA
.endm

.macro and imm
  andl $\imm, vA
.endm

.macro xor imm
  xorl $\imm, vA
.endm

.macro shl imm
  movl $\imm, rA
  movl rA, %ecx
  shll %cl, vA
.endm

.macro shr imm
  movl $\imm, rA
  movl rA, %ecx
  shlr %cl, vA
.endm

# ----------------------------------------------------------------------------
#                               JUMP INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro jnz label
  xorl $0, vA
  jz 1f
    jmp \label
  1:
.endm

# ----------------------------------------------------------------------------
#                            OTHER INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro nop
.endm

.macro reserved2 imm
  movl $\imm, %eax
  jmp vm_illegal
.endm

.macro reserved1 imm
  movl $\imm, %eax
  jmp vm_illegal
.endm

.macro halt imm
  movl $\imm, %eax
  jmp vm_exit
.endm

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

  halt 0x12345678


vm_illegal:

  TRACE_STR $illegal
  TRACE_HEX8 rA
  TRACE_STR $newline
  ret

vm_exit:

  TRACE_STR $exit
  TRACE_HEX8 rA
  TRACE_STR $newline
  ret

# ============================================================================
