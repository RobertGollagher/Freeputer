/*
                      SINGLE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170709
Updated:    20170715+
Version:    pre-alpha-0.0.0.5 for FVM 2.0


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
.equ MM_BYTES, 0x1000000
.equ vA, %ebx
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

  LATEST THOUGHTS:   ?sign, sign-extending, pc at runtime for return stack

  - FW32
  - word-addressing, 32-bit address space
  - 1 bit: reserved
  - 1 bit: instruction format (regular, jump)
  - 2 bits: mode (imm|@,@@,@@++,--@@) (not orthogonal)
  - 4 bits: opcode: (all one-directional M->R)
      - lit, from, to = 3
      - add, sub, mul, div = 4
      - shl, shr, and, or, xor = 5
      - reserved1, reserved2, nop, halt = 3
  - jumps: always absolute
      - jmp, jz, jnz, jlz, jgz, jo = 6 (maybe add <= and >=)(maybe decleq)
      - note: 10 spare instructions here, rethink
  - 24 bits: metadata

  PREFERRED:

  - bit-30 overflow solutions and 15-bit multiply, not sure div ?? carry, overflow

*/

# ----------------------------------------------------------------------------
#                                STORE MACROS
# ----------------------------------------------------------------------------
# @x = vA
.macro dst x
  movl $\x, rA
  movl vA, memory(,rA,1)
.endm

# @@x = vA
.macro dst_at x
  movl $\x, rA
  movl memory(,rA,1), rA
  movl vA, memory(,rA,1)
.endm

# @@x++ = vA
.macro dst_pp x
  movl $\x, rC
  movl memory(,rC,1), rA
  movl vA, memory(,rA,1)
  addl $4, memory(,rC,1)
.endm

# --@@x = vA
.macro dst_mm x
  movl $\x, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl vA, memory(,rC,1)
.endm
# ----------------------------------------------------------------------------
#                                 LOAD MACROS
# ----------------------------------------------------------------------------
# This may be used after each of the @ below
.macro rA_mem_vA
  movl memory(,rA,1), vA
.endm

# This may be used after each of the @@ below
.macro rA_mem_at_rA
  movl memory(,rA,1), rA
  movl memory(,rA,1), rA
.endm

# vA = @x when followed by rA_mem_vA
.macro src x
  movl $\x, rA
.endm

# vA = @@x when followed by rA_mem_vA
.macro src_at x
  movl $\x, rA
  movl memory(,rA,1), rA
.endm

# vA = @@x++ when followed by rA_mem_vA
.macro src_pp x
  movl $\x, rC
  movl memory(,rC,1), rA
  addl $4, memory(,rC,1)
.endm

# vA = --@@x when followed by rA_mem_vA
.macro src_mm x
  movl $\x, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
.endm
# ----------------------------------------------------------------------------
#                                ARITHMETIC MACROS   TODO /-1
# ----------------------------------------------------------------------------
.macro divide metadata
  movl vA, %eax
  movl $\metadata, %ebx

  test %ebx, %ebx
  je 1f

  cdq             # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx      # %edx:eax is the implied dividend
  jmp 2f
  1: # Division by zero
    movl $0, vA
  2:
.endm
# ----------------------------------------------------------------------------
#                                   JUMP MACROS
# ----------------------------------------------------------------------------
.macro indirectJump
  jmp rA
.endm

.macro doJump
  indirectJump
.endm

.macro doJmpo
  andl vA, $0x80000000
  jz positive
    andl vA, $0x40000000
    jnz ok
      indirectJump
  positive:
    andl vA, $0x40000000
    jz ok
      indirectJump \label
  ok:
.endm

.macro doJmpz
  xorl $0, vA
  jnz 1f
    indirectJump
  1:
.endm

.macro doJmpnz
  xorl $0, vA
  jz 1f
    indirectJump
  1:
.endm

.macro doJmplt
  cmp $0, vA
  jge 1f
    indirectJump
  1:
.endm

.macro doJmpgt
  cmp $0, vA
  jle 1f
    indirectJump
  1:
.endm
# ----------------------------------------------------------------------------
#                             MOVE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro lit metadata
  movl $\metadata, vA
.endm
# ----------------------------------------------------------------------------
.macro from metadata
  src \metadata
  rA_mem_vA
.endm

.macro from_at metadata
  src_at \metadata
  rA_mem_vA
.endm

.macro from_pp metadata
  src_pp \metadata
  rA_mem_vA
.endm

.macro from_mm metadata
  src_mm \metadata
  rA_mem_vA
.endm
# ----------------------------------------------------------------------------
.macro to metadata
  dst \metadata
.endm

.macro to_at metadata
  dst_at \metadata
.endm

.macro to_pp metadata
  dst_pp \metadata
.endm

.macro to_mm metadata
  dst_mm \metadata
.endm
# ----------------------------------------------------------------------------
#                           ARITHMETIC INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro add metadata
  addl $\metadata, vA
.endm

.macro add_at metadata
  src_at \metadata
  addl rA, vA
.endm

.macro add_pp metadata
  src_pp \metadata
  addl rA, vA
.endm

.macro add_mm metadata
  src_mm \metadata
  addl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro sub metadata
  subl $\metadata, vA
.endm

.macro sub_at metadata
  src_at \metadata
  subl rA, vA
.endm

.macro sub_pp metadata
  src_pp \metadata
  subl rA, vA
.endm

.macro sub_mm metadata
  src_mm \metadata
  subl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro mul metadata
  mull $\metadata, vA
.endm

.macro mul_at metadata
  src_at \metadata
  mull rA, vA
.endm

.macro mul_pp metadata
  src_pp \metadata
  mull rA, vA
.endm

.macro mul_mm metadata
  src_mm \metadata
  mull rA, vA
.endm
# ----------------------------------------------------------------------------
.macro div metadata
  divide \metadata
.endm

.macro div_at metadata
  src_at \metadata
  rA_mem_vA
  divide \metadata
.endm

.macro div_pp metadata
  src_pp \metadata
  rA_mem_vA
  divide \metadata
.endm

.macro div_mm metadata
  src_mm \metadata
  rA_mem_vA
  divide \metadata
.endm
# ----------------------------------------------------------------------------
#                               BITWISE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro or metadata
  orl $\metadata, vA
.endm

.macro or_at metadata
  src_at \metadata
  orl rA, vA
.endm

.macro or_pp metadata
  src_pp \metadata
  orl rA, vA
.endm

.macro or_mm metadata
  src_mm \metadata
  orl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro and metadata
  andl $\metadata, vA
.endm

.macro and_at metadata
  src_at \metadata
  andl rA, vA
.endm

.macro and_pp metadata
  src_pp \metadata
  andl rA, vA
.endm

.macro and_mm metadata
  src_mm \metadata
  andl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro xor metadata
  xorl $\metadata, vA
.endm

.macro xor_at metadata
  src_at \metadata
  xorl rA, vA
.endm

.macro xor_pp metadata
  src_pp \metadata
  xorl rA, vA
.endm

.macro xor_mm metadata
  src_mm \metadata
  xorl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro shl metadata
  movl $\metadata, rA
  movl rA, %ecx
  shll %cl, vA
.endm

.macro shl_at metadata
  src_at \metadata
  movl rA, %ecx
  shll %cl, vA
.endm

.macro shl_pp metadata
  src_pp \metadata
  movl rA, %ecx
  shll %cl, vA
.endm

.macro shl_mm metadata
  src_mm \metadata
  movl rA, %ecx
  shll %cl, vA
.endm
# ----------------------------------------------------------------------------
.macro shr metadata
  movl $\metadata, rA
  movl rA, %ecx
  shrl %cl, vA
.endm

.macro shr_at metadata
  src_at \metadata
  movl rA, %ecx
  shrl %cl, vA
.endm

.macro shr_pp metadata
  src_pp \metadata
  movl rA, %ecx
  shrl %cl, vA
.endm

.macro shr_mm metadata
  src_mm \metadata
  movl rA, %ecx
  shrl %cl, vA
.endm
# ----------------------------------------------------------------------------
#                   JUMP INSTRUCTIONS maybe decleq
# ----------------------------------------------------------------------------
.macro jump label
  leal \label, rA
  doJump
.endm

.macro jmpo label
  leal \label, rA
  doJmpo
.endm

.macro jmpz label
  leal \label, rA
  doJmpz
.endm

.macro jmpnz label
  leal \label, rA
  doJmpnz
.endm

.macro jmplz label
  leal \label, rA
  doJmplt
.endm

.macro jmpgz label
  leal \label, rA
  doJmpgt
.endm
# ----------------------------------------------------------------------------
.macro jump_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJump
.endm

.macro jmpo_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJmpo
.endm

.macro jmpz_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJmpz
.endm

.macro jmpnz_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJmpnz
.endm

.macro jmplz_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJmplt
.endm

.macro jmpgz_at metadata
  src_at \metadata
  rA_mem_at_rA
  doJmpgt
.endm
# ----------------------------------------------------------------------------
.macro jump_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJump
.endm

.macro jmpo_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJmpo
.endm

.macro jmpz_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJmpz
.endm

.macro jmpnz_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJmpnz
.endm

.macro jmplz_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJmplt
.endm

.macro jmpgz_pp metadata
  src_pp \metadata
  rA_mem_at_rA
  doJmpgt
.endm
# ----------------------------------------------------------------------------
.macro jump_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJump
.endm

.macro jmpo_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJmpo
.endm

.macro jmpz_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJmpz
.endm

.macro jmpnz_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJmpnz
.endm

.macro jmplz_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJmplt
.endm

.macro jmpgz_mm metadata
  src_mm \metadata
  rA_mem_at_rA
  doJmpgt
.endm
# ----------------------------------------------------------------------------
#                            OTHER INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro nop
.endm

.macro reserved1 metadata
  movl $\metadata, %eax
  jmp vm_illegal
.endm

.macro reserved2 metadata
  movl $\metadata, %eax
  jmp vm_illegal
.endm

.macro halt metadata
  movl $\metadata, %eax
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
.equ d, 0x0c
.equ base, 0x10
.equ top,  0x1c
.equ ptr, 0x20
.equ rsp, 0x24
.equ rs_base, 0x30

# ============================================================================
.section .text #                EXIT POINTS
# ============================================================================
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
#                     ENTRY POINT for an example program
# ============================================================================
.global main
main:

  lit rs_base
  to rsp

  lit base
  to ptr

  lit 1f
  to_pp rsp
  jump litMaxInt
  1:

  to_at ptr

  // Native execution time is about 3 s for 0x7fffffff iterations
  countdown:
    from base
    sub 1
    to base
    jmpgz countdown

  end:
    halt 0

  litMaxInt:
/*
    lit 0x7fffff
    shl 0x8
    or 0xff
*/
    lit 1
    jump_mm rsp

# ============================================================================
