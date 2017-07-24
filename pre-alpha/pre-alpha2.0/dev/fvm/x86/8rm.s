/*
                          8 REGISTER MACHINE (8RM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    miscvm1
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170723
Updated:    20170723
Version:    pre-alpha-0.0.0.0 for FVM 2.0

Notes: This '8rm.s' takes 'miscvm1.s' as its starting point.
This supersedes 'miscvm1.s', 'tvm.s' and 'srm.s'.

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
  BEST COMPROMISE:

    - Harvard architecture
    - Program space 24-bits max
    - Data space 32-bits max
    - FW32

  IDEA FOR FURTHER FACTORING AND SIMPLIFICATION:

    - Reduce to almost 1:1 relationship with typical 32-bit CPU instructions
    - This makes a JIT compiler or AOT compiler easy to write
    - Add a second stack pointer for various benefits
    - Accept that 8 non-orthogonal registers is the best compromise
    - This moves us away from sparse register to 8-register design
    - Rename 8rm instead of srm, use '8rm.s' instead of 'miscvm1.s'

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

.macro reg_imm x reg
  movl $\x, \reg
.endm

.macro reg_sign_extend x reg
  reg_imm \x \reg
  reg_imm 0x00800000 rTmp
  andl \reg, rTmp
  jz 1f
    orl $0xff000000, \reg
  1:
.endm

.macro reg_m x reg
  reg_imm \x rTmp
  shll $8, rTmp
  andl $0x00ffffff, \reg
  orl rTmp, \reg
.endm

# ============================================================================
.section .data #             INSTRUCTION SET
# ============================================================================
.macro lit x
  reg_imm \x vA
.endm

.macro litm x
  reg_m \x vA
.endm

.macro litx x
  reg_sign_extend \x vA
.endm

.macro op x
  reg_imm \x vA
.endm

.macro opm x
  reg_m \x vB
.endm

.macro opx x
  reg_sign_extend \x vB
.endm

.macro src x
  reg_imm \x vA
.endm

.macro srcm x
  reg_m \x vS
.endm

.macro srcx x
  reg_sign_extend \x vS
.endm

.macro dst x
  reg_imm \x vA
.endm

.macro dstm x
  reg_m \x vD
.endm

.macro dstx x
  reg_sign_extend \x vD
.endm

.macro link x
  reg_imm \x vA
.endm

.macro linkm x
  reg_m \x vL
.endm

.macro linkx x
  reg_sign_extend \x vL
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

.macro put
  movl data_memory(,vD,1), rTmp
  movl vA, data_memory(,rTmp,1)
.endm

#FIXME not robust
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
.macro ops opcode # Simple instruction
  lit 0x000000
  litm \opcode
  store
  incd
.endm

.macro opc opcode metadata # Complex instruction
  lit \metadata
  litm \opcode
  store
  incd
.endm

.macro vm_load_program_for_child
  dst 0
  opc v_LIT 0x123456
  ops v_HALT
.endm

# ============================================================================
# ========================= EXAMPLE PROGRAM ==================================
#         The example shall virtualize this VM within itself!
#     Labels starting with vm_ are used by the parent native VM.
#     Labels starting with v_ are used by the child virtualized VM.
#     The child shall use 32-bit-wide instructions (FW32).
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
.equ v_LIT,   0x010000
.equ v_HALT,  0x1f0000

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

# TODO NEXT: 1. vector table for opcodes

end:
  halt

v_opcodeTable:
  .equ v_NOP, 0
  .equ v_LIT, 1
  .equ v_HALT, 2
v_vectorTable:
  .long v_NOP
  .long v_LIT
  .long v_HALT
# ============================================================================

