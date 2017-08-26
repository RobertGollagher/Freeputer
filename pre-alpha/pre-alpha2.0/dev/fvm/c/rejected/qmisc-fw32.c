/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 100 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170801+
Version:    pre-alpha-0.0.0.10+ for FVM 2.0

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
#define METADATA_MASK 0x00ffffff
#define SHIFT_MASK 0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define DM_WORDS 0x10000000 // FIXME Too wide
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LINK vL = 0; // link register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
WORD vS = 0; // source register
WORD vD = 0; // destination register
WORD vI = 0; // immeduate register
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// TODO rel jumps?
// TODO IDEA: go FW8 using an imm register
void add()    { vA+=vB; }
void sub()    { vA-=vB; }
void or()     { vA|=vB; }
void and()    { vA&=vB; }
void xor()    { vA^=vB; }
void not()    { vA=~vA; }
void neg()    { vA=~vA; ++vA; } // MAX_NEG unchanged (an advantage)
void shl()    { vA<<=enshift(vB); }
void shr()    { vA>>=enshift(vB); }
void lit()    { vA = vI; }
void by()     { vB = vI; }
void num()    { vR = vI; }
void src()    { vS = vI; }
void dst()    { vD = vI; }
void from()   { vA = dm[vS]; }
void with()   { vB = dm[vS]; }
void pull()   { vA = dm[dm[vS]]; }
void use()    { vB = dm[dm[vS]]; }
void to()     { dm[vD] = vA; }
void put()    { dm[dm[vD]] = vA; }
void incs()   { vS++; }
void decs()   { vS--; }
void incd()   { vD++; }
void decd()   { vD--; }
// This can now be 31 bits as it is the only literal instruction
void imm(METADATA x)    { enrange(x); vI = x; }
#define jmpz(label) if (vA == 0) { goto label; } // ZERO
#define jmpm(label) if (vA == NEG_MASK) { goto label; } // MAX_NEG
#define jmpn(label) if ((vA & NEG_MASK) == NEG_MASK) { goto label; } // NEG
#define jmpb(label) if ((vA & BIG_MASK) == BIG_MASK) { goto label; } // BIG
#define jump(label) goto label; // UNCONDITIONAL
#define repeat(label) if (--vR > 0) { goto label; }
#define branch(label) { __label__ lr; vL = (LINK)&&lr; goto label; lr: ; }
#define merge goto *vL;
void tob()    { vB = vA; }
void tot()    { vT = vA; }
void tor()    { vR = vA; }
void tos()    { vS = vA; }
void tod()    { vD = vA; }
void fromb()  { vA = vB; }
void fromt()  { vA = vT; }
void fromr()  { vA = vR; }
void froms()  { vA = vS; }
void fromd()  { vA = vD; }
void nop()    { ; }
#define halt(x) return x;
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
  branch(setupToClearParent);
  branch(doFill);
end:
  halt(SUCCESS);

// Setup to doFill so as to clear entire data memory of parent
setupToClearParent:
  imm(0);
  dst();
  imm(DM_WORDS); // FIXME effectively imm(0) due to enrange(METADATA)
  num();
  merge

// Fill vR words at v_dst with value in vA.
// (Note: this will fill 1 GB in about 0.5 seconds)
doFill:
  put();
  incd();
  repeat(doFill)
  merge

} // end ofexampleProgram

