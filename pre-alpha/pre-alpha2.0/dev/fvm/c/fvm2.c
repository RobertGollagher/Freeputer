/* ===========================================================================
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180502+
Version:    pre-alpha-0.0.8.7+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  This is the new 4-stack-machine design outlined in 'pre-alpha2.0/README.md'.
  WARNING: all instructions have now been implemented but
  most instructions are as yet completely untested.

  Experiments:
    - in, out as completely blocking forever (therefore no branch-on-failure)
    - no catch instruction (incompatible with fast native implementation)
    - replace catch with safe instruction
    - for arithmetic, the two viable overflow strategies are trap or NaN,
      but the former becomes extremely complex with respect to prevention
      unless as prevention you are willing to resort to masking;
      accordingly this implementation shall use NaN until such time
      as it proves to be impractical, in which case it shall trap.

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
//#define TRACING_SUPPORTED // Uncomment this line to support tracing
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
#define WIDE int64_t  // Word type used for widening during arithmetic
#define WIDE_MASK     0xffffffff00000000
#define WD_BYTES 4
#define METADATA_MASK 0x7fffffff // 31 bits
#define FLIP_MASK     0x80000000
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
#define NaN 0x80000000
#define WORD_MAX 0x7fffffff
#define WORD_MIN 0x80000001 

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
void exampleProgram();
static jmp_buf exc_env;
static int excode;

// ---------------------------------------------------------------------------
// Mask logic
// ---------------------------------------------------------------------------
WORD dmsafe(WORD addr) { return addr & DM_MASK; }
WORD enbyte(WORD x)  { return x & BYTE_MASK; }
WORD enrange(WORD x) { return x & METADATA_MASK; }
WORD enshift(WORD x) { return x & SHIFT_MASK; }
WORD rmsafe(WORD addr) { return addr & RM_MASK; }
WORD hdsafe(WORD addr) { return addr & HD_MASK; }

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
    WORD wd = s->elem[(s->sp)-index];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeekAndDec(WDSTACK *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    if (wd == 0 || wd ==NaN) {
      return wd;
    } else {
      --(s->elem[(s->sp)-1]);
      return wd-1;
    }
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPokeAt(WORD x, WDSTACK *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    s->elem[(s->sp)-index] = x;
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
WORD wdDup(WDSTACK *s) {
  wdPush(wdPeek(s),s);
}
WORD wdOver(WDSTACK *s) {
  wdPush(wdPeekAt(s,2),s);
}
WORD wdSwap(WDSTACK *s) {
  WORD n1 = wdPeekAt(s,1);
  WORD n2 = wdPeekAt(s,2);
  wdPokeAt(n1,s,2);
  wdPokeAt(n2,s,1);
}
WORD wdRot(WDSTACK *s) {
  WORD n1 = wdPeekAt(s,1);
  WORD n2 = wdPeekAt(s,2);
  WORD n3 = wdPeekAt(s,3);
  wdPokeAt(n1,s,2);
  wdPokeAt(n2,s,3);
  wdPokeAt(n3,s,1);
}
int wdElems(WDSTACK *s) {return (s->sp);}
int wdFree(WDSTACK *s) {return STACK_MAX_INDEX - (s->sp);}
#ifdef TRACING_SUPPORTED
  char* wsTrace(WDSTACK *s) {
    if (wdElems(s) == 0) {
      return "";
    } else { // TODO show more elems
      sprintf(s->traceBuf, "%08x..",wdPeekAt(s,1));
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
WORD natPeekAt(NATSTACK *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    NAT nat = s->elem[(s->sp)-index];
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
    } else { // TODO show more elems
      sprintf(s->traceBuf, "*%08x..",natPeekAt(s,1));
      return s->traceBuf;
    }
  }
#endif

// ---------------------------------------------------------------------------
// FVM structure -- other structures can be logically equivalent
// ---------------------------------------------------------------------------
typedef struct Fvm {
  WORD dm[DM_WORDS]; // Harvard RAM data memory
  WORD rm[RM_WORDS]; // Harvard ROM data memory
  WORD hd[HD_WORDS]; // hold
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
void Noop()   { TRC("noop ") __asm(nopasm); }
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
void Tpoke()  { TRC("tpoke") 
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n1 > wdElems(&fvm.ts)) {
    longjmp(exc_env, FAILURE);
  } else {
    wdPokeAt(n2,&fvm.ts,n1);
  }
}
void Tdrop()  { TRC("tdrop") wdDrop(&fvm.ts); }
void Lit(WORD x) { TRC("lit  ") wdPush(x&METADATA_MASK,&fvm.ds); }
void Drop()   { TRC("drop ") wdDrop(&fvm.ds); }
void Swap()   { TRC("swap ") wdSwap(&fvm.ds); }
void Over()   { TRC("over ") wdOver(&fvm.ds); }
void Rot()    { TRC("rot  ") wdRot(&fvm.ds); }
void Dup()    { TRC("dup  ") wdDup(&fvm.ds); }
// Traps if address NaN or outside address space
void Get()    { TRC("get  ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    wdPush(fvm.dm[n1],&fvm.ds);
  }
}
// Traps if address NaN or outside address space
void Put()    { TRC("put  ")
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    fvm.dm[n1] = n2;
  }
}
// Traps if address NaN or outside address space
void Geti()   { TRC("geti ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    WORD got = fvm.dm[n1];
    if (got == NaN || dmsafe(got) != n1) {
      longjmp(exc_env, FAILURE);
    } else {
      wdPush(fvm.dm[got],&fvm.ds);  
    }
  }
}
// Traps if address NaN or outside address space
void Puti()   { TRC("puti ")
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    WORD got = fvm.dm[n1];
    if (got == NaN || dmsafe(got) != n1) {
      longjmp(exc_env, FAILURE);
    } else {
      fvm.dm[got] = n2;
    }
  }
}
// Traps if address NaN or outside address space
void Rom()    { TRC("rom  ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || rmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    wdPush(fvm.rm[n1],&fvm.ds);
  }
}
void Add()    { TRC("add  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WIDE wide = n2+n1;
      if (wide&WIDE_MASK) {
        wdPush(NaN,&fvm.ds);
      } else {
        wdPush((WORD)wide,&fvm.ds);
      }
  }
}
void Sub()    { TRC("sub  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WIDE wide = n2-n1;
      if (wide&WIDE_MASK) {
        wdPush(NaN,&fvm.ds);
      } else {
        wdPush((WORD)wide,&fvm.ds);
      }
  }
}
void Mul()    { TRC("mul  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WIDE wide = n2*n1;
      if (wide&WIDE_MASK) {
        wdPush(NaN,&fvm.ds);
      } else {
        wdPush((WORD)wide,&fvm.ds);
      }
  }
}
void Div()    { TRC("div  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN || n1 == 0) {
    wdPush(NaN,&fvm.ds);
  } else {
      WIDE wide = n2/n1;
      if (wide&WIDE_MASK) {
        wdPush(NaN,&fvm.ds);
      } else {
        wdPush((WORD)wide,&fvm.ds);
      }
  }
}
void Mod()    { TRC("mod  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN || n1 == 0) {
    wdPush(NaN,&fvm.ds);
  } else {
      WIDE wide = n2%n1;
      if (wide&WIDE_MASK) {
        wdPush(NaN,&fvm.ds);
      } else {
        wdPush((WORD)wide,&fvm.ds);
      }
  }
}
void Inc()    { TRC("inc  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || n1 == WORD_MAX) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = ++n1;
      wdPush(res,&fvm.ds);
  }
}
void Dec()    { TRC("dec  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || n1 == WORD_MIN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = --n1;
      wdPush(res,&fvm.ds);
  }
}
void Or()     { TRC("or   ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2|n1,&fvm.ds);
  }
}
void And()    { TRC("and  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2&n1,&fvm.ds);
  }
}
void Xor()    { TRC("xor  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2^n1,&fvm.ds);
  }
}
void Flip()   { TRC("flip ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = n1^FLIP_MASK;
      wdPush(res,&fvm.ds);
  }
}
void Neg()    { TRC("neg  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = (~n1)+1;
      wdPush(res,&fvm.ds);
  }
}
void Shl()    { TRC("shl  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN || n1^SHIFT_MASK != 0) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2<<n1,&fvm.ds);
  }
}
void Shr()    { TRC("shr  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN || n1^SHIFT_MASK != 0) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2>>n1,&fvm.ds);
  }
}
// Traps if address NaN or outside address space
void Hold()   { TRC("hold ")
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || hdsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    fvm.hd[n1] = n2;
  }
}
// Traps if address NaN or outside address space
void Give()   { TRC("give ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || hdsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    wdPush(fvm.hd[n1],&fvm.ds);
  }
}
void In()     { TRC("in   ") wdPush(getchar(), &fvm.ds); }
void Out()    { TRC("out  ") putchar(wdPop(&fvm.ds)); }
// Jump
// Jnan
// Jnnz
// Jnnp
// Jnne
// Jnng
// Jnnl
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
// Programming-language macros
// ---------------------------------------------------------------------------
#define noop Noop();
#define call(label) { __label__ lr; \
  TRC("call ") natPush((NAT)&&lr,&fvm.rs); goto label; lr: ; \
}
#define ret { TRC("ret  ") goto *(natPop(&fvm.rs)); }
// Repeat if decremented counter not NaN and > 0
#define rpt(label) { TRC("rpt  ") \
  WORD n1 = wdPeekAndDec(&fvm.cs); if ((n1 != NaN) && n1 > 0)  goto label; }
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
// Jump if n is NaN. Preserve n.
#define jnan(label) { TRC("jnan ") \
  if (wdPeek(&fvm.ds) == NaN) goto label; }
// Jump if n is 0. Preserve n.
#define jnnz(label) { TRC("jnnz ") \
  if (wdPeek(&fvm.ds) == 0) goto label; }
// Jump if n > 0. Preserve n.
#define jnnp(label) { TRC("jnnp ") \
  WORD n1 = wdPeek(&fvm.ds); \
  if ((n1 != NaN) && (n1 > 0)) goto label; }
// Jump if n2 == n1 and neither are NaN. Preserve n2.
#define jnne(label) { TRC("jnne ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPeek(&fvm.ds); \
  if ((n1 != NaN) && (n2 != NaN) && (n1 == n2)) goto label; }
// Jump if n2 > n1 and neither are NaN. Preserve n2.
#define jnng(label) { TRC("jnne ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPop(&fvm.ds); \
  if ((n1 != NaN) && (n2 != NaN) && (n1 < n2)) goto label; }
// Jump if n2 < n1 and neither are NaN. Preserve n2.
#define jnnl(label) { TRC("jnne ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPop(&fvm.ds); \
  if ((n1 != NaN) && (n2 != NaN) && (n1 > n2)) goto label; }
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

  // This is very fast indeed:
  //   64-bit gcc -O3 does 7fffffff loops in 4.0 secs with the noop
  //                                      or 2.0 secs without the noop.
  //   Note: without gcc -O3 it is 17.9 and 16.9 secs respectively.
  i(0x7fffffff)
  cpush
  loop:
    noop
    rpt(loop)                               
  halt
 
/*
  // 6.9 secs 64-bit gcc -O3 (or 2.0 seconds without the noop).
  // Note: 37.9 secs 64-bit without gcc -O3!
  i(0x7fffffff)
  loop:
    dec
    jnnp(loop)
  done:
    halt
*/

/*
  // 6.9 secs 64-bit gcc -O3 (or 2.0 seconds without the noop).
  i(0x7fffffff)
  loop:
    dec
    noop
    jnnz(done)
    jump(loop)
  done:
    halt
*/

}
// ===========================================================================
