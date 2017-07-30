/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 100 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170730+
Version:    pre-alpha-0.0.0.4+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  NOTES:

    - QMISC eliminates undefined behaviour.
    - QMISC values QUALITY and SIMPLICITY over performance.
        - Nevertheless, native QMISC N performance is reasonable:
            - doFill in exampleProgram() fills 1 GB in about 0.5 seconds.
    - QMISC is intended for EASY COMPREHENSION BY HUMANS not compilers.
    - QMISC is intended as a platform for virtualizing other virtual machines.
    - QMISC itself uses a Harvard architecture to ease native implementation.
    - QMISC requires two's complement but uses only unsigned integers.
    - QMISC has no status flags so as to ease implementation.
    - QMISC uses sensible bounds for efficient computing:
        - 32-bit word size for all registers and for data memory
        - 32-bit data-memory address space (word-addressed)
        - 24-bit program-memory address space
    - QMISC comes in two flavours:
        - QMISC N: (e.g. this 'qmisc.c' itself)
            - program memory can use any word size; and
            - program memory need not use fixed-width instructions; but
            - all registers and data memory must use 32-bit word size.
        - QMISC FW32: (e.g. exampleProgram() below)
            - not only data memory but also program memory has:
                - 32-bit word size; and
                - fixed-width 32-bit instructions.
        - Therefore for portability it is best practice for QMISC programs:
            - never to depend on word size of program memory; and
            - never to depend on instruction size of program memory; and
            - never to depend on program memory size; and
            - never to depend on program memory structure.
    - QMISC supports the use of 32-bit words in the normal manner.
    - QMISC also makes it easy to use optional strategies such as:
        - QMISC makes it easy to treat MAX_NEG (0x80000000) as NaN; and
        - QMISC makes it easy to treat bits 29..0 as a 30-bit integer; and
        - QMISC makes it easy to treat bit 30 as a smart overflow bit; and
        - QMISC makes it easy to treat bit 31 as a sign bit; and
        - this is perfectly in harmony with word-addressing.

  TODO:

    - Consider that first 24-bit region of 32-bit data memory will
        be the most-used region. Is that a concern?
    - Consider if v_src and v_dst should exist as registers in parent;
        maybe but not necessarily better that way.

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
#define DM_WORDS 0x10000000
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LINK vL = 0; // link register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// TODO rel jumps?
void add()              { vA+=vB; }
void sub()              { vA-=vB; }
void or()               { vA|=vB; }
void and()              { vA&=vB; }
void xor()              { vA^=vB; }
void not()              { vA=~vA; }
void neg()              { vA=~vA; ++vA; } // MAX_NEG unchanged (an advantage)
void shl()              { vA<<=enshift(vB); }
void shr()              { vA>>=enshift(vB); }
void lit(METADATA x)    { enrange(x); vA = x; }
void by(METADATA x)     { enrange(x); vB = x; }
void times(METADATA x)  { enrange(x); vR = x; }
void from(METADATA x)   { enrange(x); vA = dm[x]; }
void with(METADATA x)   { enrange(x); vB = dm[x]; }
void pull(METADATA x)   { enrange(x); vA = dm[dm[x]]; }
void using(METADATA x)  { enrange(x); vB = dm[dm[x]]; }
void to(METADATA x)     { enrange(x); dm[x] = vA; }
void put(METADATA x)    { enrange(x); dm[dm[x]] = vA; }
void inc(METADATA x)    { enrange(x); dm[x]++; }
void dec(METADATA x)    { enrange(x); dm[x]--; }
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
void fromb()  { vA = vB; }
void fromt()  { vA = vT; }
void fromr()  { vA = vR; }
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
#define v_pm 0
#define v_dm v_PM_WORDS
#define v_rPC v_dm + v_DM_WORDS
#define v_vA v_rPC + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vR v_vL + 1
#define v_vT v_vR + 1
#define v_src v_vT + 1
#define v_dst v_vT + 1
vm_init:
  branch(setupToClearParent);
  branch(doFill);
end:
  halt(SUCCESS);

// Setup to doFill so as to clear entire data memory of parent
setupToClearParent:
  lit(0);
  to(v_dst);
  times(DM_WORDS);
  merge

// Fill vR words at v_dst with value in vA.
// (Note: this will fill 1 GB in about 0.5 seconds)
doFill:
  put(v_dst);
  inc(v_dst);
  repeat(doFill)
  merge

} // end ofexampleProgram

