/*
                      SPARSE REGISTER MACHINE (SRM)

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    srm
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170721
Updated:    20170729+
Version:    pre-alpha-0.0.0.0+ for FVM 2.0

See 'tvm.s' (x86 assembly language). It is the primary implementation.
This 'tvm.c' is a secondard implementation in C as a sanity check.
The unusual comments are copied from 'tvm.s' for cross-checking.

                              This Edition:
                               Portable C 
                            for Linux and gcc

                               ( ) [ ] { }

==============================================================================
                                 BUILDING
==============================================================================

For now, since this is a pre-alpha implementation and thus performance is
a secondary consideration, the recommended command to build the FVM executable
for a Linux target with the provided Makefile is simply:

  make

Which is equivalent to:

  gcc -o fvm fvm.c

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#define WORD int32_t
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

#define reg_sign_extend(x,reg) \
  reg = x; \
  rBuf = x & 0x00800000; \
  if (rBuf != 0) { \
    reg = reg | 0xff000000; \
  }

#define reg_m(x,reg) \
  rTmp = x << 8; \
  reg = reg & $0x00ffffff; \
  reg = reg | rTmp;



// ===========================================================================
//                               ENTRY POINT
// ===========================================================================
main() {

}



























