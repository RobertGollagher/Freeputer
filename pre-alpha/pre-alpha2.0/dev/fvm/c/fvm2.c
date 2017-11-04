/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20171104+
Version:    pre-alpha-0.0.6.2+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.

  Currently in the process of converting this from a register machine
  to a stack machine with 4 stacks: ds, ts, rs, cs as per recent experiments
  with 'fvm2.js' but here in a native Harvard implementation for simplicity.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#define TRACING_ENABLED // Comment out unless debugging

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <setjmp.h>
#define BYTE uint8_t
#define WORD uint32_t // Word type for Harvard data memory
#define NAT uintptr_t // Native pointer type for Harvard program memory
#define WD_BYTES 4
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define MAX_DM_WORDS 0x10000000 // <= 2^28 due to C limitations
#define DM_WORDS  0x10000  // Must be some power of 2 <= MAX_DM_WORDS
#define DM_MASK   DM_WORDS-1
WORD dm[DM_WORDS]; // Harvard data memory
int exampleProgram();
static jmp_buf exc_env;
static int exc_code;
#define DS_UNDERFLOW 31
#define DS_OVERFLOW 32
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Logic for stacks
// ---------------------------------------------------------------------------
typedef enum maybe { SOMETHING=1, NOTHING=0 } Maybe;
#define STACK_CAPACITY 256
#define STACK_MAX_INDEX 255
typedef struct MaybeWord {
  Maybe m;
  WORD wd;
} MWD;
typedef struct MaybeNat {
  Maybe m;
  NAT nat;
} MNAT;
typedef struct WordStack {
  BYTE sp;
  WORD elem[STACK_CAPACITY];
  MWD mwd; // Used by wdPop function
} WDSTACK;
typedef struct NativeStack {
  BYTE sp;
  NAT elem[STACK_CAPACITY];
  MNAT mnat; // Used by natPop function
} NATSTACK;

WORD wdPush(WORD x, WDSTACK *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
    return SUCCESS;
  } else {
    return FAILURE;
  }
}
MWD wdPop(WDSTACK *s) {
  if ((s->sp)>0) {
    s->mwd.m = SOMETHING;
    s->mwd.wd = s->elem[--(s->sp)];
    return s->mwd;
  } else {
    s->mwd.m = NOTHING;
    s->mwd.wd = 0;
    return s->mwd;
  }
}
int wdElems(WDSTACK *s) {return (s->sp);}
int wdFree(WDSTACK *s) {return STACK_MAX_INDEX - (s->sp);}

WORD natPush(NAT x, NATSTACK *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
    return SUCCESS;
  } else {
    return FAILURE;
  }
}
MNAT natPop(NATSTACK *s) {
  if ((s->sp)>0) {
    s->mnat.m = SOMETHING;
    s->mnat.nat = s->elem[--(s->sp)];
    return s->mnat;
  } else {
    s->mnat.m = NOTHING;
    s->mnat.nat = 0;
    return s->mnat;
  }
}
WORD natElems(NATSTACK *s) {return (s->sp);}
WORD natFree(NATSTACK *s) {return STACK_MAX_INDEX - (s->sp);}
// ---------------------------------------------------------------------------
// Stack declarations
// ---------------------------------------------------------------------------
WDSTACK ds, ts, cs; // data stack, temporary stack, counter stack
NATSTACK rs; // return stack
// ---------------------------------------------------------------------------
// CURRENTLY 34+ OPCODES
// Arithmetic
void Add()    { /*FIXME vA+=vB;*/ }
void Sub()    { /*FIXME vA-=vB;*/ }
// Logic
void Or()     { /*FIXME vA|=vB;*/ }
void And()    { /*FIXME vA&=vB;*/ }
void Xor()    { /*FIXME vA^=vB;*/ } // Maybe add NOT and NEG too
// Shifts
void Shl()    { /*FIXME vA<<=enshift(vB);*/ }
void Shr()    { /*FIXME vA>>=enshift(vB);*/ }
// Moves
void Get()    { /*FIXME vA = dm[safe(vB)];*/ }
void Put()    { /*FIXME dm[safe(vB)] = vA;*/ }

void Geti()   { /*FIXME WORD sB = safe(vB); vA = dm[safe(dm[sB])];*/ }
void Puti()   { /*FIXME WORD sB = safe(vB); dm[safe(dm[sB])] = vA;*/ } // untested

void Decm()   { /*FIXME --dm[safe(vB)];*/ }
void Incm()   { /*FIXME ++dm[safe(vB)];*/ }

void At()     { /*FIXME vB = dm[safe(vB)];*/ }
void Copy()   { /*FIXME dm[safe(vB+vA)] = dm[safe(vB)];*/ } // a smell?
// Increments for addressing
void Inc()    { /*FIXME ++vB;*/ }
void Dec()    { /*FIXME --vB;*/ }
// Immediates
void Imm(METADATA x)    {  // bits 31..0
  WORD outcome = wdPush(enrange(x), &ds);
  if (outcome) {
    exc_code = DS_OVERFLOW;
    longjmp(exc_env, exc_code);
  }
}
void Flip()             { /*FIXME vB = vB^MSb;*/ }     // bit  32 (NOT might be better)

// Machine metadata
void Mdm()    { /*FIXME vA = DM_WORDS;*/ }
// Other
void Noop()   { /*FIXME ;*/ }
/*FIXME
#define halt return enbyte(vA);
// Jumps (static only), maybe reduce these back to jump and jmpe only
#define jmpa(label) if (vA == 0) { goto label; } // vA is 0
#define jmpb(label) if (vB == 0) { goto label; } // vB is 0
#define jmpe(label) if (vA == vB) { goto label; } // vA equals vB
#define jmpn(label) if (MSb == (vA&MSb)) { goto label; } // MSb set in vA
#define jmps(label) if (vB == (vA&vB)) { goto label; } // all vB 1s set in vA
#define jmpu(label) if (vB == (vA|vB)) { goto label; } // all vB 0s unset in vA
#define jump(label) goto label; // UNCONDITIONAL
#define rpt(label) if ( vR != 0) { --vR; goto label; }
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// Basic I/O (experimental)
#define in(label) vA = getchar(); // If fail goto label
*/
void Out() {
  MWD mwd = wdPop(&ds);
  if(mwd.m) {
    putchar(mwd.wd); // FIXME
  } else {
    exc_code = DS_UNDERFLOW;
    longjmp(exc_env, exc_code);
  }
}
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  if (!setjmp(exc_env)) {
    exampleProgram();
    return SUCCESS;
  } else {
    return exc_code;
  }

}
// ===========================================================================
int exampleProgram() {
  Imm(10);
  Imm(65);
  Out();
  Out();
}
// ===========================================================================


