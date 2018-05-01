/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180501+
Version:    pre-alpha-0.0.8.0+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Gradually converting this old QMISC implementation to the new
  4-stack-machine design outlined in 'pre-alpha2.0/README.md'.
  Currently only partly converted, still very incomplete.

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
#define WORD int32_t  // Word type for Harvard data memory
#define NAT uintptr_t // Native pointer type for Harvard program memory
#define WD_BYTES 4
#define METADATA_MASK 0x7fffffff // 31 bits
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define STACK_CAPACITY 256
#define STACK_MAX_INDEX STACK_CAPACITY-1
#define MAX_DM_WORDS 0x10000000 // <= 2^28 due to C limitations.
#define DM_WORDS  0x100  // Must be some power of 2 <= MAX_DM_WORDS.
#define DM_MASK   DM_WORDS-1
#define MAX_RM_WORDS 0x10000000 // <= 2^28 due to C limitations.
#define RM_WORDS  0x100  // Must be some power of 2 <= MAX_RM_WORDS.
#define RM_MASK   RM_WORDS-1
#define MAX_HD_WORDS 0x10000000 // <= 2^28 due to C limitations.
#define HD_WORDS  0x100  // Must be some power of 2 <= MAX_HD_WORDS.
#define HD_MASK   HD_WORDS-1
int exampleProgram();
static jmp_buf exc_env;
static int excode;
// ---------------------------------------------------------------------------
WORD safe(WORD addr) { return addr & DM_MASK; }
WORD enbyte(WORD x)  { return x & BYTE_MASK; }
WORD enrange(WORD x) { return x & METADATA_MASK; }
WORD enshift(WORD x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Stack logic
// ---------------------------------------------------------------------------
typedef struct WordStack {
  BYTE sp;
  WORD elem[STACK_CAPACITY];
} WDSTACK;
typedef struct NativeStack {
  BYTE sp;
  NAT elem[STACK_CAPACITY];
} NATSTACK;
// ---------------------------------------------------------------------------
void wdPush(WORD x, WDSTACK *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPop(WDSTACK *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[--(s->sp)];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdDrop(WDSTACK *s) {
  if ((s->sp)>0) {
    --(s->sp);
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeek(WDSTACK *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeekAndDec(WDSTACK *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    if (wd == 0) {
      return wd; // Already 0, do not decrement further
    } else {
      --(s->elem[(s->sp)-1]);
      return wd-1;
    }
  } else {
    longjmp(exc_env, FAILURE);
  }
}
int wdElems(WDSTACK *s) {return (s->sp);}
int wdFree(WDSTACK *s) {return STACK_MAX_INDEX - (s->sp);}
// ---------------------------------------------------------------------------
WORD natPush(NAT x, NATSTACK *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
    return SUCCESS;
  } else {
    return FAILURE;
  }
}
NAT natPop(NATSTACK *s) {
  if ((s->sp)>0) {
    NAT nat = s->elem[--(s->sp)];
    return nat;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD natElems(NATSTACK *s) {return (s->sp);}
WORD natFree(NATSTACK *s) {return STACK_MAX_INDEX - (s->sp);}
// ---------------------------------------------------------------------------
// FVM structure -- other implementations are possible,
//   this one is for a native implementation with complete caches.
// ---------------------------------------------------------------------------
typedef struct Fvm {
  WORD dm[DM_WORDS]; // RAM data memory
  WORD rm[RM_WORDS]; // ROM data memory
  WORD hd[HD_WORDS]; // Hold memory
  WDSTACK ds; // data stack
  WDSTACK ts; // temporary stack
  WDSTACK cs; // counter stack
  NATSTACK rs; // return stack
} FVM;
FVM fvm;
// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Immediates
void i(WORD x) {  // bits 31..0
  /* wdPush(enrange(x), &ds); */
}
// Metadata
void Dsa()    { /*wdPush(wdFree(&ds), &ds); */}
void Dse()    { /*wdPush(wdElems(&ds), &ds); */}
void Tsa()    { /*wdPush(wdFree(&ts), &ds); */}
void Tse()    { /*wdPush(wdElems(&ts), &ds); */}
void Csa()    { /*wdPush(wdFree(&cs), &ds); */}
void Cse()    { /*wdPush(wdElems(&cs), &ds); */}
void Rsa()    { /*wdPush(natFree(&rs), &ds); */}
void Rse()    { /*wdPush(natElems(&rs), &ds); */}

// Stack operations
void Drop()   { /* wdDrop(&ds); */}

// Arithmetic
void Add()    { /*wdPush(wdPop(&ds) + wdPop(&ds), &ds); */} // FIXME
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
#define halt   { return SUCCESS; }
#define fail  { return FAILURE; }
/*FIXME
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
/*
#define rpt(label) \
if (wdPeekAndDec(&cs, FAILURE) != 0) { \
  goto label; \
} else { \
  wdDrop(&cs, FAILURE); \
}
*/
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
#define put Put();
void ToCS() {
/*
  WORD wd = wdPop(&ds);
  wdPush(wd, &cs, FAILURE);
*/
}

void In() {
/*
  WORD wd = getchar();  // FIXME
  wdPush(wd, &ds);
*/
}

void Out() {
/*
  WORD wd = wdPop(&ds);
  putchar(wd); // FIXME
*/
}

#define in In();
#define out Out();

// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  assert(DM_WORDS <= MAX_DM_WORDS);
  if (!setjmp(exc_env)) {
    exampleProgram();
    return SUCCESS;
  } else {
    return FAILURE;
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

  halt

}
// ===========================================================================


