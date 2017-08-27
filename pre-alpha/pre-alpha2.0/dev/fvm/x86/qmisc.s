/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)


Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170826
Updated:    20170827+
Version:    pre-alpha-0.0.0.14 for FVM 2.0
=======

                              This Edition:
                          x86 Assembly Language
                           using GNU Assembler

                               ( ) [ ] { }

 See 'qmisc.c'. This an x86 implementation of that earlier proof of concept.
 Current work is focussed on optimizing self-virtualization.

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
.equiv TRACING_ENABLED, 1           # 0 = true,   1 = false
.equiv LINKING_WITH_LD_ON_LINUX, 0  # 0 = true,   1 = false
.equiv x86_64, 1                    # 0 = x86-64, 1 = x86-32

.equ WD_BYTES, 4
.equ ONES,          0xffffffff
.equ MSb,           0x80000000  # Bit mask for most significant bit
.equ METADATA_MASK, 0x7fffffff  # 31 bits
.equ BYTE_MASK,     0x000000ff
.equ SHIFT_MASK,    0x0000001f
.equ SUCCESS, 0
.equ FAILURE, 1
.equ ILLEGAL, 2
.equ MAX_DM_WORDS,  0x10000000  # Must be 2^(WD_BITS-4) due to C limitations.
.equ DM_WORDS, MAX_DM_WORDS     # Must be some power of 2 <= MAX_DM_WORDS.
.equ DM_BYTES, DM_WORDS*WD_BYTES
.equ DM_MASK, DM_WORDS-1
# There are only 4 accessible registers:
.equ vA, %eax; # accumulator
.equ vB, %ebx; # operand register
.equ vT, %edx; # temporary register
.equ vR, %esi; # repeat register
.ifeq x86_64
  .equ vL, %rdi; # link register (not accessible)
.else
  .equ vL, %edi; # link register (not accessible)
.endif
.equ rSwap, %ecx; # swap register (not accessible) (sometimes reused here)
.equ rShift, %cl; # register used for shift magnitude (not accessible)
# ============================================================================
#                                IMPORTS
# ============================================================================
.ifeq TRACING_ENABLED
  .extern printf
.endif
# ============================================================================
# ==================== START OF PLATFORM-SPECIFIC CODE =======================
# ============================================================================
.macro do_exit reg_vA
  .ifeq TRACING_ENABLED
    TRACE_PAR
  .endif
  .ifeq LINKING_WITH_LD_ON_LINUX
    movl \reg_vA, %ebx          # Exit code (status)
    movl $0x1, %eax             # Linux call ID for exit
    int $0x80                   # Linux interrupt for system call
  .else
    movl \reg_vA, %eax         # Exit code (status)
    ret
  .endif
.endm
.macro do_success
  movl $SUCCESS, vA
  do_exit vA
.endm
.macro do_failure
  movl $FAILURE, vA
  do_exit vA
.endm
.macro do_illegal
  movl $ILLEGAL, vA
  do_exit vA
.endm
.macro do_init
  xorl vA, vA
  xorl vB, vB
  xorl vT, vT
  xorl vR, vR
.ifeq x86_64
  xorq vL, vL
.else
  xorl vL, vL
.endif
  xorl rSwap, rSwap
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
.endm
.macro i_get
  movl vB, rSwap
  andl $DM_MASK, rSwap
  movl data_memory(,rSwap,WD_BYTES), vA
.endm
.macro i_put
  movl vB, rSwap
  andl $DM_MASK, rSwap
  movl vA, data_memory(,rSwap,WD_BYTES)
.endm
.macro i_at
  movl vB, rSwap
  andl $DM_MASK, rSwap
  movl data_memory(,rSwap,WD_BYTES), vB
.endm
.macro XP_pop
  movl vB, vA
  andl $DM_MASK, vA
  movl vA, rSwap
  movl data_memory(,vA,WD_BYTES), vA
  andl $DM_MASK, vA
  movl data_memory(,vA,WD_BYTES), vA
  incl data_memory(,rSwap,WD_BYTES)
.endm
.macro i_inc
  incl vB
.endm
.macro i_dec
  decl vB
.endm
.macro i_imm x
  movl $\x, vB
  # TODO opzn: assume there is a compile-time check limiting x to 31 bits
  #andl $METADATA_MASK, vB #opzn
.endm
.macro i_flip
  xorl $MSb, vB
.endm
.macro i_swap
  movl vA, rSwap
  movl vB, vA
  movl rSwap, vB
.endm
.macro i_tob
  movl vA, vB
.endm
.macro i_tot
  movl vA, vT
.endm
.macro i_tor
  movl vA, vR
.endm
.macro i_fromb
  movl vB, vA
.endm
.macro i_fromt
  movl vT, vA
.endm
.macro i_fromr
  movl vR, vA
.endm
.macro i_mdm
  movl $DM_WORDS, vA
.endm
.macro i_noop
  nop
.endm
.macro i_halt
  andl $BYTE_MASK, vA
  do_exit vA
.endm
.macro i_jmpe label
  cmpl vB, vA
  jne 1f
    jmp \label
  1:
.endm
.macro i_jump label
  jmp \label
.endm
.macro i_rpt label
  decl vR
  cmpl $ONES, vR
  jz 1f
.ifeq x86_64
  leaq \label, %r8
  jmp *%r8
.else
  leal \label, rSwap
  jmp *rSwap
.endif
  1:
.endm
.macro i_br label
.ifeq x86_64
  leaq 1f, vL
.else
  leal 1f, vL
.endif
  jmp \label
  1:
.endm
.macro i_link label
  jmp *vL
.endm
# ============================================================================
# ====================== END OF PLATFORM-SPECIFIC CODE =======================
# ============================================================================
# ============================================================================
.section .data #                CONSTANTS
# ============================================================================
.ifeq TRACING_ENABLED
  pntfmt: .asciz "\nPARENT: vA:%08x vB:%08x vT:%08x vR:%08x "
  chdfmt_parta: .asciz "\n%08x "
  chdfmt_partb: .asciz "%08x CHILD: vA:%08x vB:%08x vT:%08x vR:%08x vL:%08x "
  format_hex8: .asciz "%08x"
  newline: .asciz "\n"
  space: .asciz " "
.endif
# ============================================================================
#                            INSTRUCTION SET
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
.macro i x
  i_imm \x
.endm
.macro flip
  i_flip
.endm
.macro swap
  i_swap
.endm
.macro tob
  i_tob
.endm
.macro tor
  i_tor
.endm
.macro tot
  i_tot
.endm
.macro fromb
  i_fromb
.endm
.macro fromt
  i_fromt
.endm
.macro fromr
  i_fromr
.endm
.macro mdm
  i_mdm
.endm
.macro noop
  i_noop
.endm
.macro halt
  i_halt
.endm
.macro jmpe label
  i_jmpe \label
.endm
.macro jump label
  i_jump \label
.endm
.macro rpt label
  i_rpt \label
.endm
.macro br label
  i_br \label
.endm
.macro link
  i_link
.endm
# ============================================================================
.section .bss #                  VARIABLES
# ============================================================================
data_memory: .lcomm dm, DM_BYTES
# ============================================================================
#                                 TRACING
# ============================================================================
.ifeq TRACING_ENABLED
  .macro TRACE_PAR
    SAVE_REGS
    pushl vR
    pushl vT
    pushl vB
    pushl vA
    pushl $pntfmt
    call printf
    addl $20, %esp
    RESTORE_REGS
  .endm

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

  .macro PUSH_DM_VAL dm_addr
    movl $\dm_addr, rSwap
    andl $DM_MASK, rSwap
    movl data_memory(,rSwap,WD_BYTES), rSwap
    pushl rSwap
  .endm
.endif
# ============================================================================
.section .text #           EXIT POINTS for the VM
# ============================================================================
vm_success:
  do_success
vm_failure:
  do_failure
vm_illegal:
  do_illegal
# =========================== EXAMPLE PROGRAM ================================
# Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
# ============================================================================
# ===========================================================================
# Opcodes for interpreter of child VM (mostly arbitrary values for now).
# Current scheme is FW32 (poor density but simple, portable).
# These could be better optimized by grouping (interpreter vs FPGA...).
.equ iNOOP,  0x00000000 # not arbitrary, must be 0x00000000
#.equ iIMM,  0x80000000 # not arbitrary, must be 0x80000000

# Below 0x40000000 = simple
.equ iADD,  0x01000000
.equ iSUB,  0x02000000
.equ iOR,   0x03000000
.equ iAND,  0x04000000
.equ iXOR,  0x05000000
.equ iSHL,  0x08000000
.equ iSHR,  0x09000000
.equ iGET,  0x10000000
.equ iPUT,  0x11000000
.equ iAT,   0x12000000
.equ iINC,  0x20000000
.equ iDEC,  0x21000000
.equ iFLIP, 0x22000000
.equ iSWAP, 0x23000000
.equ iTOB,  0x30000000
.equ iTOR,  0x31000000
.equ iTOT,  0x32000000
.equ iFROMB,0x33000000
.equ iFROMR,0x34000000
.equ iFROMT,0x35000000

.equ iMDM,  0x36000000
.equ iLINK, 0x37000000

.equ iHALT, 0x3f000000

# Above 0x40000000 = complex
.equ COMPLEX_MASK,0x40000000

.equ iJMPE, 0x41000000
.equ iJUMP, 0x46000000
.equ iRPT,  0x50000000
.equ iBR,   0x61000000
# ===========================================================================
# ============================================================================
#                      ENTRY POINT FOR EXAMPLE PROGRAM
# ============================================================================
.ifeq LINKING_WITH_LD_ON_LINUX
  .global _start
  _start:
.else
  .global main
  main:
.endif

vm_pre_init:
  do_init

# For native parent VM speed comparison (1.4 secs for 0x7fffffff nop repeats):
#   (surprisingly, 'qmisc.c' takes 4 secs, so this is 3x faster)
#   (i.e. parent VM is faster but child VM is slower than the C version)
/*i(0x7fffffff)
fromb
tor
foo:
.ifeq TRACING_ENABLED
  TRACE_PAR
.endif
  noop
  rpt foo
jmp vm_success
*/

# ---------------------------------------------------------------------------
.equ vm_DM_WORDS, DM_WORDS
.equ v_DM_WORDS,  0x1000 # Must be a power of 2, so <= DM_WORDS/2
.equ v_DM_MASK, v_DM_WORDS-1
.equ v_PM_WORDS,  0x1000 # Must be a power of 2, so <= DM_WORDS/2
.equ v_PM_MASK, v_PM_WORDS-1
.equ v_pm, 0
.equ v_dm, v_PM_WORDS
.equ v_vZ, v_dm + v_DM_WORDS # child program counter
.equ v_vA, v_vZ + 1
.equ v_vB, v_vA + 1
.equ v_vL, v_vB + 1
.equ v_vT, v_vL + 1
.equ v_vR, v_vT + 1
.equ OPCODE_MASK,   0xff000000
.equ CELL_MASK,     0x00ffffff
# ---------------------------------------------------------------------------
.ifeq TRACING_ENABLED
  .macro TRACE_CHILD_partA
    SAVE_REGS
    PUSH_DM_VAL v_vZ
    pushl $chdfmt_parta
    call printf
    addl $8, %esp
    RESTORE_REGS
  .endm

  .macro TRACE_CHILD_partB
    SAVE_REGS
    PUSH_DM_VAL v_vL
    PUSH_DM_VAL v_vR
    PUSH_DM_VAL v_vT
    PUSH_DM_VAL v_vB
    PUSH_DM_VAL v_vA
    pushl vA
    pushl $chdfmt_partb
    call printf
    addl $28, %esp
    RESTORE_REGS
  .endm
.endif
# ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
# ---------------------------------------------------------------------------
# Process next instruction (not optimized yet)
nexti:

/*
  i(v_vZ)
  at

.ifeq TRACING_ENABLED
  TRACE_CHILD_PartA
.endif

  inc
  fromb
  i(v_PM_MASK)
  and
  i(v_vZ)
  put
  tob
  dec
  get
*/
#opzn

  i(v_vZ)
  XP_pop
  tot

.ifeq TRACING_ENABLED
  TRACE_CHILD_PartA # needs correction
  TRACE_CHILD_PartB
.endif

  i(0)
  flip
  and
  jmpe(v_Imm)

  fromt
  i(COMPLEX_MASK)
  and
  jmpe(v_complex_instrs)

  fromt
  i(OPCODE_MASK)
  and

      i(iNOOP)
        jmpe(v_Noop)
      i(iADD)
        jmpe(v_Add)
      i(iSUB)
        jmpe(v_Sub)
      i(iAND)
        jmpe(v_And)
      i(iOR)
        jmpe(v_Or)
      i(iXOR)
        jmpe(v_Xor)
      i(iSHL)
        jmpe(v_Shl)
      i(iSHR)
        jmpe(v_Shr)
      i(iGET)
        jmpe(v_Get)
      i(iPUT)
        jmpe(v_Put)
      i(iAT)
        jmpe(v_At)
      i(iINC)
        jmpe(v_Inc)
      i(iDEC)
        jmpe(v_Dec)
      i(iSWAP)
        jmpe(v_Swap)
      i(iTOB)
        jmpe(v_Tob)
      i(iTOR)
        jmpe(v_Tor)
      i(iTOT)
        jmpe(v_Tot)
      i(iFROMB)
        jmpe(v_Fromb)
      i(iFROMR)
        jmpe(v_Fromr)
      i(iFROMT)
        jmpe(v_Fromt)
      i(iMDM)
        jmpe(v_Mdm)
      i(iLINK)
        jmpe(v_Link)
      i(iHALT)
        jmpe(v_Halt)

    v_complex_instrs:
      fromt
      i(OPCODE_MASK)
      and

      i(iJMPE)
        jmpe(v_Jmpe)
      i(iJUMP)
        jmpe(v_Jump)
      i(iRPT)
        jmpe(v_Rpt)
      i(iBR)
        jmpe(v_Br)

    i(ILLEGAL)
      fromb
      halt
# ---------------------------------------------------------------------------
v_Add:
  i(v_vA)
  get
  i(v_vB)
  at
  add
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Sub:
  i(v_vA)
  get
  i(v_vB)
  at
  sub
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_And:
  i(v_vA)
  get
  i(v_vB)
  at
  and
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Or:
  i(v_vA)
  get
  i(v_vB)
  at
  or
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Xor:
  i(v_vA)
  get
  i(v_vB)
  at
  xor
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Shl:
  i(v_vA)
  get
  i(v_vB)
  at
  shl
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Shr:
  i(v_vA)
  get
  i(v_vB)
  at
  shr
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Get:
  i(v_vB)
  get
  i(v_DM_MASK)
  and
  i(v_dm)
  add
  tob
  get
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Put:
  i(v_vB)
  get
  i(v_DM_MASK)
  and
  i(v_dm)
  add
  i(v_vA)
  at
  swap
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_At:
  i(v_vB)
  get
  i(v_DM_MASK)
  and
  i(v_dm)
  add
  tob
  get
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Inc:
  i(v_vB)
  at
  inc
  fromb
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Dec:
  i(v_vB)
  at
  dec
  fromb
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Imm:
  fromt
  i(0)
  flip
  xor
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Flip:
  i(v_vB)
  at
  flip
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Swap:
  i(v_vA)
  get
  i(v_vB)
  put
  swap
  i(v_vB)
  get
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Tob:
  i(v_vA)
  get
  i(v_vB)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Tor:
  i(v_vA)
  get
  i(v_vR)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Tot:
  i(v_vA)
  get
  i(v_vT)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Fromb:
  i(v_vB)
  get
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Fromr:
  i(v_vR)
  get
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Fromt:
  i(v_vT)
  get
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Mdm:
  i(v_DM_WORDS)
  fromb
  i(v_vA)
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Noop:
  jump(nexti)
# ---------------------------------------------------------------------------
v_Jmpe:
  i(v_vA)
  get
  jmpe(v_Jmpe_do)
    jump(nexti)
  v_Jmpe_do:
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
# ---------------------------------------------------------------------------
v_Jump:
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
# ---------------------------------------------------------------------------
v_Br:
    i(v_vZ)
    get
    i(v_vL)
    put
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
# ---------------------------------------------------------------------------
v_Link:
    i(v_vL)
    get
    i(v_vZ)
    put
    jump(nexti)
# ---------------------------------------------------------------------------
v_Rpt:
  i(v_vR)
  at
  dec
  fromb
  i(v_vR)
  put
  i(0)
  dec
  jmpe(v_Repeat_end)
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
  v_Repeat_end:
    jump(nexti)
# ---------------------------------------------------------------------------
v_Halt:
  i(v_vA)
  get
  i(BYTE_MASK)
  and
  halt
# ---------------------------------------------------------------------------
# Program child's program memory then run program
program:
  i(0)
  fromb
  i(3)
  flip
  br(si)
  i(iADD)
  br(si)
  i(5)
  flip
  br(si)
  i(iADD)
  br(si)
  i(0x10000000) # Performance test (asm child does 0x7fffffff in about 30 sec)
  flip  # 2 0x10000000 0x7fffffff  (i.e. no weird gcc fu but 1.5-3.0x slower)
  br(si) #  (so perhaps an interpreted VM, rather than child, best for asm?)
  i(iFROMB)
  br(si)
  i(iTOR)
  br(si)
  i(iNOOP) # This is instruction 7 in this program.
  br(si)
  i(iRPT|7)
  br(si)
  i(iHALT)
  br(si)
  jump(nexti)
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# Store instruction to child's program memory
si:
  swap
  put
  inc
  fromb
  link
# ---------------------------------------------------------------------------
# Fill vR words at dm[vA] with value in vV (fills 1 GB in about 0.63 seconds)
doFill:
  doFillLoop:
    put
    inc
    rpt(doFillLoop)
    link
# ---------------------------------------------------------------------------
# Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  i(DM_WORDS)
  fromb
  tor
  i(0)
  fromb
  link
# ---------------------------------------------------------------------------
# Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  i(vm_DM_WORDS)
  jmpe(assertedParentSize)
    i(FAILURE)
    fromb
    halt
  assertedParentSize:
    link
# ---------------------------------------------------------------------------
# end of exampleProgram
# ===========================================================================
# ============================================================================


