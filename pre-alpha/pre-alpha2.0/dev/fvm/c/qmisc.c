/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170806+
Version:    pre-alpha-0.0.0.37+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.
  Harvard architecture. Data memory: 32-bit address space.
  Program memory: 24-bit program space if interpreted.
  Program memory: no limit if native.

    20170806: THIS NOW LOOKS VERY PROMISING AS THE BASIC CORE OF Plan G.

      FIXME NEXT: need to add a call/return solution as otherwise
        native implementation is impractical (link insufficient).
        Also a register to hold a PC for child would be good.

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
#define NEG_MASK 0x80000000 // If negative or MAX_NEG in two's complement.
#define BIG_MASK 0x40000000 // If absolute value > 0x3fffffff.
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31-bits now
#define SET_MASK 0x80000000 // Sets msb
#define SHIFT_MASK 0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define DM_WORDS 0x10000000 // Must be power of 2
#define DM_MASK  0x0fffffff
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LNKT vL = 0; // link register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
WORD vS = 0; // source register
WORD vD = 0; // destination register
WORD vP = 0; // stack pointer register
WORD v1 = 0; // stack pointer parking register 1
WORD v2 = 0; // stack pointer parking register 2
WORD v3 = 0; // stack pointer parking register 3
WORD v4 = 0; // stack pointer parking register 4
WORD vI = 0; // immediate register
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Arithmetic
void Add()    { vA+=vB; }
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
// A moves
void From()   { vA = dm[vS&DM_MASK]; }
void To()     { dm[vD&DM_MASK] = vA; }
void Pull()   { vA = dm[dm[vS&DM_MASK]]; }
void Put()    { dm[dm[vD&DM_MASK]] = vA; }
void Pop()    { vP = vP&DM_MASK; vA = dm[vP++]; vP = vP&DM_MASK; }
void Push()   { --vP; dm[vP&DM_MASK] = vA; }
// B moves
void BFrom()  { vB = dm[vS&DM_MASK]; }
void BPull()  { vB = dm[dm[vS&DM_MASK]]; }
void BPop()   { vP = vP&DM_MASK; vB = dm[vP++]; vP = vP&DM_MASK; }
// Increments
void Inc()    { ++vA; }
void Dec()    { --vD; }
void IncS()   { ++vS; }
void DecS()   { --vS; }
void IncD()   { ++vD; }
void DecD()   { --vD; }
// Immediates
void Imm(METADATA x)    { enrange(x); vI = x; } // bits 31..0
void Set()    { vI|=SET_MASK; }                 // bit  32
void ImmA()   { vA = vI; }
void ImmB()   { vB = vI; }
void ImmR()   { vR = vI; }
void ImmS()   { vS = vI; }
void ImmD()   { vD = vI; }
void ImmP()   { vP = vI; }
// Transfers
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Tos()    { vS = vA; }
void Tod()    { vD = vA; }
void Toi()    { vI = vA; }
void Top()    { vP = vA; }
void Fromb()  { vA = vB; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
void Froms()  { vA = vS; }
void Fromd()  { vA = vD; }
void Fromp()  { vA = vP; }
// Stack parking
void Use1()   { vP = v1; }
void Use2()   { vP = v2; }
void Use3()   { vP = v3; }
void Use4()   { vP = v4; }
void Pto1()   { v1 = vP; }
void Pto2()   { v2 = vP; }
void Pto3()   { v3 = vP; }
void Pto4()   { v4 = vP; }
// Jumps (static only) (an interpreter would enforce a 24-bit program space)
#define jmpz(label) if (vA == 0) { goto label; } // ZERO
#define jmpm(label) if (vA == NEG_MASK) { goto label; } // MAX_NEG
#define jmpn(label) if ((vA & NEG_MASK) == NEG_MASK) { goto label; } // NEG
#define jmpb(label) if ((vA & BIG_MASK) == BIG_MASK) { goto label; } // BIG
#define jump(label) goto label; // UNCONDITIONAL
#define repeat(label) if (--vR != 0) { goto label; }
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Nop()    { ; }
#define halt(x) return x;
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
#define load From();
#define store To();
#define pull Pull();
#define put Put();
#define pop Pop();
#define push Push();
#define bfrom BFrom();
#define bpull BPull();
#define bpop BPop();
#define inc Inc();
#define dec Dec();
#define incs IncS();
#define decs DecS();
#define incd IncD();
#define decd DecD();
#define imm(x) Imm(x);
#define set Set();
#define imma ImmA();
#define immb ImmB();
#define immr ImmR();
#define imms ImmS();
#define immd ImmD();
#define immp ImmP();
#define tob Tob();
#define tot Tot();
#define tor Tor();
#define tos Tos();
#define tod Tod();
#define toi Toi();
#define top Top();
#define fromb Fromb();
#define fromt Fromt();
#define fromr Fromr();
#define froms Froms();
#define fromd Fromd();
#define fromp Fromp();
#define use1 Use1();
#define use2 Use2();
#define use3 Use3();
#define use4 Use4();
#define pto1 Pto1();
#define pto2 Pto2();
#define pto3 Pto3();
#define pto4 Pto4();
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
#define v_rPC v_dm + v_DM_WORDS
#define v_instr v_rPC + 1
#define v_vA v_instr + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vT v_vL + 1
#define v_vR v_vT + 1
#define v_vS v_vR + 1
#define v_vD v_vS + 1
#define v_vP v_vD + 1
#define v_v1 v_vP + 1
#define v_v2 v_v1 + 1
#define v_v3 v_v2 + 1
#define v_v4 v_v3 + 1
#define v_vI v_v4 + 1
// ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
// ---------------------------------------------------------------------------
next: // FIXME faulty (and efficiency dubious)
// prepare to use v_rPC
    imm(v_rPC)
    imms
    immd
// increment v_rPC
    load
    inc
    store
// load instruction
    pull
    tot
// case iNOP:
    jmpz(Nop)
// case iHALT:
    fromt
    imm(iHALT)
    immb
    sub
    jmpz(Halt) 
// default:
    halt(8)
// ---------------------------------------------------------------------------
Nop:
  link  
// ---------------------------------------------------------------------------
Halt:
  halt(9)
// ---------------------------------------------------------------------------
// Program child's program memory then run program
program:
  imm(0)
  immd
  imm(iNOP)
  br(istore)
  imm(iHALT)
  br(istore)
  jump(run)
// ---------------------------------------------------------------------------
// Store instruction in vI to v_pm[vD++] in child's program memory
istore:
  imma
  store
  incd
  link
// ---------------------------------------------------------------------------
// Run child's program
run:
  br(next)
  jump(run)
// ---------------------------------------------------------------------------
// Fill vR words at v_dst with value in vA (fills 1 GB in about 0.36 seconds)
doFill:
  doFillLoop:
    put
    incd
    repeat(doFillLoop)
    link
// ---------------------------------------------------------------------------
// Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  imm(0)
  immd
  imm(DM_WORDS)
  immr
  link
// ---------------------------------------------------------------------------
// Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  imm(vm_DM_WORDS)
  immb
  sub
  jmpz(assertedParentSize)
    halt(FAILURE)
  assertedParentSize:
    link
// ---------------------------------------------------------------------------

} // end of exampleProgram
// ===========================================================================
