/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170827+
Version:    pre-alpha-0.0.1.7+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.
  This is a meta-machine: its purpose is to safely virtualize other VM types.
  No call/return, no stack pointer: thus correct and robust without trapping.
  Supports branch/link: uses a standalone, inaccessible link register.
  No undefined behaviour: everything is unsigned, no <= >= operators.
  Harvard architecture: allows easy native implementation.

  20170806/20170819: STILL LOOKS VERY PROMISING AS THE BASIC CORE OF Plan G.
  20170820: Has been simplified to an excellent degree (needs cleaning up).
  Performance is very good despite only having 4 accessible registers.
  Self-virtualization is easy and reasonably performant.
  Keep it simple for later JIT compilation.

  IMPORTANT:
    - Due to C limitations, maximum size of data memory is limited to 1 GB;
      this is because of the way C array size is declared in C and the
      limitations of this size when compiling on 32-bit platforms.
      It is also to make it easy to prevent undefined behaviour.
      Thus maximum DM address space is 28 bits (word-addressed).
      Thus MAX_DM_WORDS is always 0x10000000.

  PROGRESS:
    - This is probably close to the best balance of speed and simplicity
      and ease of self-virtualization, favouring simplicity and ease
      over speed but still achieving good speed. No CISC opcodes remain.
      From here we can tweak this a bit but this is the basic design.
      What is missing is I/O; there is little or no I/O yet.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
//#define TRACING_ENABLED // Comment out unless debugging

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t
#define WD_BYTES 4
#define WD_BITS WD_BYTES*8
#define MSb 0x80000000 // Bit mask for most significant bit
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2
#define MAX_DM_WORDS 0x10000000 // Must be 2^(WD_BITS-4) due to C limitations.
#define DM_WORDS  MAX_DM_WORDS  // Must be some power of 2 <= MAX_DM_WORDS.
#define DM_MASK   DM_WORDS-1
#define nopasm "nop" // The name of the native hardware nop instruction
// There are only 4 accessible registers:
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
LNKT vL = 0; // link register (not accessible)
WORD rSwap = 0; // swap register (not accessible)
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// CURRENTLY 29 OPCODES
// Arithmetic
void Add()    { vA+=vB; }
void Sub()    { vA-=vB; }
// Logic
void Or()     { vA|=vB; }
void And()    { vA&=vB; }
void Xor()    { vA^=vB; }
// Shifts
void Shl()    { vA<<=enshift(vB); }
void Shr()    { vA>>=enshift(vB); }
// Moves
void Get()    { vA = dm[safe(vB)]; }
void Put()    { dm[safe(vB)] = vA; }
void At()     { vB = dm[safe(vB)]; }
// Increments for addressing
void Inc()    { ++vB; }
void Dec()    { --vB; }
// Immediates
void Imm(METADATA x)    { vB = enrange(x); } // bits 31..0
void Flip()             { vB = vB^MSb; }     // bit  32
// Transfers (maybe expand these)
void Swap()   { rSwap = vA; vA = vB; vB = rSwap; }
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Fromb()  { vA = vB; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Noop()    { asm(nopasm); } // prevents unwanted 'optimization' by gcc
#define halt return enbyte(vA);
// Jumps (static only) (an interpreter would enforce a 24-bit program space)
#define jmpe(label) if (vB == vA) { goto label; } // vA equals vB
#define jump(label) goto label; // UNCONDITIONAL
#define rpt(label) if (--vR != 0) { asm(""); goto label; } // prevents optmzn
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// ===========================================================================
// Convenient macros to save typing
#define add Add();
#define sub Sub();
#define or Or();
#define and And();
#define xor Xor();
#define shl Shl();
#define shr Shr();
#define get Get();
#define put Put();
#define at At();
#define inc Inc();
#define dec Dec();
#define i(x) Imm(x);
#define flip Flip();
#define swap Swap();
#define tob Tob();
#define tot Tot();
#define tor Tor();
#define fromb Fromb();
#define fromt Fromt();
#define fromr Fromr();
#define mdm Mdm();
#define noop Noop();
// ===========================================================================
// Opcodes for interpreter of child VM (mostly arbitrary values for now).
// Current scheme is FW32 (poor density but simple, portable).
// These could be better optimized by grouping (interpreter vs FPGA...).
#define iNOOP   0x00000000 // not arbitrary, must be 0x00000000
//#define iIMM   0x80000000 // not arbitrary, must be 0x80000000

// Below 0x40000000 = simple
#define iADD   0x01000000
#define iSUB   0x02000000
#define iOR    0x03000000
#define iAND   0x04000000
#define iXOR   0x05000000
#define iSHL   0x08000000
#define iSHR   0x09000000
#define iGET   0x10000000
#define iPUT   0x11000000
#define iAT    0x12000000
#define iINC   0x20000000
#define iDEC   0x21000000
#define iFLIP  0x22000000
#define iSWAP  0x23000000
#define iTOB   0x30000000
#define iTOR   0x31000000
#define iTOT   0x32000000
#define iFROMB 0x33000000
#define iFROMR 0x34000000
#define iFROMT 0x35000000

#define iMDM   0x36000000
#define iLINK  0x37000000

#define iHALT  0x3f000000

// Above 0x40000000 = complex
#define COMPLEX_MASK 0x40000000

#define iJMPE  0x41000000
#define iJUMP  0x46000000
#define iRPT   0x50000000
#define iBR    0x61000000

// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  return exampleProgram();
}
// ===========================================================================
// Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
int exampleProgram() {

// For native parent VM speed comparison:
// (m32=4 secs, same as m64=4 secs, for 0x7fffffff nop repeats)
// (surprisingly, this is about 3x slower than 'qmisc.s')
//i(0x7fffffff) fromb tor foo: noop rpt(foo) return 0;

#define vm_DM_WORDS DM_WORDS
#define v_DM_WORDS  0x1000 // Must be a power of 2, so <= DM_WORDS/2
#define v_DM_MASK v_DM_WORDS-1
#define v_PM_WORDS  0x1000 // Must be a power of 2, so <= DM_WORDS/2
#define v_PM_MASK v_PM_WORDS-1
#define v_pm 0
#define v_dm v_PM_WORDS
#define v_vZ v_dm + v_DM_WORDS // child program counter
#define v_vA v_vZ + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vT v_vL + 1
#define v_vR v_vT + 1
#define OPCODE_MASK   0xff000000
#define CELL_MASK     0x00ffffff
// ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
// ---------------------------------------------------------------------------
// Process next instruction (not optimized yet)
nexti:
  i(v_vZ)
  at

#ifdef TRACING_ENABLED
printf("\n%08x ", dm[v_vZ]);
#endif

  inc
  fromb
  i(v_PM_MASK)
  and
  i(v_vZ)
  put
  tob
  dec
  get
  tot

#ifdef TRACING_ENABLED
printf("%08x CHILD: vA:%08x vB:%08x vT:%08x vR:%08x vL:%08x ",
        vA, dm[v_vA], dm[v_vB], dm[v_vT], dm[v_vR], dm[v_vL]);
#endif

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
// ---------------------------------------------------------------------------
v_Add:
  i(v_vA)
  get
  i(v_vB)
  at
  add
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Sub:
  i(v_vA)
  get
  i(v_vB)
  at
  sub
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_And:
  i(v_vA)
  get
  i(v_vB)
  at
  and
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Or:
  i(v_vA)
  get
  i(v_vB)
  at
  or
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Xor:
  i(v_vA)
  get
  i(v_vB)
  at
  xor
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Shl:
  i(v_vA)
  get
  i(v_vB)
  at
  shl
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Shr:
  i(v_vA)
  get
  i(v_vB)
  at
  shr
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
v_Inc:
  i(v_vB)
  at
  inc
  fromb
  i(v_vB)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Dec:
  i(v_vB)
  at
  dec
  fromb
  i(v_vB)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Imm:
  fromt
  i(0)
  flip
  xor
  i(v_vB)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Flip:
  i(v_vB)
  at
  flip
  i(v_vB)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
v_Tob:
  i(v_vA)
  get
  i(v_vB)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Tor:
  i(v_vA)
  get
  i(v_vR)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Tot:
  i(v_vA)
  get
  i(v_vT)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Fromb:
  i(v_vB)
  get
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Fromr:
  i(v_vR)
  get
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Fromt:
  i(v_vT)
  get
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Mdm:
  i(v_DM_WORDS)
  fromb
  i(v_vA)
  put
  jump(nexti)
// ---------------------------------------------------------------------------
v_Noop:
  jump(nexti)
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
v_Jump:
    fromt
    i(CELL_MASK)
    and
    i(v_vZ)
    put
    jump(nexti)
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
v_Link:
    i(v_vL)
    get
    i(v_vZ)
    put
    jump(nexti)
// ---------------------------------------------------------------------------
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
// ---------------------------------------------------------------------------
v_Halt:
  i(v_vA)
  get
  i(BYTE_MASK)
  and
  halt
// ---------------------------------------------------------------------------
// Program child's program memory then run program
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
  i(2) // Performance test (C child does 0x7fffffff in 11-19 sec)
  flip  // 2 0x10000000 0x7fffffff  (i.e. fast but uses impenetrable gcc fu)
  br(si)                         // ( 11 sec = 64 bit ; 19 sec = 32 bit) 
  i(iFROMB)
  br(si)
  i(iTOR)
  br(si)
  i(iNOOP) // This is instruction 7 in this program.
  br(si)
  i(iRPT|7)
  br(si)
  i(iHALT)
  br(si)
  jump(nexti)
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Store instruction to child's program memory
si:
  swap
  put
  inc
  fromb
  link
// ---------------------------------------------------------------------------
// Fill vR words at dm[vA] with value in vV (fills 1 GB in about 0.63 seconds)
doFill:
  doFillLoop:
    put
    inc
    rpt(doFillLoop)
    link
// ---------------------------------------------------------------------------
// Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  i(DM_WORDS)
  fromb
  tor
  i(0)
  fromb
  link
// ---------------------------------------------------------------------------
// Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  i(vm_DM_WORDS)
  jmpe(assertedParentSize)
    i(FAILURE)
    fromb
    halt
  assertedParentSize:
    link
// ---------------------------------------------------------------------------
} // end of exampleProgram
// ===========================================================================


