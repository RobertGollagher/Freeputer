/*
                      TINY VIRTUAL MACHINE (TVM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170721
Updated:    20170722+
Version:    pre-alpha-0.0.0.4 for FVM 2.0

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
.extern printf
.extern putchar

# ============================================================================
#                                SYMBOLS
# ============================================================================
.equ SUCCESS, 0
.equ FAILURE, 1
.equ MM_BYTES, 0x1000000
.equ WORD_SIZE, 4
.equ vA, %ebx
.equ vX, %edx
.equ vL, %edi
.equ rA, %eax
.equ rC, %ecx

# ============================================================================
#                            GENERAL MACROS
# ============================================================================
.macro OUTCHAR reg
  pushl \reg
  call putchar
  addl $WORD_SIZE, %esp
.endm

.macro INCHAR
  call getchar
.endm

.macro reg_imm metadata reg
  movl $\metadata, \reg
.endm

.macro reg_sign_extend reg
  reg_imm 0x00800000 rC
  andl \reg, rC
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

.macro reg_m metadata reg
  shll $8, rA
  andl $0x00ffffff, \reg
  orl rA, \reg
.endm

.macro reg_store regSrc regDst
  movl \regSrc, memory(,\regDst,1)
.endm

.macro reg_load regSrc regDst
  movl memory(,\regSrc,1), \regDst
.endm

.macro reg_ptr_load regPtr regToLoad
  reg_load \regPtr, \regPtr
  reg_load \regPtr, \regToLoad
.endm

.macro reg_ptr_load_pp regPtr regToLoad
  reg_load \regPtr rA
  reg_ptr_pp \regPtr
  reg_load rA regToLoad
.endm

.macro reg_ptr_load_mm regPtr regToLoad
  reg_load \regPtr rC
  reg_mm rC
  reg_store rC \regPtr
  reg_load rC vA
.endm

# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
# ----------------------------------------------------------------------------
#                           MOVE INSTRUCTIONS - keep these
# ----------------------------------------------------------------------------
# IDEA: make word 8-bits and use indirection a lot, in 2 layers
.macro lit metadata
  reg_imm \metadata vA
.endm

.macro litx metadata
  reg_imm \metadata vA
  reg_sign_extend vA
.endm

.macro litm metadata
  reg_imm \metadata rA
  reg_m rA
.endm
# ----------------------------------------------------------------------------
.macro from metadata
  reg_imm \metadata rA
  reg_load rA vA
.endm

.macro from_ptr metadata
  reg_imm \metadata rA
  reg_ptr_load rA vA
.endm

.macro from_ptr_pp metadata
  reg_imm \metadata rC
  reg_ptr_load_pp rC vA
.endm

.macro from_ptr_mm metadata
  reg_imm \metadata rA
  reg_ptr_load_mm rA vA
.endm
# ----------------------------------------------------------------------------
.macro by metadata
  reg_imm \metadata vX
.endm

.macro byx metadata
  reg_imm \metadata vX
  reg_sign_extend vX
.endm

.macro bym metadata
  reg_imm \metadata rA
  reg_m vX
.endm

.macro by_at metadata
  reg_imm \metadata rA
  reg_load rA vX
.endm

.macro by_ptr metadata
  reg_imm \metadata rA
  reg_ptr_load rA vX
.endm

.macro by_ptr_pp metadata
  reg_imm \metadata rC
  reg_ptr_load_pp rC vX 
.endm

.macro by_ptr_mm metadata
  reg_imm \metadata rA
  reg_ptr_load_pp rA vX
.endm
# ----------------------------------------------------------------------------
.macro to metadata
  reg_imm \metadata rA
  reg_store vA rA
.endm

.macro to_ptr metadata
  reg_imm \metadata rA
  reg_load rA rA
  reg_store vA rA
.endm

.macro to_ptr_pp metadata
  reg_imm \metadata rC
  reg_load rC rA
  reg_store vA rA
  reg_ptr_pp rC
.endm

.macro to_ptr_mm metadata
  reg_imm \metadata rA
  reg_load rA rC
  reg_mm rC
  reg_store rC rA
  reg_store vA rC
.endm
# ----------------------------------------------------------------------------
#                           ARITHMETIC INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro add by metadata
  \by \metadata
  addl vX, vA
.endm
# ----------------------------------------------------------------------------
.macro sub by metadata
  \by \metadata
  subl vX, vA
.endm
# ----------------------------------------------------------------------------
#                               BITWISE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro or by metadata
  \by \metadata
  orl vX, vA
.endm
# ----------------------------------------------------------------------------
.macro and by metadata
  \by \metadata
  andl vX, vA
.endm
# ----------------------------------------------------------------------------
.macro xor by metadata
  \by \metadata
  xorl vX, vA
.endm
# ----------------------------------------------------------------------------
.macro shl by metadata
  \by \metadata
  movl vX, rA
  movl rA, %ecx
  shll %cl, vA
.endm
# ----------------------------------------------------------------------------
.macro shr by metadata
  \by \metadata
  movl vX, rA
  movl rA, %ecx
  shrl %cl, vA
.endm
# ----------------------------------------------------------------------------
#                   JUMP INSTRUCTIONS maybe decleq
# ----------------------------------------------------------------------------
.macro jumprel baseAddr # FIXME harmonize
  # A bit dicey
  leal \baseAddr(,vA,WORD_SIZE), %eax
  jmp *(%eax)
.endm

.macro jumpx by metadata # FIXME harmonize
  \by \metadata
  jmp vX
.endm

.macro jump label
  leal \label, rA
  jmp rA
.endm

# This provides a nice bit-30 overflow-detection solution
.macro jmpo label
  leal \label, rA
  andl vA, $0x80000000
  jz positive
    andl vA, $0x40000000
    jnz ok
      jmp rA
  positive:
    andl vA, $0x40000000
    jz ok
      jmp rA \label
  ok:
.endm

.macro jmpz label
  leal \label, rA
  xorl $0, vA
  jnz 1f
    jmp rA
  1:
.endm

.macro jmpnz label
  leal \label, rA
  xorl $0, vA
  jz 1f
    jmp rA
  1:
.endm

.macro jmplz label
  leal \label, rA
  cmp $0, vA
  jge 1f
    jmp rA
  1:
.endm

.macro jmpgz label
  leal \label, rA
  cmp $0, vA
  jle 1f
    jmp rA
  1:
.endm

.macro jmplez label
  leal \label, rA
  cmp $0, vA
  jg 1f
    jmp rA
  1:
.endm

.macro jmpgez label
  leal \label, rA
  cmp $0, vA
  jl 1f
    jmp rA
  1:
.endm
# ----------------------------------------------------------------------------
#                            OTHER INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro swap
  movl vA, rC
  movl vX, vA
  movl rC, vX
.endm

.macro nop
.endm

.macro halt by metadata
  \by \metadata
  movl vX, %eax
  jmp vm_success
.endm

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
# For the meta-machine
memory: .lcomm mm, MM_BYTES

# ============================================================================
.section .text #             EXIT POINTS for the VM
# ============================================================================
vm_failure:

  movl $FAILURE, rA
  ret

vm_success:

  movl $SUCCESS, rA
  ret

# ============================================================================
# ============================================================================
# ========================= EXAMPLE PROGRAM ==================================
# ============================================================================
# ============================================================================
#                 EXAMPLE VARIABLES for an example program
# ============================================================================
.equ v_memory, 0
.equ rsp, v_memory + 16
.equ linkr, rsp + WORD_SIZE
.equ v_vA, linkr + WORD_SIZE
.equ v_vX, v_vA + WORD_SIZE
.equ v_pc, v_vX + WORD_SIZE
.equ instr, v_pc + WORD_SIZE

# ============================================================================
#   EXAMPLE MACROS for an example program (not part of the instruction set!)
# ============================================================================
.macro CALLING label
  lit 1f
  to_ptr_pp rsp
  jump \label
  1:
.endm

.macro RETURN
  jumpx by_ptr_mm rsp
.endm

# FIXME make brl instruction using vL and merge instruction
.macro BRANCH label
  lit 1f
  to linkr
  jump \label
  1:
.endm

.macro MERGE
  jumpx by_at linkr
.endm

# ============================================================================
#                     ENTRY POINT for an example program
# ============================================================================
.global main
  main:
    lit 1
    halt by 0

# ============================================================================

