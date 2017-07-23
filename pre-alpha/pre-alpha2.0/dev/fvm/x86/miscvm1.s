/*
                              MISC VM 1

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    miscvm1
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170723
Updated:    20170723+
Version:    pre-alpha-0.0.0.2+ for FVM 2.0

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
  - Replaced to,from with load,store,pull,put,pop,push (1 stack direction)
  - Experimental: vS, vD; lit goes to vB!
  - Removed litx, litm (code smell)

  NOTES:

  - need space for up to 64 opcodes
  - so have 2 spare bits

  DECISIONS:

  - Harvard architecture is critical to success
  - Lits and jumps need to be followed by a full word
  - Therefore must have 2 instruction types, not fixed width
  - Or very wasteful encoding similar to FVM 1
  - not 16 bit, must be 32 bit

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
.equ vA, %eax # was %ebx # accumulator
.equ vB, %ebx # was %edx # operand register
.equ vL, %edx # was %eax # link register # FIXME maybe remove this
.equ vS, %esi # source address register
.equ vD, %edi # destination address register
# Registers of the implementation:
.equ rTmp, %ecx # temporary register (must be %ecx because of shl, shr)

# ============================================================================
# ============================================================================
.macro OUTCHAR reg
  pushl \reg
  call putchar
  addl $WORD_SIZE, %esp
.endm

.macro INCHAR #FIXME
  call getchar
  movl %eax, vA
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
  xorl vD, vD
  xorl vS, vS
  xorl rTmp, rTmp
.endm

# ============================================================================
.section .data #             INSTRUCTION SET  #FIXME maybe go 32 not 24?
# ============================================================================
.macro lit x
  movl $\x, vB
.endm
# ----------------------------------------------------------------------------
.macro load # replaces from x
  movl data_memory(,vS,1), vA
.endm

.macro store # replaces to x
  movl vA, data_memory(,vD,1)
.endm

.macro pull # replaces from_ptr x
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
.endm

.macro put # replaces to_ptr x
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
.endm

.macro decs
  subl $WORD_SIZE, vS
.endm

.macro incs
  addl $WORD_SIZE, vS
.endm

.macro decd
  subl $WORD_SIZE, vS
.endm

.macro incd
  addl $WORD_SIZE, vS
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
  movl vB, %ecx
  shll %cl, vA
.endm

.macro shr
  movl vB, %ecx
  shrl %cl, vA
.endm
# ----------------------------------------------------------------------------
.macro jmpr baseAddr
  leal \baseAddr(,vA,WORD_SIZE), rTmp
  jmp *(rTmp)
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
.macro lcall label
  movl 1f, vL
  jump \label
  1:
.endm

.macro lret
  jmp vL
.endm
# ----------------------------------------------------------------------------
.macro in # FIXME
  INCHAR
.endm

.macro out # FIXME
  OUTCHAR vA
.endm
# ----------------------------------------------------------------------------
.macro movab
  movl vA, vB
.endm

.macro movad
  movl vA, vD
.endm

.macro moval
  movl vA, vL
.endm

.macro movas
  movl vA, vS
.endm

.macro movba
  movl vB, vA
.endm

.macro movbd
  movl vB, vD
.endm

.macro movbl
  movl vB, vD
.endm

.macro movbs
  movl vB, vS
.endm

.macro movda
  movl vS, vA
.endm

.macro movla
  movl vL, vA
.endm

.macro movsa
  movl vS, vA
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

  lit 0x7fffffff
  movba

  loop: # 1.4 seconds
    lit 1
    sub
    jmpgz loop

  halt

# ============================================================================

