/* ===========================================================================
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180501+
Version:    pre-alpha-0.0.8.5+ for FVM 2.0
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
// Tracing
// ---------------------------------------------------------------------------
#define TRACING_SUPPORTED // Uncomment this line to support tracing
#ifdef TRACING_SUPPORTED
  #define TRC(mnem) if (fvm.tracing) { __label__ ip; ip: \
    fprintf(stdtrcHandle, "*%08x %s / %s / ( %s ) [ %s ] { %s } \n", \
      &&ip, mnem, wsTrace(&fvm.cs), wsTrace(&fvm.ds), wsTrace(&fvm.ts), nsTrace(&fvm.rs)); }
#else
  #define TRC(mnem)
#endif

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
#ifdef TRACING_SUPPORTED
  #define stdtrcFilename "std.trc"
  FILE *stdtrcHandle;
#endif
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
#define FALSE 0
#define TRUE 1
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
  #ifdef TRACING_SUPPORTED
    char traceBuf[20];
  #endif
} WDSTACK;
typedef struct NativeStack {
  BYTE sp;
  NAT elem[STACK_CAPACITY];
  #ifdef TRACING_SUPPORTED
    char traceBuf[20];
  #endif
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
WORD wdPeekAt(WDSTACK *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    WORD wd = s->elem[index-1];
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
#ifdef TRACING_SUPPORTED
  char* wsTrace(WDSTACK *s) {
    if (wdElems(s) == 0) {
      return "";
    } else { // FIXME TODO more elems
      sprintf(s->traceBuf, "%08x...",wdPeekAt(s,1));
      return s->traceBuf;
    }
  }
#endif

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
WORD nsPeekAt(NATSTACK *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    NAT nat = s->elem[index-1];
    return nat;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD natElems(NATSTACK *s) {return (s->sp);}
WORD natFree(NATSTACK *s) {return STACK_MAX_INDEX - (s->sp);}
#ifdef TRACING_SUPPORTED
  char* nsTrace(NATSTACK *s) {
    if (natElems(s) == 0) {
      return "";
    } else { // FIXME TODO more elems
      sprintf(s->traceBuf, "*%08x...",nsPeekAt(s,1));
      return s->traceBuf;
    }
  }
#endif

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
  #ifdef TRACING_SUPPORTED
    WORD tracing;
  #endif
} FVM;
// One global instance only (for program source-code portability)
FVM fvm;

// ---------------------------------------------------------------------------
// Instruction set
// ---------------------------------------------------------------------------
void Noop()    { __asm(nopasm); }
// Call
// Ret
// Rpt
void Cpush()  { TRC("cpush") wdPush(wdPop(&fvm.ds),&fvm.cs); }
void Cpop()   { TRC("cpop ") wdPush(wdPop(&fvm.cs),&fvm.ds); }
void Cpeek()  { TRC("cpeek") wdPush(wdPeek(&fvm.cs),&fvm.ds); }
void Cdrop()  { TRC("cdrop") wdDrop(&fvm.cs); }
void Tpush()  { TRC("tpush") wdPush(wdPop(&fvm.ds),&fvm.ts); }
void Tpop()   { TRC("tpop ") wdPush(wdPop(&fvm.ts),&fvm.ds); }
void Tpeek()  { TRC("tpeek") wdPush(wdPeek(&fvm.ts),&fvm.ds); }
void Tpoke()  { TRC("tpoke") ; }
void Tdrop()  { TRC("tdrop") wdDrop(&fvm.ts); }
void Lit(WORD x) { TRC("lit  ") wdPush(x&METADATA_MASK,&fvm.ds); }
void Drop()   { TRC("drop ") wdDrop(&fvm.ds); }
void Swap()   { TRC("swap ") ; }
void Over()   { TRC("over ") ; }
void Rot()    { TRC("rot  ") ; }
void Dup()    { TRC("dup  ") ; }
void Get()    { TRC("get  ") ; }
void Put()    { TRC("put  ") ; }
void Geti()   { TRC("geti ") ; }
void Puti()   { TRC("puti ") ; }
void Rom()    { TRC("rom  ") ; }
void Add()    { TRC("add  ") wdPush(wdPop(&fvm.ds)+wdPop(&fvm.ds),&fvm.ds); } // FIXME
void Sub()    { TRC("sub  ") ; }
void Mul()    { TRC("mul  ") ; }
void Div()    { TRC("div  ") ; }
void Mod()    { TRC("mod  ") ; }
void Inc()    { TRC("inc  ") ; }
void Dec()    { TRC("dec  ") ; }
void Or()     { TRC("or   ") ; }
void And()    { TRC("and  ") ; }
void Xor()    { TRC("xor  ") ; }
void Flip()   { TRC("flip ") ; }
void Neg()    { TRC("neg  ") ; }
void Shl()    { TRC("shl  ") ; }
void Shr()    { TRC("shr  ") ; }
void Hold()   { TRC("hold ") ; }
void Give()   { TRC("give ") ; }
void In()     { TRC("in   ") wdPush(getchar(), &fvm.ds); }
void Out()    { TRC("out  ") putchar(wdPop(&fvm.ds)); }
// Jump
// Jmpz
// Jmpe
// Jmpg
// Jmpl
// Halt
// Fail
// Safe
void Dsa()    { TRC("dsa  ") wdPush(wdElems(&fvm.ds),&fvm.ds); }
void Dse()    { TRC("dse  ") wdPush(wdFree(&fvm.ds),&fvm.ds); }
void Tsa()    { TRC("tsa  ") wdPush(wdElems(&fvm.ts),&fvm.ds); }
void Tse()    { TRC("tse  ") wdPush(wdFree(&fvm.ts),&fvm.ds); }
void Csa()    { TRC("csa  ") wdPush(wdElems(&fvm.cs),&fvm.ds); }
void Cse()    { TRC("cse  ") wdPush(wdFree(&fvm.cs),&fvm.ds); }
void Rsa()    { TRC("rsa  ") wdPush(natElems(&fvm.rs),&fvm.ds); }
void Rse()    { TRC("rse  ") wdPush(natFree(&fvm.rs),&fvm.ds); }
void Pmi()    { TRC("pmi  ") wdPush(PMI,&fvm.ds); }
void Dmw()    { TRC("dmw  ") wdPush(DM_WORDS,&fvm.ds); }
void Rmw()    { TRC("rmw  ") wdPush(RM_WORDS,&fvm.ds); }
void Hw()     { TRC("hw   ") wdPush(HD_WORDS,&fvm.ds); }
void Tron()   { TRC("tron ") 
  #ifdef TRACING_SUPPORTED
    fvm.tracing = TRUE;
  #endif
}
void Troff()  { TRC("troff") 
  #ifdef TRACING_SUPPORTED
    fvm.tracing = FALSE;
  #endif
}

// ---------------------------------------------------------------------------
// Programming language macros
// ---------------------------------------------------------------------------
#define noop Noop();
#define call(label) { __label__ lr; \
  TRC("call ") natPush((NAT)&&lr,&fvm.rs); goto label; lr: ; \
}
#define ret { TRC("ret  ") goto *(natPop(&fvm.rs)); }
#define rpt(label) { TRC("rpt ") if (wdPeekAndDec(&fvm.cs) > 0) goto label; }
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
#define jump(label) { TRC("jump ") goto label; }
#define jmpz(label) { TRC("jmpz ") if (wdPop(&fvm.ds) == 0) goto label; }
#define jmpe(label) { TRC("jmpe ") if (wdPop(&fvm.ds) == wdPop(&fvm.ds)) goto label; }
#define jmpg(label) { TRC("jmpg ") if (wdPop(&fvm.ds) < wdPop(&fvm.ds)) goto label; }
#define jmpl(label) { TRC("jmpl ") if (wdPop(&fvm.ds) > wdPop(&fvm.ds)) goto label; }
#define halt { TRC("halt ") excode = SUCCESS; return; }
#define fail { TRC("fail ") excode = FAILURE; return; }
#define safe(label) { TRC("safe ") if (wdElems(&fvm.ds) < 2) goto label; }
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
  #ifdef TRACING_SUPPORTED
    fvm->tracing = FALSE;
    stdtrcHandle = fopen(stdtrcFilename, "w");
    if (!stdtrcHandle) return FAILURE;
  #endif
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
  #ifdef TRACING_SUPPORTED
    if (fclose(stdtrcHandle) == EOF) shutdown = FAILURE;
  #endif
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

tron
call(x0)
halt

failed:
  fail

unsafe:
  i(0x41)
  out
  halt

x0:
  // safe(unsafe)
  i(0x11111111)
  cpush
  i(0x33333333)
  tpush
  i(0x22222222)
  safe(unsafe)
  add
  ret

}
// ===========================================================================
