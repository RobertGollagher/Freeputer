/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 100 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170805+
Version:    pre-alpha-0.0.0.33+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed notes so as not to prejudice lateral thinking during design.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD size_t
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
#define DM_WORDS 0x10000000
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
void From()   { vA = dm[vS]; }
void To()     { dm[vD] = vA; }
void Pull()   { vA = dm[dm[vS]]; }
void Put()    { dm[dm[vD]] = vA; } // FIXME these are all not robust (bounds)
void Pop()    { vA = dm[vP++]; }
void Push()   { dm[--vP] = vA; }
// B moves
void BFrom()  { vB = dm[vS]; }
void BPull()  { vB = dm[dm[vS]]; }
void BPop()   { vB = dm[vP++]; }
// Increments
void IncS()   { vS++; }
void DecS()   { vS--; }
void IncD()   { vD++; }
void DecD()   { vD--; }
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
// Jumps (intentionally not dynamic)
#define JMPZ(label) if (vA == 0) { goto label; } // ZERO
#define JMPM(label) if (vA == NEG_MASK) { goto label; } // MAX_NEG
#define JMPN(label) if ((vA & NEG_MASK) == NEG_MASK) { goto label; } // NEG
#define JMPB(label) if ((vA & BIG_MASK) == BIG_MASK) { goto label; } // BIG
#define JUMP(label) goto label; // UNCONDITIONAL
#define REPEAT(label) if (--vR > 0) { goto label; }
#define BRANCH(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define LINK goto *vL;
// Other
void Nop()    { ; }
#define HALT(x) return x;

// ---------------------------------------------------------------------------
int main() {
  assert(sizeof(WORD) == sizeof(size_t));
  return exampleProgram();
}
// ---------------------------------------------------------------------------
// Example: to be a small QMISC FW32 implementation
int exampleProgram() {
#define v_PM_WORDS 0x1000
#define v_DM_WORDS 0x1000
#define v_pm 0xff // Skip zero page
#define v_dm v_PM_WORDS
#define v_rPC v_dm + v_DM_WORDS
#define v_vA v_rPC + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vR v_vL + 1
#define v_vT v_vR + 1
#define v_src v_vT + 1
#define v_dst v_src + 1
vm_init:
  BRANCH(setupToClearParent)
  BRANCH(doFill)
  JUMP(foo)
end:
  Imm(1);
  ImmB();
  Imm(2);
  ImmP();
  ImmA();
  Push();
  Sub();
  Push();
  Pop();
  Pop();
  Pop();
  HALT(SUCCESS)
foo:
  JUMP(end)

// Setup to doFill so as to clear entire data memory of parent
setupToClearParent:
  Imm(0);
  ImmD();
  Imm(DM_WORDS);
  ImmR();
  LINK

// Fill vR words at v_dst with value in vA.
// (Note: this will fill 1 GB in about 0.46/0.63 seconds varied by jump method)
doFill:
  doFillLoop:
    Put();
    IncD();
    REPEAT(doFillLoop)
    LINK

} // end ofexampleProgram
// ---------------------------------------------------------------------------
