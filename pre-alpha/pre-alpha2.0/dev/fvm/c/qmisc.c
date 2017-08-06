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
#define LINK uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31-bits now
#define SET_MASK 0x80000000 // Sets msb
#define SHIFT_MASK 0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define DM_WORDS 0x10000000
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LINK vL = 0; // link register
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
void Incs()   { vS++; }
void Decs()   { vS--; }
void Incd()   { vD++; }
void Decd()   { vD--; }
// Immediates
void imm(METADATA x)    { enrange(x); vI = x; } // bits 31..0
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


// SLOWER but more consistent design, larger program space,
// FIXME not robust; also im macro is cheating. Rel jmps?
#define im(label) imm((WORD)&&label)
#define jmpz if (vA == 0) { goto *vI; } // ZERO
#define jmpm if (vA == NEG_MASK) { goto *vI; } // MAX_NEG
#define jmpn if ((vA & NEG_MASK) == NEG_MASK) { goto *vI; } // NEG
#define jmpb if ((vA & BIG_MASK) == BIG_MASK) { goto *vI; } // BIG
#define jump goto *vI; // UNCONDITIONAL
#define repeat if (--vR > 0) { goto *vI; }
#define branch(label) { __label__ lr; vL = (LINK)&&lr; goto label; lr: ; }
#define merge goto *vL;

/* // FASTER but cannot do with 31-bit literals, so <=28-bit program space
#define jmpz(label) if (vA == 0) { goto label; } // ZERO
#define jmpm(label) if (vA == NEG_MASK) { goto label; } // MAX_NEG
#define jmpn(label) if ((vA & NEG_MASK) == NEG_MASK) { goto label; } // NEG
#define jmpb(label) if ((vA & BIG_MASK) == BIG_MASK) { goto label; } // BIG
#define jump(label) goto label; // UNCONDITIONAL
#define repeat(label) if (--vR > 0) { goto label; }
#define branch(label) { __label__ lr; vL = (LINK)&&lr; goto label; lr: ; }
#define merge goto *vL;
*/

// Other
void Nop()    { ; }
#define halt(x) return x;

// ---------------------------------------------------------------------------
int main() {
  assert(sizeof(WORD) == sizeof(size_t));
  return exampleProgram();
}
// ---------------------------------------------------------------------------
// Convenience macros
#define by ImmB();
#define sp ImmP();
#define immA ImmA();
#define push Push();
#define sub Sub();
#define pop Pop();
#define dst ImmD();
#define num ImmR();
#define put Put();
#define incd Incd();
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
  branch(setupToClearParent);
  branch(doFill);
  im(foo);
  jump
end:
  imm(1);
  by
  imm(2);
  sp
  immA
  push
  sub
  push
  pop
  pop
  pop
  halt(SUCCESS);
foo:
  im(end);
  jump

// Setup to doFill so as to clear entire data memory of parent
setupToClearParent:
  imm(0);
  dst
  imm(DM_WORDS);
  num
  merge

// Fill vR words at v_dst with value in vA.
// (Note: this will fill 1 GB in about 0.5 seconds)
doFill:
  im(doFillLoop);
  doFillLoop:
    put
    incd
    repeat
    merge

} // end ofexampleProgram
// ---------------------------------------------------------------------------
