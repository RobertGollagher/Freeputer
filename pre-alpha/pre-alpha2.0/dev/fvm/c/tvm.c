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
  - FIXME properly address compiler warnings

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

// ===========================================================================
//                THE PLUMBING (not to be used in programs)
// ===========================================================================
void reg_sign_extend(METADATA x, WORD *reg) {
  reg = x;
  rBuf = x & 0x00800000;
  if (rBuf != 0) {
    reg = *reg | 0xff000000;
  }
}

void reg_m(METADATA x, WORD *reg) {
  rTmp = x << 8;
  reg = *reg & 0x00ffffff;
  reg = rTmp | *reg;
}

void by(METADATA x) {
  vB = x;
}

void byx(METADATA x) {
  reg_sign_extend(x,vB);
}

void bym(METADATA x) {
  reg_m(x,vB);
}

void add() {
  vA+=vB;
}

void sub() {
  vA-=vB;
}

// ===========================================================================
//                            INSTRUCTION SET
// ===========================================================================
void add_by(METADATA x) {
  by(x);
  add();
}

void sub_by(METADATA x) {
  by(x);
  sub();
}

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



























