/*
                      SINGLE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170709
Updated:    20170720+
Version:    pre-alpha-0.0.0.12 for FVM 2.0


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
.extern putchar

# ============================================================================
#                                SYMBOLS
# ============================================================================
.equ MM_BYTES, 0x1000000
.equ WORD_SIZE, 4
.equ vA, %ebx
.equ vX, %edx
.equ vL, %edi
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

  - This is a meta-machine. It runs natively on the target platform.
  - The meta-machine is effectively Harvard architecture.
  - The meta-machine is effectively a set of strictly standardized macros.
  - We do not care what the word size is of the meta-machine's hardware.
  - We do care what the word size of the data space is: always 32 bits.
  - Thus the meta-machine can be used either to:
      - run a previously prepared program (embedded environment); or
      - virtualize a machine in its data space!
  - Memory-access exceptions can occur since:
      - size of the data space is not standardized; and
      - this is for reasons of flexibility and portability.

  NEXT STEPS:

  - Therefore the next step is to virtualize this machine in itself.

  SOME FUNDAMENTALS:

  - FW32
  - word-addressing, 30-bit address space
  - 3 bits: mode (imm,@,@@,@@++,--@@) (not orthogonal)
  - 5 bits: opcode: (all one-directional M->R)
      - lit, litx, litm, by, byx, bym, from, to = 8
      - add, sub, mul, div = 4
      - shl, shr, and, or, xor = 5
      - halt = 1
      - xjmp, jmp, jz, jnz, jlz, jgz, jle, jge, jo = 9 (maybe decleq)
      - in, out = 2
      - a few spare
  - 24 bits: metadata

*/
# ----------------------------------------------------------------------------
#                   I/O INSTRUCTIONS just for fun for now
# ----------------------------------------------------------------------------
.macro in by port
  \by \port
  movl vX, rA
  orl $0x00000000, rA
  jnz 1f
    # Only port 0 supported for now = byte stdin
    call getchar
  1:
.endm

.macro out by port
  \by \port
  movl vX, rA
  andl $0x00000001, rA
  jz 1f
    # Only port 1 supported for now = byte stdout
    pushl vA
    call putchar
    addl $4, %esp
  1:
.endm
# ----------------------------------------------------------------------------
#                             MOVE INSTRUCTIONS
# ----------------------------------------------------------------------------
.macro lit metadata
  movl $\metadata, vA
.endm

.macro litx metadata
  movl $\metadata, vA
  movl $0x00800000, rC
  andl vA, rC
  jz 1f
    orl $0xff000000, vA
  1:
.endm

.macro litm metadata
  movl $\metadata, rA
  shll $8, rA
  andl $0x00ffffff, vA
  orl rA, vA
.endm
# ----------------------------------------------------------------------------
.macro from metadata
  movl $\metadata, rA
  movl memory(,rA,1), vA
.endm

.macro fromx metadata
  movl $\metadata, rA
  movl memory(,rA,1), vA
.endm

.macro from_ptr metadata
  movl $\metadata, rA
  movl memory(,rA,1), rA
  movl memory(,rA,1), vA
.endm

.macro from_ptr_pp metadata
  movl $\metadata, rC
  movl memory(,rC,1), rA
  addl $4, memory(,rC,1)
  movl memory(,rA,1), vA
.endm

.macro from_ptr_mm metadata
  movl $\metadata, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl memory(,rC,1), vA
.endm
# ----------------------------------------------------------------------------
.macro by metadata
  movl $\metadata, vX
.endm

.macro byx metadata
  movl $\metadata, vX
  movl $0x00800000, rC
  andl vX, rC
  jz 1f
    orl $0xff000000, vX
  1:
.endm

.macro bym metadata
  movl $\metadata, rA
  shll $8, rA
  andl $0x00ffffff, vX
  orl rA, vX
.endm

.macro by_at metadata
  movl $\metadata, rA
  movl memory(,rA,1), vX
.endm

.macro by_ptr metadata
  movl $\metadata, rA
  movl memory(,rA,1), rA
  movl memory(,rA,1), vX
.endm

.macro by_ptr_pp metadata
  movl $\metadata, rC
  movl memory(,rC,1), rA
  addl $4, memory(,rC,1)
  movl memory(,rA,1), vX
.endm

.macro by_ptr_mm metadata
  movl $\metadata, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl memory(,rC,1), vX
.endm
# ----------------------------------------------------------------------------
.macro to metadata
  movl $\metadata, rA
  movl vA, memory(,rA,1)
.endm

.macro to_ptr metadata
  movl $\metadata, rA
  movl memory(,rA,1), rA
  movl vA, memory(,rA,1)
.endm

.macro to_ptr_pp metadata
  movl $\metadata, rC
  movl memory(,rC,1), rA
  movl vA, memory(,rA,1)
  addl $4, memory(,rC,1)
.endm

.macro to_ptr_mm metadata
  movl $\metadata, rA
  movl memory(,rA,1), rC
  subl $4, rC
  movl rC, memory(,rA,1)
  movl vA, memory(,rC,1)
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
.macro mul by metadata
  \by \metadata
  mull vX, vA     # TODO consider limiting to 15-bit mul so cannot overflow
.endm
# ----------------------------------------------------------------------------
.macro div by metadata
  \by \metadata
  pushl vX        # untested
  movl vA, %eax
  movl vX, %ebx
  test %ebx, %ebx
  je 1f           # TODO consider what to do on /-1
  cdq             # MUST widen %eax here to %edx:eax or (neg) div wrong
  idivl %ebx      # %edx:eax is the implied dividend
  jmp 2f
  1: # Division by zero shall yield 0
    movl $0, vA
  2:
  movl %eax, vA
  popl vX
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
.macro jumpx by metadata
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
  jmp vm_exit
.endm

# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
# For the meta-machine
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
.section .text #             EXIT POINTS for the VM
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
# ============================================================================
# ========================= EXAMPLE PROGRAM ==================================
# ============================================================================
# ============================================================================
#                 EXAMPLE VARIABLES for an example program
# ============================================================================
.equ v_memory, 0
.equ rsp, v_memory + WORD_SIZE
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

  init:

    lit 0
    to v_vA

    lit 0
    to v_vX

    lit 0
    to v_pc

  loadProg:

    lit 1
    to 0

    lit 2
    to 4

    lit 3
    to 8

    lit 0
    to 12

  next:
    from_ptr_pp v_pc
    to instr
    swap

    lit 1
    xor


    jmpz illeg

    jmp next

  illeg:
    halt by 1

  end:
    halt by 0


# ============================================================================
