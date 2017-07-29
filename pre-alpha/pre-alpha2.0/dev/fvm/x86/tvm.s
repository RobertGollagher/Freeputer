/*
                      SPARSE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170721
Updated:    20170729+
Version:    pre-alpha-0.0.0.12+ for FVM 2.0

As of 20170729 this 'tvm.s' is again the front runner.
It really is easier to grok than the alternatives which use more registers.
Also, it is perfect for FW32 instruction encoding with simple decoding.
Also, surprisingly, it actually has rather good performance:

  # This countdown is only 1.4 seconds
  lit    0xffffff
  litm 0x7f0000
  toz
  looping:
    repeat looping

  # This countdown is also only 1.4 seconds
  lit    0xffffff
  litm 0x7f0000
  looping:
    sub by 1
    jmpgz looping

  # This countdown is only 2.8 seconds
  lit    0xffffff
  litm 0x7f0000
  looping:
    toz
    fromz
    sub by 1
    jmpgz looping

  # This countdown is only 4.8 seconds
  lit    0xffffff
  litm 0x7f0000
  looping:
    to 0x100
    from 0x100
    sub by 1
    jmpgz looping

Performance is a lesser consideration than simplicity and portability.

Notes: This is an experiment along the lines of srm.s but even simpler.
It attemps to be about an order of magnitude simpler than FVM 2.0.
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

Note:

  - Simple yet useful and practical
  - Designed for extreme ease of porting
  - Harvard architecture (here an advantage not a disadvantage)
  - Program space is essentially just a bunch of standard macros and:
      - can use any suitably capable architecture
      - can use variable-width instructions
      - can use any word size
      - can be of any size
      - can be native
  - Program space has about 32+ instruction macros
  - Many of these support multiple addressing modes
  - Data space has 32-bit words and a defined architecture
  - Data space has a standard maximum size but is allowed to be smaller
  - Out-of-bounds memory access of data space causes runtime exception
  - Otherwise robust: there are no other runtime exceptions
  - Designed for virtualizing other virtual machines
  - Can easily virtualize itself

Simplicity:

  - Arguably this is still a little heavy
  - However, it is just a set of macros easily ported
  - An underlying MISC CPU could be used beneath these macros
    - Yes, but doing so isn't relevant or worthwhile here
  - These macros are a sweet spot between simplicity and performance
  - Consider pseudo-stack direction

Other:

  - For now, multiply and divide are not included (to be reconsidered)
  - TODO change from byte- to word-addressing

Assessment:

  - This 'tvm.s' is again the front-runner for implementing FVM 2.0
    as of 20170729

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
.equ vZ, %esi # buffer register, also used for repeat
# Registers of the implementation:
.equ rTmp, %eax # primary temporary register
.equ rBuf, %ecx # secondary temporary register

# ============================================================================
# ==================== START OF PLATFORM-SPECIFIC CODE =======================
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

.macro do_swap reg1 reg2
  movl \reg1, rBuf
  movl \reg2, \reg1
  movl rBuf, \reg2
.endm

.macro i_swapAB
  do_swap vA vB
.endm

.macro i_swapAL
  do_swap vA vL
.endm

.macro i_swapAZ
  do_swap vA vZ
.endm

.macro i_toz
  movl vA, vZ
.endm

.macro i_fromz
  movl vZ, vA
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

.macro do_init
  xorl vA, vA
  xorl vB, vB
  xorl vL, vL
  xorl vZ, vZ
  xorl rTmp, rTmp
  xorl rBuf, rBuf
.endm

/*
# FIXME Not robust! Not suitable here!?
# But then we should have some kind of relative jump?
.macro i_jmpr baseAddr
  leal \baseAddr(,vA,WORD_SIZE), %eax
  jmp *(%eax)
.endm
*/

.macro i_jump label
  jmp \label
.endm

.macro i_jmpz label
  xorl $0, vA
  jz \label
.endm

.macro i_jmpnz label
  xorl $0, vA
  jnz \label
.endm

.macro i_djmpgz label
  dec vZ
  xorl $0, vZ
  jg \label
.endm

.macro i_jmpgz label
  xorl $0, vA
  jg \label
.endm

.macro i_jmplz label
  xorl $0, vA
  jl \label
.endm

.macro i_jmpgez label
  xorl $0, vA
  jge \label
.endm

.macro i_jmplez label
  xorl $0, vA
  jle \label
.endm

# ============================================================================
# ====================== END OF PLATFORM-SPECIFIC CODE =======================
# ============================================================================
.macro reg_at x reg
  reg_imm \x rTmp
  reg_load rTmp \reg
.endm

.macro reg_ptr_load x reg
  reg_at x rTmp
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
.macro by x
  reg_imm \x vB
.endm

.macro byx x
  reg_sign_extend \x vB
.endm

.macro bym x
  reg_m \x vB
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
.macro add by x
  \by \x
  i_add
.endm

.macro sub by x
  \by \x
  i_sub
.endm
# ----------------------------------------------------------------------------
.macro or by x
  \by \x
  i_or
.endm

.macro and by x
  \by \x
  i_and
.endm

.macro xor by x
  \by \x
  i_xor
.endm

.macro shl by x
  \by \x
  i_shl
.endm

.macro shr by x
  \by \x
  i_shr
.endm
# ----------------------------------------------------------------------------
.macro jmpr baseAddr
  i_jmpr \baseAddr
.endm

.macro jump label
  i_jump \label
.endm

.macro jmpz label
  i_jmpz \label
.endm

.macro jmpnz label
  i_jmpnz \label
.endm

.macro jmpgz label
  i_jmpgz \label
.endm

.macro jmplz label
  i_jmplz \label
.endm

.macro jmplez label
  i_jmplez \label
.endm

.macro jmpgez label
  i_jmpgez \label
.endm

.macro repeat label
  i_djmpgz \label
.endm
# ----------------------------------------------------------------------------
.macro branch label
  i_branch \label
.endm

.macro merge
  i_merge
.endm
# ----------------------------------------------------------------------------
.macro swapAB
  i_swapAB
.endm

.macro swapAL
  i_swapAL
.endm

.macro swapAZ
  i_swapAZ
.endm

.macro toz
  i_toz
.endm

.macro fromz
  i_fromz
.endm

.macro noop
  i_nop
.endm

.macro halt by x
  \by \x
  i_halt
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
#         The example shall virtualize this VM within itself!
#     Labels starting with vm_ are used by the parent native VM.
#     Labels starting with v_ are used by the child virtualized VM.
#     The child shall use fixed-width 32-bit instructions (FW32).
# ============================================================================
.equ v_data_memory, 0
.equ v_DM_BYTES, 0x100000 # Smaller than parent DM_BYTES by arbitrary amount
.equ v_rPC, v_data_memory + DM_BYTES # Note: parent has no explicit rPC
.equ v_vA, v_rPC + WORD_SIZE
.equ v_vB, v_vA + WORD_SIZE
.equ v_vL, v_vB + WORD_SIZE
.equ v_vZ, v_vL + WORD_SIZE
# Just using arbitrary opcode designations for now:
.equ v_LIT,   0x010000
.equ v_HALT,  0x1f0000

# ============================================================================
#                     ENTRY POINT
# ============================================================================
.global main
main:
vm_init: # parent
  do_init

vm_load_program_for_child:
  # lit 0x123456 = 0x01123456
  lit 0x123456
  to 0
  litm v_LIT
  to 0

  # halt by 0 = 0x1f000000
  lit 0x000000
  to 4
  litm v_HALT
  to 4

v_init:  # child
  lit 0
  to v_rPC
  to v_vA
  to v_vB
  to v_vL
  to v_vZ

  sub by 1

end:
  halt by 0

# ============================================================================

