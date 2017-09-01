/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)


Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170826
Updated:    20170901+
Version:    pre-alpha-0.0.0.23+ for FVM 2.0
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
.equiv TRACING_ENABLED, 0           # 0 = true,   1 = false
.equiv LINKING_WITH_LD_ON_LINUX, 1  # 0 = true,   1 = false
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
.equ vB, %ecx; # operand register (update: using ecx to simplify shl, shr)
.equ vT, %edx; # temporary register
.equ vR, %esi; # repeat register
.ifeq x86_64
  .equ vL, %rdi; # link register (not accessible)
.else
  .equ vL, %edi; # link register (not accessible)
.endif
.equ rD, %ebx; # address register (not accessible, results from masking)
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
  xorl rD, rD
.endm
# ============================================================================
#                            INSTRUCTION SET
# ============================================================================
# ------------------------ New Instructions ----------------------------------
.macro asav
  movl vA, data_memory(,rD,WD_BYTES)
.endm
.macro bsav
  movl vB, data_memory(,rD,WD_BYTES)
.endm
.macro copy
  movl vB, rD
  andl $DM_MASK, rD
  movl data_memory(,rD,WD_BYTES), vB
  addl vA, rD
  andl $DM_MASK, rD
  movl vB, data_memory(,rD,WD_BYTES)
.endm
.macro jmpb label
  cmpl $0, vB
  jnz 1f
    jmp \label
  1:
.endm
# ------------------------ Nice Lean Instructions ----------------------------
.macro i x # Assume compile-time check limits x to 31 bits
  movl $\x, vB    # Idea: 16-bit... leverage?
.endm
.macro add
  addl vB, vA
.endm
.macro sub
  subl vB, vA
.endm
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
  shll rShift, vA # Requires vB and rShift to be based on %ecx
.endm
.macro shr
  shrl rShift, vA # Requires vB and rShift to be based on %ecx
.endm


# Experimentally these now cause vB to change
.macro get
  andl $DM_MASK, vB
  movl data_memory(,vB,WD_BYTES), vA
.endm
.macro put
  andl $DM_MASK, vB
  movl vA, data_memory(,vB,WD_BYTES)
.endm
.macro at
  andl $DM_MASK, vB
  movl vB, rD # only at changes vD (not get, put)
  movl data_memory(,vB,WD_BYTES), vB
.endm


.macro inc
  incl vB
.endm
.macro dec
  decl vB
.endm
.macro flip
  xorl $MSb, vB
.endm
.macro swap
.ifeq x86_64
  movl vA, %r8
  movl %r8, vA
  movl rSwap, vB
.else
  xorl vA, vB
  xorl vB, vA
  xorl vA, vB
.endif
.endm
.macro tob
  movl vA, vB
.endm
.macro tot
  movl vA, vT
.endm
.macro tor
  movl vA, vR
.endm
.macro fromb
  movl vB, vA
.endm
.macro fromt
  movl vT, vA
.endm
.macro fromr
  movl vR, vA
.endm
.macro mdm
  movl $DM_WORDS, vA
.endm
.macro noop
  nop
.endm
.macro halt
  andl $BYTE_MASK, vA # Reconsider
  do_exit vA
.endm
.macro jump label
  jmp \label
.endm
.macro jmpe label
  cmpl vB, vA
  jne 1f
    jmp \label
  1:
.endm
.macro br label
.ifeq x86_64
  leaq 1f, vL
.else
  leal 1f, vL
.endif
  jmp \label
  1:
.endm
.macro link label
  jmp *vL
.endm
.macro rpt label
  cmpl $0, vR
  jz 1f
  decl vR
  jmp \label
  1:
.endm
# ----------------------------------------------------------------------------
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
.section .bss #                  VARIABLES
# ============================================================================
saved_word: .lcomm sw, WD_BYTES
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
    movl rD, saved_word; # TODO check this
    movl $\dm_addr, rD
    andl $DM_MASK, rD
    movl data_memory(,rD,WD_BYTES), rD
    pushl rD
    movl saved_word, rD # TODO check this
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
.equ iASAV, 0x13000000
.equ iBSAV, 0x14000000
.equ iCOPY, 0x15000000
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
.equ iJMPB, 0x42000000
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
/*
i(0x7fffffff)
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
.equ v_rD, v_vR + 1
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
  i(v_vZ)
  at
  get
  tot
  inc
  bsav

.ifeq TRACING_ENABLED
  TRACE_CHILD_PartA # needs correction
  TRACE_CHILD_PartB
.endif

  i(MSb)
  and
  jmpe(v_Imm)

  fromt
  i(OPCODE_MASK)
  and

      i(iNOOP)
        jmpe(v_Noop)
      i(iRPT)
        jmpe(v_Rpt)

  fromt
  i(COMPLEX_MASK)
  and
  jmpe(v_complex_instrs)


  fromt

      i(iASAV)
        jmpe(v_Asav)
      i(iBSAV)
        jmpe(v_Bsav)
      i(iCOPY)
        jmpe(v_Copy)


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
      i(iJMPE)
        jmpe(v_Jmpe)


      i(iJMPB)
        jmpe(v_Jmpb)


      i(iJUMP)
        jmpe(v_Jump)
      i(iBR)
        jmpe(v_Br)

    i(ILLEGAL)
      fromb
      halt



# ---------------------------------------------------------------------------
v_Asav:
  # FIXME incorrect algorithm
  i(v_vA)
  get
  i(v_rD)
  at
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Bsav:
  # FIXME incorrect algorithm
  i(v_vB)
  get
  i(v_rD)
  at
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Copy:
  # FIXME algorithm not implemented yet
  jump(nexti)
# ---------------------------------------------------------------------------
v_Jmpb:
  # FIXME incorrect algorithm
  i(v_vA)
  get
  jmpb(v_Jmpe_do)
    jump(nexti)
  v_Jmpb_do:
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
v_Add: # Would 24-bit space make this any faster?
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
  jmpb(v_Repeat_end)
    dec
    bsav
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
  br(si)                                # (since improved to < 17 sec)
  i(2) # Performance test (asm child did 0x7fffffff in about 30 sec)
  flip  # 2 0x10000000 0x7fffffff  (= no weird gcc fu but was 1.5-3.0x slower)
  br(si) #  (native asm parent does 0x7fffffff in 2.0 sec)
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


