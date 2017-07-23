/*
                              MISC VM 1

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    miscvm1
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170723
Updated:    20170723+
Version:    pre-alpha-0.0.0.1+ for FVM 2.0

Notes: This is a MISC experiment which takes 'tvm.s' as its starting point
and attempts to further simplify that by another order of magnitude.


                              This Edition:
                           32-bit i386 native
                          x86 Assembly Language
                           using GNU Assembler

                               ( ) [ ] { }

This experimental version uses macros and is compiled to native x86.
Therefore this can easily be ported to other architectures such as ARM.
Once experimentation is complete an interpreted VM will also be implemented.

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
  SIMPLIFICATIONS COMPARED TO 'tvm.s':

  - Removed instructions:
      - noop (unnecessary)
      - swapAB (code smell: B should be silent)
  - Removed vZ (code smell: adds complexity)
  - Removed metadata from general-purpose instructions (act on vA, vB)
  - Removed by (code duplication of from)

  NEED TO ADD:

  - Some other way to populate vB (now that by is removed)

  RETAINING:

  - swapAL so long as retaining branch and merge (useful)


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
# Size of the virtual machine:
.equ DM_BYTES, 0x1000000 # could be up to 0x100000000
.equ WORD_SIZE, 4
# Registers of the virtual machine:
.equ vA, %ebx # accumulator
.equ vB, %edx # operand register
.equ vL, %edi # link register
# Registers of the implementation:
.equ rTmp, %eax # primary temporary register
.equ rBuf, %ecx # secondary temporary register

# ============================================================================
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

.macro reg_sign_extend x reg
  reg_imm \x \reg
  reg_imm 0x00800000 rBuf
  andl \reg, rBuf
  jz 1f
    orl $0xff000000, \reg
  1:
.endm

.macro reg_ptr_pp regPtr
  addl $WORD_SIZE, data_memory(,\regPtr,1)
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
  movl \regSrc, data_memory(,\regDst,1)
.endm

.macro reg_load regSrc regDst
  movl data_memory(,\regSrc,1), \regDst
.endm

.macro do_swap reg1 reg2
  movl \reg1, rBuf
  movl \reg2, \reg1
  movl rBuf, \reg2
.endm

.macro do_failure
  movl $FAILURE, rTmp
  ret
.endm

.macro do_success
  movl $SUCCESS, rTmp
  ret
.endm

.macro do_init
  xorl vA, vA
  xorl vB, vB
  xorl vL, vL
  xorl rTmp, rTmp
  xorl rBuf, rBuf
.endm

# ============================================================================
# ============================================================================
.macro reg_at x reg
  reg_imm \x rTmp
  reg_load rTmp \reg
.endm

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
# ----------------------------------------------------------------------------
# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
.macro lit x
  reg_imm \x vA
.endm

.macro litx x
  reg_sign_extend \x vA
.endm

.macro litm x
  reg_m \x vA
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
.macro add
  addl vB, vA
.endm

.macro sub
  subl vB, vA
.endm
# ----------------------------------------------------------------------------
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
  movl vB, rTmp
  movl rTmp, %ecx
  shll %cl, vA
.endm

.macro shr
  movl vB, rTmp
  movl rTmp, %ecx
  shrl %cl, vA
.endm
# ----------------------------------------------------------------------------
.macro jmpr baseAddr
  leal \baseAddr(,vA,WORD_SIZE), %eax
  jmp *(%eax)
.endm

.macro jump label
  jmp \label
.endm

.macro jmpz label
  xorl $0, vA
  jz \label
.endm

.macro jmpnz label
  xorl $0, vA
  jnz \label
.endm

.macro jmpgz label
  xorl $0, vA
  jg \label
.endm

.macro jmplz label
  xorl $0, vA
  jl \label
.endm

.macro jmpgez label
  xorl $0, vA
  jge \label
.endm

.macro jmplez label
  xorl $0, vA
  jle \label
.endm
# ----------------------------------------------------------------------------
.macro branch label
  movl 1f, vL
  jump \label
  1:
.endm

.macro merge
  jmp vL
.endm
# ----------------------------------------------------------------------------

.macro swapAL
  do_swap vA vL
.endm

.macro halt
  movl vB, %eax
  jmp vm_success
.endm

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
data_memory: .lcomm mm, DM_BYTES

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

# ============================================================================
#                             ENTRY POINT
# ============================================================================
.global main
main:
  do_init
  halt

# ============================================================================

