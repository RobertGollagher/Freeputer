/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    self.s
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170903
Updated:    20170903+
Version:    pre-alpha-0.0.0.3+ for FVM 2.0

This is a FW32 QMISC self-virtualization of the 'qmisc.s' virtual machine.
That is, it virtualizes the VM within itself using its own instructions.
This implementation is a very early, untested, incomplete one.
See 'qmisc.c' for build instructions.

Build and run by:

  ./go.sh self

This requires the build flag NO_PROGRAM to be NO in 'qmisc.c'.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
.include "qmisc.s"
jmp vm_init

# ============================================================================
#  Mostly arbitrary FW32 opcodes (to be refined at a later date)
# ============================================================================
.equ iNOOP, 0x00000000 # not arbitrary
.equ iIMM,  0x80000000 # not arbitrary
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
# Below 0x40000000 = simple
.equ iJMPE, 0x41000000
.equ iJMPB, 0x42000000
.equ iJMPM, 0x43000000
.equ iJUMP, 0x46000000
.equ iRPT,  0x50000000
.equ iBR,   0x61000000
# Above 0x40000000 = complex
.equ COMPLEX_MASK,0x40000000

# ============================================================================
#  Constants
# ============================================================================
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
.equ v_vD, v_vR + 1
.equ OPCODE_MASK,   0xff000000
.equ CELL_MASK,     0x00ffffff
# ---------------------------------------------------------------------------
.ifeq TRACING_ENABLED
  chdfmt_parta: .asciz "\n%08x "
  chdfmt_partb: .asciz "%08x CHILD: vA:%08x vB:%08x vT:%08x vR:%08x vD:%08x vL:%08x "

  .macro PUSH_DM_VAL dm_addr
    movl $\dm_addr, %ebx
    andl $DM_MASK, %ebx
    movl data_memory(,%ebx,WD_BYTES), %ebx
    pushl %ebx
  .endm

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
    PUSH_DM_VAL v_vD
    PUSH_DM_VAL v_vR
    PUSH_DM_VAL v_vT
    PUSH_DM_VAL v_vB
    PUSH_DM_VAL v_vA
    pushl vA
    pushl $chdfmt_partb
    call printf
    addl $32, %esp
    RESTORE_REGS
  .endm
.endif
# ----------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  #br(setupToClearParent) # Unneccessary (.lcomm always zeros)
  #br(doFill)             # Unneccessary (.lcomm always zeros)
  jump(program)
# ----------------------------------------------------------------------------
# Process next instruction (not optimized yet)
nexti:
  i(v_vZ)
  at
  get
  tot

.ifeq TRACING_ENABLED
  TRACE_CHILD_PartA
.endif

  inc
  bsav

.ifeq TRACING_ENABLED
  TRACE_CHILD_PartB
.endif

  jmpm(v_Imm)

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
  i(v_vD)
  at
  put
  jump(nexti)
# ---------------------------------------------------------------------------
v_Bsav:
  # FIXME incorrect algorithm
  i(v_vB)
  get
  i(v_vD)
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
v_Jmpm:
  i(v_vA)
  get
  jmpm(v_Jmpm_do)
    jump(nexti)
  v_Jmpm_do:
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
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
  br(si)
  i(2) # Performance test (asm child 0x7fffffff in 18.3 sec)
  flip # 2 0x10000000 0x7fffffff (native asm parent 0x7fffffff in 1.39 sec)
  br(si)
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
