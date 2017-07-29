/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 100 lines of code
                 and totally free of undefined behaviour

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170729+
Version:    pre-alpha-0.0.0.0+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD size_t
#define WORD_SIZE sizeof(size_t)
#define LINK uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x00ffffff
#define SHIFT_MASK 0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define DM_BYTES 0x1000000
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LINK vL = 0; // link register
WORD vZ = 0; // repeat register
WORD rTmp = 0;
WORD dm[DM_BYTES]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
void add(METADATA x)  { vB = enrange(x); vA+=vB; } // would need self-mod...
void sub(METADATA x)  { vB = enrange(x); vA-=vB; }
void or(METADATA x)   { vB = enrange(x); vA|=vB; }
void and(METADATA x)  { vB = enrange(x); vA&=vB; }
void xor(METADATA x)  { vB = enrange(x); vA^=vB; }
void shl(METADATA x)  { vB = enshift(x); vA<<=vB; }
void shr(METADATA x)  { vB = enshift(x); vA>>=vB; }
void lit(METADATA x)  { enrange(x); vA = x; }
void from(METADATA x) { enrange(x); vA = dm[x]; }
void to(METADATA x)   { enrange(x); dm[x] = vA; }
void pull(METADATA x) { enrange(x); vA = dm[dm[x]]; }
void put(METADATA x)  { enrange(x); dm[dm[x]] = vA; }
#define jmpz(label) if (vA == 0) { goto label; }
#define jmpnz(label) if (vA != 0) { goto label; }
#define repeat(label) if (--vZ > 0) { goto label; }
#define branch(label) { __label__ lr; vL = (LINK)&&lr; goto label; lr: ; }
#define merge goto *vL;
void swapAB() { rTmp = vA; vA = vB; vB = rTmp; }
void swapAZ() { rTmp = vA; vA = vZ; vZ = rTmp; }
void toz()    { vZ = vA; }
void fromz()  { vA = vZ; }
void nop()   { ; }
#define halt(x) return x;
// ---------------------------------------------------------------------------
int main() {
  assert(sizeof(WORD) == sizeof(size_t));
  return exampleProgram();
}
// ---------------------------------------------------------------------------
// Example: virtualize this VM inside itself!
int exampleProgram() {
  #define v_DM_BYTES 0x100000 // Need to switch to word-indexing
  #define v_dm 0
  #define v_rPC v_DM_BYTES
  #define v_vA v_rPC + WORD_SIZE
  #define v_vB v_vA + WORD_SIZE
  #define v_vL v_vB + WORD_SIZE
  #define v_vZ v_vL + WORD_SIZE
  #define v_rTmp v_vZ + WORD_SIZE
  #define v_dst v_rTmp + WORD_SIZE
  vm_init:
    lit(0);
    to(v_rPC);
    to(v_vA);
    to(v_vB);
    to(v_vL);
    to(v_vZ);
    to(v_rTmp);
    branch(clearDm);
  end:
    halt(SUCCESS);

  clearDm:
    merge
}

