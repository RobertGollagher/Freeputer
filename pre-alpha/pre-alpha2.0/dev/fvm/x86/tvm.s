/*
                      TINY VIRTUAL MACHINE (TVM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170721
Updated:    20170722+
Version:    pre-alpha-0.0.0.6 for FVM 2.0

Notes: This is an experiment along the lines of srm.s but even simpler.
It attemps to be about two orders of magnitude simpler than FVM 2.0.
The initial version of tvm.s is identical to srm.s.
It will be progressively cut down and simplified.


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
/*



*/
# ============================================================================
#                                IMPORTS
# ============================================================================
.extern putchar
.extern getchar

# ============================================================================
#                                SYMBOLS
# ============================================================================
.equ SUCCESS, 0
.equ FAILURE, 1
.equ MM_BYTES, 0x1000000
.equ WORD_SIZE, 4
# Registers of the virtual machine:
.equ vA, %ebx # accumulator
.equ vB, %edx # operand register
.equ vL, %edi # link register
# Registers of the implementation:
.equ rTmp, %eax # primary temporary register
.equ rBuf, %ecx # secondary temporary register

# ============================================================================
#                            MACROS: PRIMITIVES
# ============================================================================
.macro OUTCHAR reg
  pushl \reg
  call putchar
  addl $WORD_SIZE, %esp
.endm

.macro INCHAR
  call getchar
.endm

.macro reg_imm x reg
  movl $\x, \reg
.endm

.macro reg_at x reg
  reg_imm \x rTmp
  reg_load rTmp \reg
.endm

.macro reg_sign_extend x reg
  reg_imm \x \reg
  reg_imm 0x00800000 rBuf
  andl \reg, rBuf
  jz 1f
    orl $0xff000000, \reg
  1:
.endm

.macro reg_ptr_pp regPtr
  addl $WORD_SIZE, memory(,\regPtr,1)
.endm

.macro reg_mm regPtr
  subl $WORD_SIZE, \regPtr
.endm

.macro reg_m x reg
  reg_imm \x rTmp
  shll $8, rTmp
  andl $0x00ffffff, \reg
  orl rTmp, \reg
.endm

.macro reg_store regSrc regDst
  movl \regSrc, memory(,\regDst,1)
.endm

.macro reg_load regSrc regDst
  movl memory(,\regSrc,1), \regDst
.endm

.macro i_add
  addl vB, vA
.endm

.macro i_sub
  subl vB, vA
.endm

.macro i_or
  orl vB, vA
.endm

.macro i_and
  andl vB, vA
.endm

.macro i_xor
  xorl vB, vA
.endm

.macro i_shl
  movl vB, rTmp
  movl rTmp, %ecx
  shll %cl, vA
.endm

.macro i_shr
  movl vB, rTmp
  movl rTmp, %ecx
  shrl %cl, vA
.endm

.macro i_branch label
  movl 1f, vL
  jump \label
  1:
.endm

.macro i_merge
  jmp vL
.endm

.macro i_swap
  movl vA, rBuf
  movl vB, vA
  movl rBuf, vB
.endm

.macro i_nop
  nop
.endm

.macro i_halt
  movl vB, %eax
  jmp vm_success
.endm

.macro do_failure
  movl $FAILURE, rTmp
  ret
.endm

.macro do_success
  movl $SUCCESS, rTmp
  ret
.endm

# ============================================================================
#                            MACROS: DERIVED
# ============================================================================

.macro reg_ptr_load x reg
  reg_at rTmp
  reg_load rTmp \reg
.endm

.macro reg_ptr_load_pp x reg
  reg_imm \x rBuf
  reg_load rBuf rTmp
  reg_ptr_pp rBuf
  reg_load rTmp reg
.endm

.macro reg_ptr_load_mm x reg
  reg_at \x rBuf
  reg_mm rBuf
  reg_store rBuf rTmp
  reg_load rBuf vA
.endm

# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
# ----------------------------------------------------------------------------
#                           MOVE INSTRUCTIONS - keep these
# ----------------------------------------------------------------------------
# IDEA: make word 8-bits and use indirection a lot, in 2 layers
.macro lit x
  reg_imm \x vA
.endm

.macro litx x
  reg_sign_extend \x vA
.endm

.macro litm x
  reg_m vA
.endm
# ----------------------------------------------------------------------------
.macro from x
  reg_at \x vA
.endm

.macro from_ptr x
  reg_ptr_load x vA
.endm

.macro from_ptr_pp x
  reg_ptr_load_pp x vA
.endm

.macro from_ptr_mm x
  reg_ptr_load_mm x vA
.endm
# ----------------------------------------------------------------------------
.macro by x
  reg_imm \x vB
.endm

.macro byx x
  reg_sign_extend \x vB
.endm

.macro bym x
  reg_m vB
.endm

.macro by_at x
  reg_at \x vB
.endm

.macro by_ptr x
  reg_ptr_load x vB
.endm

.macro by_ptr_pp x
  reg_ptr_load_pp x vB 
.endm

.macro by_ptr_mm x
  reg_ptr_load_pp x vB
.endm
# ----------------------------------------------------------------------------
.macro to x
  reg_imm \x rTmp
  reg_store vA rTmp
.endm

.macro to_ptr x
  reg_at \x rTmp
  reg_store vA rTmp
.endm

.macro to_ptr_pp x
  reg_imm \x rBuf
  reg_load rBuf rTmp
  reg_store vA rTmp
  reg_ptr_pp rBuf
.endm

.macro to_ptr_mm x
  reg_at \x rBuf
  reg_mm rBuf
  reg_store rBuf rTmp
  reg_store vA rBuf
.endm
# ----------------------------------------------------------------------------
#                           ARITHMETIC INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro add by x
  \by \x
  i_add
.endm
# ----------------------------------------------------------------------------
.macro sub by x
  \by \x
  i_subl
.endm
# ----------------------------------------------------------------------------
#                               BITWISE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro or by x
  \by \x
  i_orl
.endm
# ----------------------------------------------------------------------------
.macro and by x
  \by \x
  andl vB, vA
.endm
# ----------------------------------------------------------------------------
.macro xor by x
  \by \x
  i_xorl
.endm
# ----------------------------------------------------------------------------
.macro shl by x
  \by \x
  i_shl
.endm
# ----------------------------------------------------------------------------
.macro shr by x
  \by \x
  i_shr
.endm
# ----------------------------------------------------------------------------
#                   JUMP INSTRUCTIONS maybe decleq
# ----------------------------------------------------------------------------
.macro jmpr baseAddr
  # FIXME a bit dicey
  leal \baseAddr(,vA,WORD_SIZE), %eax
  jmp *(%eax)
.endm

.macro jump label
  leal \label, rTmp
  jmp rTmp
.endm

# This provides a nice bit-30 overflow-detection solution
.macro jmpo label
  leal \label, rTmp
  andl vA, $0x80000000
  jz positive
    andl vA, $0x40000000
    jnz ok
      jmp rTmp
  positive:
    andl vA, $0x40000000
    jz ok
      jmp rTmp \label
  ok:
.endm

.macro jmpz label
  leal \label, rTmp
  xorl $0, vA
  jnz 1f
    jmp rTmp
  1:
.endm

.macro jmpnz label
  leal \label, rTmp
  xorl $0, vA
  jz 1f
    jmp rTmp
  1:
.endm

.macro jmplz label
  leal \label, rTmp
  cmp $0, vA
  jge 1f
    jmp rTmp
  1:
.endm

.macro jmpgz label
  leal \label, rTmp
  cmp $0, vA
  jle 1f
    jmp rTmp
  1:
.endm

.macro jmplez label
  leal \label, rTmp
  cmp $0, vA
  jg 1f
    jmp rTmp
  1:
.endm

.macro jmpgez label
  leal \label, rTmp
  cmp $0, vA
  jl 1f
    jmp rTmp
  1:
.endm
# ----------------------------------------------------------------------------
#                     BRANCH/MERGE using vL link register
# ----------------------------------------------------------------------------
.macro branch label
  i_branch \label
.endm

.macro merge
  i_merge
.endm
# ----------------------------------------------------------------------------
#                            OTHER INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro swap
  i_swap
.endm

.macro nop
  i_nop
.endm

.macro halt by x
  \by \x
  i_halt
.endm

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
memory: .lcomm mm, MM_BYTES

# ============================================================================
.section .text #             EXIT POINTS for the VM
# ============================================================================
vm_failure:

  do_failure

vm_success:

  do_success

# ============================================================================
# ========================= EXAMPLE PROGRAM ==================================
# ============================================================================
.equ v_memory, 0

# ============================================================================
#                     ENTRY POINT for example program
# ============================================================================
.global main
  main:
    lit 1
    halt by 0

# ============================================================================

