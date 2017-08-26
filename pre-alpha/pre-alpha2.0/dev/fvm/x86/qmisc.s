/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)


Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170826
Updated:    20170826+
Version:    pre-alpha-0.0.0.1 for FVM 2.0
=======

                              This Edition:
                          x86 Assembly Language
                           using GNU Assembler
                            for 32-bit Linux

                               ( ) [ ] { }

 See 'qmisc.c'. This an x86 implementation of that earlier proof of concept.

==============================================================================
                            BUILDING FOR i386
==============================================================================

You may need gcc-multilib installed.

When linking with gcc for debugging, build with:

  as -g --gstabs -o qmisc.o qmisc.s --32
  gcc -o qmisc qmisc.o -m32

When linking with gcc for release, build with:

  as -o qmisc.o qmisc.s --32
  gcc -o qmisc qmisc.o -m32

When LINKING_WITH_LD_ON_LINUX without any imports, build with:

  as -o qmisc.o qmisc.s --32
  ld -o qmisc qmisc.o -m elf_i386

Or for convenience, build with:

  ./build.sh

Or for convenience, build and run with:

  ./go.sh

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/

# ============================================================================
#                                SYMBOLS
# ============================================================================
.equiv TRACING_ENABLED, 0           # 0 = true, 1 = false
.equiv LINKING_WITH_LD_ON_LINUX, 0  # 0 = true, 1 = false

.equ WD_BYTES, 4
.equ MSb,           0x80000000  # Bit mask for most significant bit
.equ METADATA_MASK, 0x7fffffff  # 31 bits
.equ BYTE_MASK,     0x000000ff
.equ SHIFT_MASK,    0x0000001f
.equ SUCCESS, 0
.equ FAILURE, 1
.equ ILLEGAL, 2
.equ MAX_DM_WORDS,  0x10000000  # Must be 2^(WD_BITS-4) due to C limitations.
.equ DM_WORDS, MAX_DM_WORDS     # Must be some power of 2 <= MAX_DM_WORDS.
.equ DM_MASK, DM_WORDS-1
# There are only 4 accessible registers:
.equ vA, %eax; # accumulator
.equ vB, %ebx; # operand register
.equ vT, %edx; # temporary register
.equ vR, %esi; # repeat register
.equ vL, %edi; # link register (not accessible)
.equ rSwap, %ecx; # swap register (not accessible) (sometimes reused here)
.equ rShift, %cl; # register used for shift magnitude (not accessible)
# ============================================================================
# ==================== START OF PLATFORM-SPECIFIC CODE =======================
# ============================================================================
.macro do_exit status
  .ifdef LINKING_WITH_LD_ON_LINUX
    movl $\status, %ebx         # Exit code (status)
    movl $0x1, %eax             # Linux call ID for exit
    int $0x80                   # Linux interrupt for system call
  .else
    movl $status, %eax         # Exit code (status)
    ret
  .endif
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
  movl vB, rSwap
  andl $SHIFT_MASK, rSwap
  shll rShift, vA
.endm
.macro i_shr
  movl vB, rSwap
  andl $SHIFT_MASK, rSwap
  shrl rShift, vA
.endm.macro i_get
  #FIXME
.endm
.macro i_put
  #FIXME
.endm
.macro i_at
  #FIXME
.endm
.macro i_inc
  incl vB
.endm
.macro i_dec
  decl vB
.endm
.macro i_imm x
  #FIXME
.endm
.macro i_flip
  xorl $MSb, vB
.endm
# ============================================================================
# ====================== END OF PLATFORM-SPECIFIC CODE =======================
# ============================================================================

# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
.macro add
  i_add
.endm
.macro sub
  i_sub
.endm
.macro or
  i_or
.endm
.macro and
  i_and
.endm
.macro xor
  i_xor
.endm
.macro shl
  i_shl
.endm
.macro shr
  i_shr
.endm
.macro get
  i_get
.endm
.macro put
  i_put
.endm
.macro at
 i_at
.endm
.macro inc
  i_inc
.endm
.macro dec
  i_dec
.endm
.macro imm x
  i_imm \x
.endm
.macro flip
  i_flip
.endm
# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================

# ============================================================================
.section .text #           EXIT POINTS for the VM
# ============================================================================
vm_failure:
  do_exit(FAILURE)
vm_success:
  do_exit(SUCCESS)
# =========================== EXAMPLE PROGRAM ================================
# Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
# ============================================================================

# ============================================================================
#                      ENTRY POINT FOR EXAMPLE PROGRAM
# ============================================================================
.ifdef LINKING_WITH_LD_ON_LINUX
  .global _start
  _start:
.else
  .global main
  main:
.endif

jmp vm_success
# ============================================================================

