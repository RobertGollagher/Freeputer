/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170816+
Version:    pre-alpha-0.0.0.52+ for FVM 2.0

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

  20170806/20170816: STILL LOOKS VERY PROMISING AS THE BASIC CORE OF Plan G.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t
#define WORD_SIZE 4
#define SET_MASK 0x80000000  // Topmost bit 1
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2
#define DM_WORDS 0x10000000 // Must be power of 2
#define DM_MASK  0x0fffffff
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LNKT vL = 0; // link register
WORD vT = 0; // temporary register
WORD vV = 0; // value register (for put)
WORD vR = 0; // repeat register
WORD rSwap = 0; // not exposed, supports Swap() instruction
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Arithmetic
void Add()    { vA+=vB; } // Hard to know if overflowed, easy to know if can overflow
void Sub()    { vA-=vB; }
// Logic
void Or()     { vA|=vB; }
void And()    { vA&=vB; }
void Xor()    { vA^=vB; }
void Not()    { vA=~vA; }
void Neg()    { vA=~vA; ++vA; } // MAX_NEG unchanged (an advantage)
// Shifts
void Shl()    { vA<<=enshift(vB); }
void Shr()    { vA>>=enshift(vB); }
// Moves
void Get()    { vA = dm[safe(vA)]; }
void Put()    { dm[safe(vA)] = vV; }
// Increments
void Inc()    { ++vA; }
void Dec()    { --vA; }
// Immediates
void Imm(METADATA x)    { enrange(x); vB = x; } // bits 31..0
void Set()    { vB|=SET_MASK; }                 // bit  32
void ImmR()   { vR = vB; }
void ImmT()   { vT = vB; }
void ImmV()   { vV = vB; }
// Transfers (maybe expand these)
void Swap()   { rSwap = vA; vA = vV; vV = rSwap; }
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Tov()    { vV = vA; }
void Fromb()  { vA = vB; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
void Fromv()  { vA = vV; }
// Jumps (static only) (an interpreter would enforce a 24-bit program space)
#define jmpz(label) if (vA == 0)     { goto label; } // vA is zero
#define jmpe(label) if (vB == vA)    { goto label; } // vA equals vB
#define jmps(label) if (vB == vA&vB) { goto label; } // vA has all 1s of vB
#define jmpu(label) if (vB == vA|vB) { goto label; } // vA has all 0s of vB
#define jump(label) goto label; // UNCONDITIONAL
#define repeat(label) if (--vR != 0)  { goto label; }
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Nop()    { ; }
#define halt return enbyte(vA);
// ===========================================================================
// Convenient macros to save typing
#define add Add();
#define sub Sub();
#define or Or();
#define and And();
#define xor Xor();
#define not Not();
#define neg Neg();
#define shl Shl();
#define shr Shr();
#define get Get();
#define put Put();
#define inc Inc();
#define dec Dec();
#define imm(x) Imm(x);
#define set Set();
#define immr ImmR();
#define immt ImmT();
#define immv ImmV();
#define msba(x) MsbA(x);
#define msbb(x) MsbB(x);
#define msbr(x) MsbR(x);
#define msbv(x) MsbV(x);
#define swap Swap();
#define tob Tob();
#define tot Tot();
#define tor Tor();
#define tov Tov();
#define fromb Fromb();
#define fromt Fromt();
#define fromr Fromr();
#define fromv Fromv();
#define mdm Mdm();
#define nop Nop();
// ===========================================================================
// Opcodes for interpreter
#define iNOP  0x00
#define iHALT 0x7f
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WORD_SIZE);
  return exampleProgram();
}
// ===========================================================================
// Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
int exampleProgram() {
#define vm_DM_WORDS 0x10000000
#define v_DM_WORDS  0x1000
#define v_PM_WORDS  0x1000
#define v_pm 0
#define v_dm v_PM_WORDS
// Actually not using v_rPC any more (using v1 instead)
#define v_rPC v_dm + v_DM_WORDS
#define v_instr v_rPC + 1
#define v_vA v_instr + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vT v_vL + 1
#define v_vR v_vT + 1
#define v_vV v_vR + 1
#define v_vI v_vV + 1
// ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
// ---------------------------------------------------------------------------
// Process next instruction
next:
// get dm[v_rPC]
    imm(v_rPC)
    fromb
    tov // store v_rPC addr in vV
    get // get val of v_rPC into vT
    tot
// increment dm[v_rPC]
    inc
    swap
    put
// get current instr into vT
    fromt // retrieve val of v_rPC from vT
    get   // get instr at dm[v_rPC] into vT
    tot
// case iNOP:
    jmpz(Nop)
// case iHALT:
    fromt
    imm(iHALT)
    jmpe(Halt)
// default:
    imm(ILLEGAL)
    fromb
    halt
// ---------------------------------------------------------------------------
Nop:
  jump(next)
// ---------------------------------------------------------------------------
Halt:
  imm(SUCCESS)
  fromb
  halt
// ---------------------------------------------------------------------------
// Program child's program memory then run program
program:
  imm(0)
  fromb
  imm(iNOP)
  br(istore)
  imm(iHALT)
  br(istore)
  jump(next)
// ---------------------------------------------------------------------------
// Store value in vB to v_pm[vA++] in child's program memory
istore:
  immv
  put
  inc
  link
// ---------------------------------------------------------------------------
// Fill vR words at dm[vA] with value in vV (fills 1 GB in about 0.63 seconds)
doFill:
  doFillLoop:
    put
    inc
    repeat(doFillLoop)
    link
// ---------------------------------------------------------------------------
// Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  imm(0)
  immv
  imm(DM_WORDS)
  immr
  link
// ---------------------------------------------------------------------------
// Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  imm(vm_DM_WORDS)
  sub
  jmpz(assertedParentSize)
    imm(FAILURE)
    fromb
    halt
  assertedParentSize:
    link
// ---------------------------------------------------------------------------
// Increment variable at dm[vA]
Incr:
  tov
  get
  inc
  swap
  put
  link
// ---------------------------------------------------------------------------

} // end of exampleProgram
// ===========================================================================
