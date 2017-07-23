/*
                      SPARSE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    miscvm1
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170723
Updated:    20170723+
Version:    pre-alpha-0.0.0.4+ for FVM 2.0

Notes: This is a MISC experiment which takes 'tvm.s' as its starting point
and attempts to further simplify that by another order of magnitude.
This 'miscvm1.s' supersedes 'tvm.s' and the earlier 'srm.s'.


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
  - 8,32=8 or 42 or very wasteful encoding 32,32=32 or 64 similar to FVM 1
  - not 16 bit, must be 32 bit

  DECISION TIME:

  - Therefore it is this with 8,32 or 32,32 or it is tvm.s with 32
  - This miscvm1.s has simpler VM implementation but more complex binaries
  - Whereas tvm.s has more complex VM implementation but simpler binaries

  DECISION:

  - In principal this miscvm1.s is better because:
      - tvm.s will lead to a weird no-man's land of addresses above 0xffffff
      - tvm.s is more tiring to port
  - Thus miscvm1.s is hereby chosen as the basis for further work

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
.equ vL, %edx # was %eax # link register
.equ vS, %esi # source address register
.equ vD, %edi # destination address register
.equ vP, %esp # stack pointer
# Registers of the implementation:
.equ rTmp, %ecx # temporary register (must be %ecx because of shl, shr)

# ============================================================================
# ============================================================================
.macro OUTCHAR reg
  do_save_vm_sp
  do_restore_sys_sp
  pushl \reg
  call putchar
  addl $WORD_SIZE, %esp
  do_save_sys_sp
  do_restore_vm_sp
.endm

.macro INCHAR #FIXME
  do_save_vm_sp
  do_restore_sys_sp
  call getchar
  movl %eax, vA
  do_save_sys_sp
  do_restore_vm_sp
.endm

.macro do_save_sys_sp
  movl %esp, sys_sp
.endm

.macro do_restore_sys_sp
  movl sys_sp, %esp
.endm

.macro do_save_vm_sp
  movl %esp, vm_sp
.endm

.macro do_restore_vm_sp
  movl vm_sp, %esp
.endm

.macro vm_init
  do_save_sys_sp
  xorl vA, vA
  xorl vB, vB
  xorl vL, vL
  xorl vD, vD
  xorl vS, vS
  leal data_memory, vP
  xorl rTmp, rTmp
.endm

# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
.macro lit x
  movl $\x, vA
.endm

.macro op x
  movl $\x, vB
.endm

.macro src x
  movl $\x, vS
.endm

.macro dst x
  movl $\x, vD
.endm

.macro link x
  movl $\x, vD
.endm
# ----------------------------------------------------------------------------
.macro load
  movl data_memory(,vS,1), vA
.endm

.macro store
  movl vA, data_memory(,vD,1)
.endm

.macro decs
  subl $WORD_SIZE, vS
.endm

.macro incs
  addl $WORD_SIZE, vS
.endm

.macro decd
  subl $WORD_SIZE, vD
.endm

.macro incd
  addl $WORD_SIZE, vD
.endm

.macro pull
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
.endm

.macro pull_pp
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
  incs
.endm

.macro pull_mm
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
  decs
.endm

.macro pp_pull
  incs
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
  incs
.endm

.macro mm_pull
  decs
  movl data_memory(,vS,1), rTmp
  movl data_memory(,rTmp,1), vA
  decs
.endm

.macro put
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
.endm

.macro put_pp
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
  incd
.endm

.macro put_mm
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
  decd
.endm

.macro pp_put
  incd
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
.endm

.macro mm_put
  decd
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
.endm

# FIXME think about all this some more
.macro pushsd
  decs
  decd
  put
.endm

.macro popsd
  incd
  incs
  pull
.endm

#FIXME native speed but not robust
.macro push
  pushl vA
.endm

.macro pop
  popl vA
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
.macro do label
  leal 1f, vL
  jump \label
  1:
.endm

.macro done
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
# ----------------------------------------------------------------------------
.macro halt
  movl vB, %eax
  jmp vm_success
.endm

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
.lcomm data_memory, DM_BYTES
.lcomm sys_sp, WORD_SIZE
.lcomm vm_sp, WORD_SIZE

# ============================================================================
.section .text #             EXIT POINTS for the VM
# ============================================================================
vm_failure:

  do_restore_sys_sp
  movl $FAILURE, rTmp
  ret

vm_success:

  do_restore_sys_sp
  movl $SUCCESS, rTmp
  ret

# ============================================================================
#                             HANDY SUBROUTINES
#    These are perhaps candidates to be added to the instruction set.
# ============================================================================


# ============================================================================
#                        PARENT SUBROUTINES AND MACROS
#  These are not part of the child virtualized VM. They belong to the parent.
# ============================================================================
.macro vm_load_program_for_child
  dst 0
  lit v_LIT + 0x123456
  put_pp
  lit v_HALT
  put_pp
.endm
# ============================================================================
# ========================= EXAMPLE PROGRAM ==================================
#         The example shall virtualize this VM within itself!
#     Labels starting with vm_ are used by the parent native VM.
#     Labels starting with v_ are used by the child virtualized VM.
#     The child shall use ???-wide instructions (two formats?).
# ============================================================================
.equ v_data_memory, 0
.equ v_DM_BYTES, 0x100000 # Smaller than parent DM_BYTES by arbitrary amount
.equ v_rPC, v_data_memory + v_DM_BYTES # Note: parent has no explicit rPC
.equ v_vA, v_rPC + WORD_SIZE
.equ v_vB, v_vA + WORD_SIZE
.equ v_vL, v_vB + WORD_SIZE
.equ v_vS, v_vL + WORD_SIZE
.equ v_vD, v_vS + WORD_SIZE
.equ v_vP, v_vD + WORD_SIZE
.equ v_vTmp, v_vP + WORD_SIZE
.equ v_instr, v_vTmp + WORD_SIZE
.equ v_opcode, v_instr + WORD_SIZE
.equ v_metadata, v_opcode + WORD_SIZE
# Just using arbitrary opcode designations for now:
.equ v_LIT,   0x01000000
.equ v_HALT,  0x1f000000

# ============================================================================
#                           CHILD SUBROUTINES
# ============================================================================
v_clear_regs:
  lit 0
  dst v_rPC
  store
  dst v_vA
  store
  dst v_vB
  store
  dst v_vL
  store
  dst v_vS
  store
  lit v_vD
  store
  lit v_vP
  store
  done

# ============================================================================
#                             ENTRY POINT
# ============================================================================
.global main
main:
  vm_init
  vm_load_program_for_child
v_init:
  do v_clear_regs
v_next:
  src v_rPC
  load
  dst v_instr
  store
  
# TODO NEXT: 1. vector table for opcodes; 2. reconsider 24/32

end:
  halt


# ============================================================================

