/* ===========================================================================
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180501+
Version:    pre-alpha-0.0.8.4+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Gradually converting this old QMISC implementation to the new
  4-stack-machine design outlined in 'pre-alpha2.0/README.md'.
  Currently only partly converted, still very incomplete.

  Experiments:
    - in, out as completely blocking forever (therefore no branch-on-failure)
    - no catch instruction (incompatible with fast native implementation)
    - replace catch with safe instruction

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/

// ---------------------------------------------------------------------------
// Dependencies
// ---------------------------------------------------------------------------
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <setjmp.h>

// ---------------------------------------------------------------------------
// Platform-specific constants
// ---------------------------------------------------------------------------
#define nopasm "nop"  // The name of the native hardware nop instruction
#define NAT uintptr_t // Native pointer type for Harvard program memory

// ---------------------------------------------------------------------------
// Files -- other storage mechanisms can be logically equivalent
//
//   Blank std.hld and std.rom files can be created thus:
//     head -c 1024 /dev/zero > std.hld
//     head -c 1024 /dev/zero > std.rom
// ---------------------------------------------------------------------------
#define rmFilename "std.rom"
FILE *rmHandle;
#define stdhldFilename "std.hld"
FILE *stdhldHandle;

// ---------------------------------------------------------------------------
// Platform-independent constants
// ---------------------------------------------------------------------------
#define TRACING_ENABLED // Comment out unless debugging
#define BYTE uint8_t
#define WORD int32_t  // Word type for Harvard data memory
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
#define MAX_PMI 0x1000000 // <= 2^24 by design.
#define PMI MAX_PMI // Must be some power of 2 <= MAX_PMI.

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
void exampleProgram();
static jmp_buf exc_env;
static int excode;

// ---------------------------------------------------------------------------
// Mask logic
// ---------------------------------------------------------------------------
WORD safe(WORD addr) { return addr & DM_MASK; }
WORD enbyte(WORD x)  { return x & BYTE_MASK; }
WORD enrange(WORD x) { return x & METADATA_MASK; }
WORD enshift(WORD x) { return x & SHIFT_MASK; }
WORD romsafe(WORD addr)      { return addr & RM_MASK; }
WORD hdsafe(WORD addr)       { return addr & HD_MASK; }

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
// FVM structure -- other structures can be logically equivalent
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
FVM fvm; // One global instance only (for program source-code portability)

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------
void Dbg()  { ; }
#define dbg Dbg()

// ---------------------------------------------------------------------------
// Instruction set
// ---------------------------------------------------------------------------
void Noop()    { __asm(nopasm); }
// Call
// Ret
// Rpt
void Cpush()  { wdPush(wdPop(&fvm.ds),&fvm.cs); }
void Cpop()   { wdPush(wdPop(&fvm.cs),&fvm.ds); }
void Cpeek()  { wdPush(wdPeek(&fvm.cs),&fvm.ds); }
void Cdrop()  { wdDrop(&fvm.cs); }
void Tpush()  { wdPush(wdPop(&fvm.ds),&fvm.ts); }
void Tpop()   { wdPush(wdPop(&fvm.ts),&fvm.ds); }
void Tpeek()  { wdPush(wdPeek(&fvm.ts),&fvm.ds); }
void Tpoke()  { ; }
void Tdrop()  { wdDrop(&fvm.ts); }
void Lit(WORD x)    { wdPush(x&METADATA_MASK,&fvm.ds); }
void Drop()   { wdDrop(&fvm.ds); }
void Swap()   { ; }
void Over()   { ; }
void Rot()    { ; }
void Dup()    { ; }
void Get()    { ; }
void Put()    { ; }
void Geti()   { ; }
void Puti()   { ; }
void Rom()    { ; }
void Add()    { wdPush(wdPop(&fvm.ds)+wdPop(&fvm.ds),&fvm.ds); } // FIXME
void Sub()    { ; }
void Mul()    { ; }
void Div()    { ; }
void Mod()    { ; }
void Inc()    { ; }
void Dec()    { ; }
void Or()     { ; }
void And()    { ; }
void Xor()    { ; }
void Flip()   { ; }
void Neg()    { ; }
void Shl()    { ; }
void Shr()    { ; }
void Hold()   { ; }
void Give()   { ; }
void In()     { wdPush(getchar(), &fvm.ds); }
void Out()    { putchar(wdPop(&fvm.ds)); }
// Jump
// Jmpz
// Jmpe
// Jmpg
// Jmpl
// Halt
// Fail
// Safe
void Dsa()    { wdPush(wdElems(&fvm.ds),&fvm.ds); }
void Dse()    { wdPush(wdFree(&fvm.ds),&fvm.ds); }
void Tsa()    { wdPush(wdElems(&fvm.ts),&fvm.ds); }
void Tse()    { wdPush(wdFree(&fvm.ts),&fvm.ds); }
void Csa()    { wdPush(wdElems(&fvm.cs),&fvm.ds); }
void Cse()    { wdPush(wdFree(&fvm.cs),&fvm.ds); }
void Rsa()    { wdPush(natElems(&fvm.rs),&fvm.ds); }
void Rse()    { wdPush(natFree(&fvm.rs),&fvm.ds); }
void Pmi()    { wdPush(PMI,&fvm.ds); }
void Dmw()    { wdPush(DM_WORDS,&fvm.ds); }
void Rmw()    { wdPush(RM_WORDS,&fvm.ds); }
void Hw()     { wdPush(HD_WORDS,&fvm.ds); }
void Tron()   { ; }
void Troff()  { ; }

// ---------------------------------------------------------------------------
// Programming language macros
// ---------------------------------------------------------------------------
#define noop Noop();
#define call(label) { \
  __label__ lr; natPush((NAT)&&lr,&fvm.rs); goto label; lr: ; \
}
#define ret { goto *(natPop(&fvm.rs)); }
#define rpt(label) if (wdPeekAndDec(&fvm.cs) > 0) { goto label; }
#define cpush Cpush();
#define cpop Cpop();
#define cpeek Cpeek();
#define cdrop Cdrop();
#define tpush Tpush();
#define tpop Tpop();
#define tpeek Tpeek();
#define tpoke Tpoke();
#define tdrop Tdrop();
#define i(x) Lit(x);
#define drop Drop();
#define swap Swap();
#define over Over();
#define rot Rot();
#define dup Dup();
#define get Get();
#define put Put();
#define geti Geti();
#define puti Puti();
#define rom Rom();
#define add Add();
#define sub Sub();
#define mul Mul();
#define div Div();
#define mod Mod();
#define inc Inc();
#define dec Dec();
#define or Or() ;
#define and And();
#define xor Xor();
#define flip Flip();
#define neg Neg();
#define shl Shl();
#define shr Shr();
#define hold Hold();
#define give Give();
#define in In();
#define out Out();
#define jump(label) goto label;
#define jmpz(label) if (wdPop(&fvm.ds) == 0) { goto label; }
#define jmpe(label) if (wdPop(&fvm.ds) == wdPop(&fvm.ds)) { goto label; }
#define jmpg(label) if (wdPop(&fvm.ds) < wdPop(&fvm.ds)) { goto label; }
#define jmpl(label) if (wdPop(&fvm.ds) > wdPop(&fvm.ds)) { goto label; }
#define halt { excode = SUCCESS; return; }
#define fail { excode = FAILURE; return; }
#define safe(label) if (wdElems(&fvm.ds) < 2) {goto label; }
#define dsa Dsa();
#define dse Dse();
#define tsa Tsa();
#define tse Tse();
#define csa Csa();
#define cse Cse();
#define rsa Rsa();
#define rse Rse();
#define pmi Pmi();
#define dmw Dmw();
#define rmw Rmw();
#define hw Hw() ;
#define tron Tron();
#define troff Troff();

// ---------------------------------------------------------------------------
// I/O start-up 
// ---------------------------------------------------------------------------
int startup(FVM *fvm) {
  stdhldHandle = fopen(stdhldFilename, "r+b");
  if (!stdhldHandle) return FAILURE;
  if (fread(fvm->hd,WD_BYTES,HD_WORDS,stdhldHandle) < HD_WORDS) {
    fclose(stdhldHandle);
    return FAILURE;
  }
  rmHandle = fopen(rmFilename, "rb");
  if (!rmHandle) {
    fclose(stdhldHandle);
    return FAILURE;
  }
  if (fread(fvm->rm,WD_BYTES,RM_WORDS,rmHandle) < RM_WORDS) {
    fclose(rmHandle);
    fclose(stdhldHandle);
    return FAILURE;
  }
  return SUCCESS;
}

// ---------------------------------------------------------------------------
// I/O shutdown 
// --------------------------------------------------------------------------
int shutdown(FVM *fvm) {
  int shutdown = SUCCESS;
  if (fclose(rmHandle) == EOF) shutdown = FAILURE;
  if (fseek(stdhldHandle,0,SEEK_SET) !=0) {
    shutdown = FAILURE;
  } else {
    if (fwrite(fvm->hd,WD_BYTES,HD_WORDS,stdhldHandle) < HD_WORDS) {
      shutdown = FAILURE;
    }
  }
  if (fclose(stdhldHandle) == EOF) shutdown = FAILURE;
  return shutdown;
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  assert(PMI <= MAX_PMI);
  assert(DM_WORDS <= MAX_DM_WORDS);
  assert(RM_WORDS <= MAX_RM_WORDS);
  assert(HD_WORDS <= MAX_HD_WORDS);
  assert(startup(&fvm) == SUCCESS);
  if (!setjmp(exc_env)) {
    exampleProgram();
    assert(shutdown(&fvm) == SUCCESS);
    return excode;
  } else {
    return FAILURE;
  }
}

// ---------------------------------------------------------------------------
// Program
// ---------------------------------------------------------------------------
void exampleProgram() {

jump(x0)

failed:
  fail

unsafe:
  i(0x41)
  out
  halt

x0:
  safe(unsafe)
  add
  halt

}
// ===========================================================================
