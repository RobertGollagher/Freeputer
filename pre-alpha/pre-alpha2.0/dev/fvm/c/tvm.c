/*
                      SPARSE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170721
Updated:    20170729+
Version:    pre-alpha-0.0.0.1+ for FVM 2.0

See 'tvm.s' (x86 assembly language). It is the primary implementation.
This 'tvm.c' is a secondard implementation in C as a sanity check.
The unusual comments are copied from 'tvm.s' for cross-checking.
The unusual syntax and patterns are to correspond to 'tvm.s'.

  // With -O3 this takes 1.4 seconds:
  add_by(0x7fffffff);
  loop:
    sub_by(1);
    jmpgz(loop)

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

==============================================================================
                          SANITY CHECK: PROBLEMS
==============================================================================

  - 24-bit metadata is not a C type
  - TODO maybe refactor to more C-like referencing and dereferencing
  - this implementation is straightforward but tedious boilerplate

==============================================================================
                                 BUILDING
==============================================================================

For now, since this is a pre-alpha implementation and thus performance is
a secondary consideration, the recommended command to build the FVM executable
for a Linux target with the provided Makefile is simply:

  make

Which is equivalent to:

  gcc -o tvm tvm.c

But performance is hopeless. For better performance do:

  make good

Which is equivalent to:

  gcc -o tvm tvm.c -O3

Then conveniently run and time the example program by:

  time ./tvm; echo $?

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#define WORD int32_t
#define METADATA int32_t // FIXME PROBLEM: C has no int24_t
#define SUCCESS 0
#define FAILURE 1
// Size of the virtual machine:
#define DM_BYTES 0x1000000 // could be up to 0x100000000
#define WORD_SIZE 4
// Registers of the virtual machine:
WORD vA = 0; // (was %ebx) accumulator
WORD vB = 0; // (was %edx operand register
WORD vL = 0; // (was %edi) link register
WORD vZ = 0; // (was %esi) buffer register, also used for repeat
// Registers of the implementation:
WORD rTmp = 0; // (was %eax) primary temporary register
WORD rBuf = 0; // (was %ecx) secondary temporary register
WORD data_memory[DM_BYTES];
int exampleProgram();

// ===========================================================================
//                THE PLUMBING (not to be used in programs)
// ===========================================================================
void reg_sign_extend(METADATA x, WORD *reg) {
  *reg = x;
  rBuf = x & 0x00800000;
  if (rBuf != 0) {
    *reg = *reg | 0xff000000;
  }
}

void reg_m(METADATA x, WORD *reg) {
  rTmp = x << 8;
  *reg = *reg & 0x00ffffff;
  *reg = rTmp | *reg;
}

void reg_ptr_pp(WORD *regPtr) {
  data_memory[*regPtr] += WORD_SIZE;
}

void reg_mm(WORD *regPtr) {
  *regPtr -= WORD_SIZE;
}

void reg_store(WORD *regSrc, WORD *regDst) {
  data_memory[*regDst] = *regSrc;
}

void reg_load(WORD *regSrc, WORD *regDst) {
  *regDst = data_memory[*regSrc];
}

// ---------------------------------------------------------------------------
void reg_at(METADATA x, WORD *reg) {
  rTmp = x;
  reg_load(&rTmp, reg);
}

void reg_ptr_load(METADATA x, WORD *reg) {
  reg_at(x, &rTmp);
  reg_load(&rTmp, reg);
}

void reg_ptr_load_pp(METADATA x, WORD *reg) {
  rBuf = x;
  reg_load(&rBuf, reg);
  reg_ptr_pp(&rBuf);
  reg_load(&rTmp, reg);
}

void reg_ptr_load_mm(METADATA x, WORD *reg) {
  reg_at(x, &rBuf);
  reg_mm(&rBuf);
  reg_store(&rBuf, &rTmp);
  reg_load(&rBuf, &vA);
}
// ---------------------------------------------------------------------------
void by(METADATA x) {
  vB = x;
}

void byx(METADATA x) {
  reg_sign_extend(x,&vB);
}

void bym(METADATA x) {
  reg_m(x,&vB);
}

void by_at(METADATA x) {
  reg_at(x,&vB);
}

void by_ptr(METADATA x) {
  reg_ptr_load(x,&vB);
}

void by_ptr_pp(METADATA x) {
  reg_ptr_load_pp(x,&vB);
}

void by_ptr_mm(METADATA x) {
  reg_ptr_load_pp(x,&vB);
}
// ---------------------------------------------------------------------------
void add() {
  vA+=vB;
}

void sub() {
  vA-=vB;
}
// ===========================================================================
//                            INSTRUCTION SET
// ===========================================================================
void lit(METADATA x) {
  vA = x;
}

void litx(METADATA x) {
  reg_sign_extend(x,&vA);
}

void litm(METADATA x) {
  reg_m(x,&vA);
}
// ---------------------------------------------------------------------------
void from(METADATA x) {
  reg_at(x,&vA);
}

void from_ptr(METADATA x) {
  reg_ptr_load(x,&vA);
}

void from_ptr_pp(METADATA x) {
  reg_ptr_load_pp(x,&vA);
}

void from_ptr_mm(METADATA x) {
  reg_ptr_load_mm(x,&vA);
}
// ---------------------------------------------------------------------------
void to(METADATA x) {
  rTmp = x;
  reg_store(&vA,&rTmp);
}

void to_ptr(METADATA x) {
  rTmp = x;
  reg_store(&vA,&rTmp);
}

void to_ptr_pp(METADATA x) {
  rBuf = x;
  reg_load(&rBuf,&rTmp);
  reg_store(&vA,&rTmp);
  reg_ptr_pp(&rBuf);
}

void to_ptr_mm(METADATA x) {
  reg_at(x,&rBuf);
  reg_mm(&rBuf);
  reg_store(&rBuf,&rTmp);
  reg_store(&vA,&rBuf);
}
// ---------------------------------------------------------------------------
void add_by(METADATA x) {
  by(x);
  add();
}

void add_bym(METADATA x) {
  bym(x);
  add();
}

void add_byx(METADATA x) {
  byx(x);
  add();
}

void add_by_at(METADATA x) {
  by_at(x);
  add();
}

void add_by_ptr(METADATA x) {
  by_ptr(x);
  add();
}

void add_by_ptr_mm(METADATA x) {
  by_ptr_mm(x);
  add();
}

void add_by_ptr_pp(METADATA x) {
  by_ptr_pp(x);
  add();
}
// ---------------------------------------------------------------------------
void sub_by(METADATA x) {
  by(x);
  sub();
}

void sub_bym(METADATA x) {
  bym(x);
  sub();
}

void sub_byx(METADATA x) {
  byx(x);
  sub();
}

void sub_by_at(METADATA x) {
  by_at(x);
  sub();
}

void sub_by_ptr(METADATA x) {
  by_ptr(x);
  sub();
}

void sub_by_ptr_mm(METADATA x) {
  by_ptr_mm(x);
  sub();
}

void sub_by_ptr_pp(METADATA x) {
  by_ptr_pp(x);
  sub();
}
// ---------------------------------------------------------------------------

#define jmpgz(label) if (vA > 0) { goto label; }
// ===========================================================================
//                               ENTRY POINT
// ===========================================================================
int main() {
  int outcome = exampleProgram();
  return outcome;
}

/*
  This is an example of a native program.
*/
int exampleProgram() {
  // With -O3 this takes 1.4 seconds:
  add_by(0x7fffffff);
  loop:
    sub_by(1);
    jmpgz(loop)

  return 0;
}



























