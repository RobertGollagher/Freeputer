/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180428+
Version:    pre-alpha-0.0.7.0+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  TODO Reconsider alternate arith strategy eg NaN/wrap/unsigned/other
  mainly since could simplify VM and avoid undef behav. Consider JS also.

  Removed most notes so as not to prejudice lateral thinking during design.

  WORDs are unsigned to avoid undefined behaviour in C!

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
#define RS_UNDERFLOW 33
#define RS_OVERFLOW 34
#define TS_UNDERFLOW 35
#define TS_OVERFLOW 36
#define CS_UNDERFLOW 37
#define CS_OVERFLOW 38

// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Logic for stacks
// ---------------------------------------------------------------------------
#define STACK_CAPACITY 256
#define STACK_MAX_INDEX 255
typedef struct WordStack {
  BYTE sp;
  WORD elem[STACK_CAPACITY];
} WDSTACK;
typedef struct NativeStack {
  BYTE sp;
  NAT elem[STACK_CAPACITY];
} NATSTACK;

void wdPush(WORD x, WDSTACK *s, int error_code) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
  }
}
WORD wdPop(WDSTACK *s, int error_code) {
  if ((s->sp)>0) {
    WORD wd = s->elem[--(s->sp)];
    return wd;
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
  }
}
WORD wdDrop(WDSTACK *s, int error_code) {
  if ((s->sp)>0) {
    --(s->sp);
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
  }
}
WORD wdPeek(WDSTACK *s, int error_code) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    return wd;
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
  }
}
WORD wdPeekAndDec(WDSTACK *s, int error_code) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    if (wd == 0) {
      return wd; // Already 0, do not decrement further
    } else {
      --(s->elem[(s->sp)-1]);
      return wd-1;
    }
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
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
NAT natPop(NATSTACK *s, int error_code) {
  if ((s->sp)>0) {
    NAT nat = s->elem[--(s->sp)];
    return nat;
  } else {
    exc_code = error_code;
    longjmp(exc_env, exc_code);
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
// Utilities
// ---------------------------------------------------------------------------
void dsTopTrace() {
  printf("%x:",wdPeek(&ds,DS_UNDERFLOW));
}
#define tr dsTopTrace();
// ---------------------------------------------------------------------------
// Immediates
void i(METADATA x) {  // bits 31..0
  wdPush(enrange(x), &ds, DS_OVERFLOW);
}
// Metadata
void Dsa()    { wdPush(wdFree(&ds), &ds, DS_OVERFLOW); }
void Dse()    { wdPush(wdElems(&ds), &ds, DS_OVERFLOW); }
void Tsa()    { wdPush(wdFree(&ts), &ds, TS_OVERFLOW); }
void Tse()    { wdPush(wdElems(&ts), &ds, TS_OVERFLOW); }
void Csa()    { wdPush(wdFree(&cs), &ds, CS_OVERFLOW); }
void Cse()    { wdPush(wdElems(&cs), &ds, CS_OVERFLOW); }
void Rsa()    { wdPush(natFree(&rs), &ds, RS_OVERFLOW); }
void Rse()    { wdPush(natElems(&rs), &ds, RS_OVERFLOW); }

// Stack operations
void Drop()   { wdDrop(&ds, DS_UNDERFLOW); }

// Arithmetic
void Add()    { wdPush(wdPop(&ds, DS_UNDERFLOW) + wdPop(&ds, DS_UNDERFLOW), &ds, DS_OVERFLOW); } // FIXME
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

void Flip()             { /*FIXME vB = vB^MSb;*/ }     // bit  32 (NOT might be better)

// Machine metadata
void Mdm()    { /*FIXME vA = DM_WORDS;*/ }
// Other
#define nopasm "nop" // The name of the native hardware nop instruction
void Noop()   { __asm(nopasm); }
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

#define rpt(label) \
if (wdPeekAndDec(&cs, CS_UNDERFLOW) != 0) { \
  goto label; \
} else { \
  wdDrop(&cs, CS_UNDERFLOW); \
}

#define dsa Dsa();
#define dse Dse();
#define tsa Tsa();
#define tse Tse();
#define csa Csa();
#define cse Cse();
#define rsa Rsa();
#define rse Rse();
#define add Add();
#define drop Drop();

void ToCS() {
  WORD wd = wdPop(&ds, DS_UNDERFLOW);
  wdPush(wd, &cs, CS_OVERFLOW);
}

void In() {
  WORD wd = getchar();  // FIXME
  wdPush(wd, &ds, DS_OVERFLOW);
}

void Out() {
  WORD wd = wdPop(&ds, DS_UNDERFLOW);
  putchar(wd); // FIXME
}

#define in In();
#define out Out();

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
/*
  // Performance test, 0x7fffffff repeats: 2.1/4.1 s without/with Noop()
  i(0x7fffffff);
  ToCS();
  perfLoop:
    //Noop();
    rpt(perfLoop)
*/

  i(1000000);
  ToCS();
  loop:
    in
    out
    rpt(loop)

}
// ===========================================================================


