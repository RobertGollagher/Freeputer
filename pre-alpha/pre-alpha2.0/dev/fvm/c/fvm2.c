/* ===========================================================================
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    fvm2.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180512+
Version:    pre-alpha-0.0.8.26+ for FVM 2.0
=======

                              This Edition:
                             Portable C for

        1. FVMP_STDIO: gcc using <stdio.h> (eg Linux targets) 
        2. FVMP_ARDUINO_IDE: Arduino IDE (eg Arduino or chipKIT targets)

                               ( ) [ ] { }

  This is the new 4-stack-machine design outlined in 'pre-alpha2.0/README.md'.
  WARNING: all instructions have now been implemented but
  most instructions are as yet completely untested.

  See 'exampleProgram.fp2' for build instructions.

  Experiments:
    - now using stderr for tracing output
    - added inw, outw instructions (word I/O)
    - stdin, stdout completely blocking forever (no branch-on-failure)
    - no catch instruction (incompatible with fast native implementation)
    - use NaN scheme rather than trapping (better for self-virtualization)
    - FVM 2.0 shall be BIG-ENDIAN 'everywhere' (the opposite of FVM 1.0)
      since this means memory, the hold, rom and output piped to or from
      external files will all be human-readable in exactly the same
      format. The endianness of elements on the stack is private;
      thus hardware computation can be little-endian.

  Currently experimenting with language format and adding interpreter.

  TODO NEXT test interpreted version on Arduino.
  Then expand interpreter to support the full instruction set.
  Then port to JavaScript and from there to Node.js.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/

// ---------------------------------------------------------------------------
// Choose target platform
// ---------------------------------------------------------------------------
#define FVMP_STDIO 0        // gcc using <stdio.h>
#define FVMP_ARDUINO_IDE 1  // Arduino IDE
#define FVMP FVMP_STDIO

// ---------------------------------------------------------------------------
// Choose implementation type
// ---------------------------------------------------------------------------
#define FVMI_NATIVE 0       // Run as native code
#define FVMI_INTERPRETED 1  // Run as an interpreter
#define FVMI FVMI_INTERPRETED
#if FVMI != FVMI_NATIVE && FVMI != FVMI_INTERPRETED
  #pragma GCC error "Problem: Invalid implementation type for compilation.\n \
  Solution: You must set FVMI to FVMI_NATIVE or FVMI_INTERPRETED.\n \
  Details: See 'fvm2.c' source."
#endif

// ---------------------------------------------------------------------------
// Dependencies
// ---------------------------------------------------------------------------
#if FVMP == FVMP_STDIO
  #include <stdio.h>
#elif FVMP == FVMP_ARDUINO_IDE
  #include <Arduino.h>
  /*int getchar() { return Serial.read(); }
  int putchar(int c) { Serial.write(c); }*/
#else
  #pragma GCC error "Problem: Invalid target platform for compilation.\n \
  Solution: You must set FVMP to FVMP_STDIO or FVMP_ARDUINO_IDE.\n \
  Details: See 'fvm2.c' source."
#endif

#include <inttypes.h>
#include <assert.h>
#include <setjmp.h>

// ---------------------------------------------------------------------------
// Tracing
// ---------------------------------------------------------------------------
#define TRACING_SUPPORTED // Uncomment this line to support tracing
#if FVMI == FVMI_NATIVE
  #ifdef TRACING_SUPPORTED
    #define TRC(mnem) if (fvm.tracing) { __label__ ip; ip: \
      fprintf(stderr, "*%08x %s / %s / ( %s ) [ %s ] { %s } \n", \
        &&ip, mnem, wsTrace(&fvm.cs), wsTrace(&fvm.ds), \
        wsTrace(&fvm.ts), nsTrace(&fvm.rs)); }
  #else
    #define TRC(mnem)
  #endif
#elif FVMI == FVMI_INTERPRETED
  #ifdef TRACING_SUPPORTED
    #define TRC(mnem) if (fvm.tracing) {  \
      fprintf(stderr, "%08x %s / %s / ( %s ) [ %s ] { %s } \n", \
        fvm.pc, mnem, wsTrace(&fvm.cs), wsTrace(&fvm.ds), \
        wsTrace(&fvm.ts), nsTrace(&fvm.rs)); }
  #else
    #define TRC(mnem)
  #endif
#endif

// ---------------------------------------------------------------------------
// Platform-independent constants
// ---------------------------------------------------------------------------
#define FALSE 0
#define TRUE 1
#define BYTE uint8_t
#define WORD int32_t  // Word type for Harvard data memory
#define WIDE int64_t  // Word type used for widening during arithmetic
#define WIDE_MASK     0xffffffff00000000
#define WD_BYTES 4
#define IMM_MASK      0x7fffffff // 31 bits
#define FLIP_MASK     0x80000000
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2 // FIXME
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

// FIXME only for FVMI_INTERPRETED, also add assertions
#define PM_WORDS  0x100
#define PM_MASK   PM_WORDS-1
#define OPCODE_MASK   0xff000000
#define METADATA_MASK 0x00ffffff

#define NaN 0x80000000
#define WORD_MAX 0x7fffffff
#define WORD_MIN 0x80000001

// ---------------------------------------------------------------------------
// Platform-specific constants
// ---------------------------------------------------------------------------
#define nopasm "nop"  // The name of the native hardware nop instruction
#define NAT uintptr_t // Native pointer type for Harvard program memory
#define CPU_LITTLE_ENDIAN TRUE
#if CPU_LITTLE_ENDIAN
  WORD reverseWord(WORD abcd) {
    WORD dcba = ( // gcc -O3 on x86 uses bswap here so performance is good
      abcd >> 24 & 0x000000ff |
      abcd >>  8 & 0x0000ff00 |
      abcd << 24 & 0xff000000 |
      abcd <<  8 & 0x00ff0000
    );
    return dcba;
  }
#endif

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
// Stack structure
// ---------------------------------------------------------------------------
typedef struct WdStack {
  BYTE sp;
  WORD elem[STACK_CAPACITY];
  #ifdef TRACING_SUPPORTED
    char traceBuf[20];
  #endif
} WDSTACK; // Arduino IDE cannot use this
typedef struct NatStack {
  BYTE sp;
  NAT elem[STACK_CAPACITY];
  #ifdef TRACING_SUPPORTED
    char traceBuf[20];
  #endif
} NATSTACK; // Arduino IDE cannot use this

// ---------------------------------------------------------------------------
// FVM structure -- other structures can be logically equivalent
// ---------------------------------------------------------------------------
typedef struct Fvm {
  #if FVMI ==FVMI_INTERPRETED
    WORD pc; // program counter
    WORD pm[DM_WORDS]; // Harvard program memory
  #endif
  WORD dm[DM_WORDS]; // Harvard RAM data memory
  WORD rm[RM_WORDS]; // Harvard ROM data memory
  WORD hd[HD_WORDS]; // hold
  struct WdStack ds; // data stack
  struct WdStack ts; // temporary stack
  struct WdStack cs; // counter stack
  struct NatStack rs; // return stack
  WORD readBuf;
  WORD writeBuf;
  #ifdef TRACING_SUPPORTED
    WORD tracing;
  #endif
} FVM;  // Arduino IDE cannot use this
// One global instance only (for program source-code portability)
FVM fvm;

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
void runProgram();
static jmp_buf exc_env;
static int excode;

// ---------------------------------------------------------------------------
// Mask logic
// ---------------------------------------------------------------------------
WORD dmsafe(WORD addr) { return addr & DM_MASK; }
WORD enbyte(WORD x)  { return x & BYTE_MASK; }
WORD enshift(WORD x) { return x & SHIFT_MASK; }
WORD rmsafe(WORD addr) { return addr & RM_MASK; }
WORD hdsafe(WORD addr) { return addr & HD_MASK; }
WORD pmsafe(WORD addr) { return addr & PM_MASK; }

// ---------------------------------------------------------------------------
// Stack logic
// ---------------------------------------------------------------------------
void wdPush(WORD x, struct WdStack *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPop(struct WdStack *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[--(s->sp)];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeek(struct WdStack *s) {
  if ((s->sp)>0) {
    WORD wd = s->elem[(s->sp)-1];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeekAt(struct WdStack *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    WORD wd = s->elem[(s->sp)-index];
    return wd;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdPeekAndDec(struct WdStack *s) {
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
WORD wdPokeAt(WORD x, struct WdStack *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    s->elem[(s->sp)-index] = x;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdDrop(struct WdStack *s) {
  if ((s->sp)>0) {
    --(s->sp);
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD wdDup(struct WdStack *s) {
  wdPush(wdPeek(s),s);
}
WORD wdOver(struct WdStack *s) {
  wdPush(wdPeekAt(s,2),s);
}
WORD wdSwap(struct WdStack *s) {
  WORD n1 = wdPeekAt(s,1);
  WORD n2 = wdPeekAt(s,2);
  wdPokeAt(n1,s,2);
  wdPokeAt(n2,s,1);
}
WORD wdRot(struct WdStack *s) {
  WORD n1 = wdPeekAt(s,1);
  WORD n2 = wdPeekAt(s,2);
  WORD n3 = wdPeekAt(s,3);
  wdPokeAt(n1,s,2);
  wdPokeAt(n2,s,3);
  wdPokeAt(n3,s,1);
}
int wdElems(struct WdStack *s) {return (s->sp);}
int wdFree(struct WdStack *s) {return STACK_MAX_INDEX - (s->sp);}
#ifdef TRACING_SUPPORTED
  char* wsTrace(struct WdStack *s) {
    if (wdElems(s) == 0) {
      return "";
    } else { // TODO show more elems
      sprintf(s->traceBuf, "%08x..",wdPeekAt(s,1));
      return s->traceBuf;
    }
  }
#endif
// ---------------------------------------------------------------------------
WORD natPush(NAT x, struct NatStack *s) {
  if ((s->sp) < STACK_MAX_INDEX) {
    s->elem[(s->sp)++] = x;
    return SUCCESS;
  } else {
    return FAILURE;
  }
}
NAT natPop(struct NatStack *s) {
  if ((s->sp)>0) {
    NAT nat = s->elem[--(s->sp)];
    return nat;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD natPeekAt(struct NatStack *s, WORD index) {
  if (index > 0 && (s->sp)>=index) {
    NAT nat = s->elem[(s->sp)-index];
    return nat;
  } else {
    longjmp(exc_env, FAILURE);
  }
}
WORD natElems(struct NatStack *s) {return (s->sp);}
WORD natFree(struct NatStack *s) {return STACK_MAX_INDEX - (s->sp);}
#ifdef TRACING_SUPPORTED
  char* nsTrace(struct NatStack *s) {
    if (natElems(s) == 0) {
      return "";
    } else { // TODO show more elems
      sprintf(s->traceBuf, "*%08x..",natPeekAt(s,1));
      return s->traceBuf;
    }
  }
#endif

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
void Lit(WORD x) { TRC("lit  ") wdPush(x&IMM_MASK,&fvm.ds); }
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
    // wdPush(fvm.dm[n1],&fvm.ds);
    #if CPU_LITTLE_ENDIAN
      wdPush(reverseWord(fvm.dm[n1]),&fvm.ds);
    #else
      wdPush(fvm.dm[n1],&fvm.ds);
    #endif
  }
}
// Traps if address NaN or outside address space
void Put()    { TRC("put  ")
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    //fvm.dm[n1] = n2;
    #if CPU_LITTLE_ENDIAN
      fvm.dm[n1] = reverseWord(n2);
    #else
      fvm.dm[n1] = n2;
    #endif
  }
}
// Traps if address NaN or outside address space
void Geti()   { TRC("geti ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || dmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    // WORD got = fvm.dm[n1];
    #if CPU_LITTLE_ENDIAN
      WORD got = reverseWord(fvm.dm[n1]);
    #else
      WORD got = fvm.dm[n1];
    #endif
    if (got == NaN || dmsafe(got) != n1) {
      longjmp(exc_env, FAILURE);
    } else {
      //wdPush(fvm.dm[got],&fvm.ds);
      #if CPU_LITTLE_ENDIAN
        wdPush(reverseWord(fvm.dm[got]),&fvm.ds);
      #else
        wdPush(fvm.dm[got],&fvm.ds);
      #endif
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
    //WORD got = fvm.dm[n1];
    #if CPU_LITTLE_ENDIAN
      WORD got = reverseWord(fvm.dm[n1]);
    #else
      WORD got = fvm.dm[n1];
    #endif
    if (got == NaN || dmsafe(got) != n1) {
      longjmp(exc_env, FAILURE);
    } else {
      //fvm.dm[got] = n2;
      #if CPU_LITTLE_ENDIAN
        fvm.dm[got] = reverseWord(n2);
      #else
        fvm.dm[got] = n2;
      #endif
    }
  }
}
// Traps if address NaN or outside address space
void Rom()    { TRC("rom  ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || rmsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    //wdPush(fvm.rm[n1],&fvm.ds);
    #if CPU_LITTLE_ENDIAN
      wdPush(reverseWord(fvm.rm[n1]),&fvm.ds);
    #else
      wdPush(fvm.rm[n1],&fvm.ds);
    #endif
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
// FIXME in C this would be implementation-defined behaviour
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
// FIXME in C this would be implementation-defined behaviour
void Or()     { TRC("or   ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2|n1,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
void And()    { TRC("and  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2&n1,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
void Xor()    { TRC("xor  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2^n1,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
void Flip()   { TRC("flip ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = n1^FLIP_MASK;
      wdPush(res,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
void Neg()    { TRC("neg  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN) {
    wdPush(NaN,&fvm.ds);
  } else {
      WORD res = (~n1)+1;
      wdPush(res,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
void Shl()    { TRC("shl  ") // BEWARE: Preserves NaN
  WORD n1 = wdPop(&fvm.ds);
  WORD n2 = wdPop(&fvm.ds);
  if (n1 == NaN || n2 == NaN || n1^SHIFT_MASK != 0) {
    wdPush(NaN,&fvm.ds);
  } else {
    wdPush(n2<<n1,&fvm.ds);
  }
}
// FIXME in C this would be implementation-defined behaviour
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
    #if CPU_LITTLE_ENDIAN
      fvm.hd[n1] = reverseWord(n2);
    #else
      fvm.hd[n1] = n2;
    #endif
  }
}
// Traps if address NaN or outside address space
void Give()   { TRC("give ")
  WORD n1 = wdPop(&fvm.ds);
  if (n1 == NaN || hdsafe(n1) != n1) {
    longjmp(exc_env, FAILURE);
  } else {
    #if CPU_LITTLE_ENDIAN
      wdPush(reverseWord(fvm.hd[n1]),&fvm.ds);
    #else
      wdPush(fvm.hd[n1],&fvm.ds);
    #endif
  }
}

#if FVMP == FVMP_STDIO
  void In()     { TRC("in   ") wdPush(getchar(), &fvm.ds); }
  void Out()    { TRC("out  ") putchar(wdPop(&fvm.ds)); }
#elif FVMP == FVMP_ARDUINO_IDE
  void In()     { TRC("in   ") wdPush(getchar(), &fvm.ds); }
  void Out()    { TRC("out  ") putchar(wdPop(&fvm.ds)); }
#endif

void Inw()    { TRC("inw  ") // TODO these could all fail
#if CPU_LITTLE_ENDIAN
  fread(&(fvm.readBuf),WD_BYTES,1,stdin);
  fvm.readBuf = reverseWord(fvm.readBuf);
  wdPush(fvm.readBuf, &fvm.ds);
#else
  fread(&(fvm.readBuf),WD_BYTES,1,stdin);
  wdPush(fvm.readBuf, &fvm.ds);
#endif
}
void Outw()   { TRC("outw ")
#if CPU_LITTLE_ENDIAN
  fvm.writeBuf = wdPop(&fvm.ds);
  fvm.writeBuf = reverseWord(fvm.writeBuf);
  fwrite(&(fvm.writeBuf),WD_BYTES,1,stdout);
#else
  fvm.writeBuf = wdPop(&fvm.ds);
  fwrite(&(fvm.writeBuf),WD_BYTES,1,stdout);
#endif
}
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
#define d(label) { __label__ lr; \
  TRC("do   ") natPush((NAT)&&lr,&fvm.rs); goto label; lr: ; \
}
#define done { TRC("done ") goto *(natPop(&fvm.rs)); }
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
#define orr Or() ;
#define andd And();
#define xorr Xor();
#define flip Flip();
#define neg Neg();
#define shl Shl();
#define shr Shr();
#define hold Hold();
#define give Give();
#define in In();
#define out Out();
#define inw Inw();
#define outw Outw();
#define jjump(label) { TRC("jump ") goto label; }
// Jump if n is NaN. Preserve n.
#define jjnan(label) { TRC("jnan ") \
  if (wdPeek(&fvm.ds) == NaN) goto label; }


// Jump if either n1 or n2 is NaN. Preserve both.
#define jjorn(label) { TRC("jorn ") \
  WORD n1 = wdPeek(&fvm.ds); WORD n2 = wdPeekAt(&fvm.ds,2); \
  if ((n1 == NaN) || (n2 == NaN)) goto label; }

// Jump if n1 and n2 are both NaN. Preserve both.
#define jjann(label) { TRC("jann ") \
  WORD n1 = wdPeek(&fvm.ds); WORD n2 = wdPeekAt(&fvm.ds,2); \
  if ((n1 == NaN) && (n2 == NaN)) goto label; }

// Jump if n2 == n1 and neither are NaN. Preserve n2.
#define jjnne(label) { TRC("jnne ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPeek(&fvm.ds); \
  if ((n1 != NaN) && (n2 != NaN) && (n1 == n2)) goto label; }

// Jump if n2 == n1 (even if they are NaN). Preserve n2.
#define jjmpe(label) { TRC("jmpe ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPeek(&fvm.ds); \
  if (n1 == n2) goto label; }



// Jump if n is 0. Preserve n.
#define jjnnz(label) { TRC("jnnz ") \
  if (wdPeek(&fvm.ds) == 0) goto label; }
// Jump if n > 0 and not NaN. Preserve n.
#define jjnnp(label) { TRC("jnnp ") \
  WORD n1 = wdPeek(&fvm.ds); \
  if ((n1 != NaN) && (n1 > 0)) goto label; }
// Jump if n2 > n1 and neither are NaN. Preserve n2.
#define jjnng(label) { TRC("jnne ") \
  WORD n1 = wdPop(&fvm.ds); WORD n2 = wdPop(&fvm.ds); \
  if ((n1 != NaN) && (n2 != NaN) && (n1 < n2)) goto label; }
// Jump if n2 < n1 and neither are NaN. Preserve n2.
#define jjnnl(label) { TRC("jnne ") \
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
#if FVMP == FVMP_STDIO
  int startup(struct Fvm *fvm) {
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
#elif FVMP == FVMP_ARDUINO_IDE
  // TODO load stdhold and rom
  int startup(struct Fvm *fvm) {
    Serial.begin(9600);
    while (!Serial) {}
    Serial.println("About to run VM..."); // FIXME
    Serial.flush();
  }
#endif

// ---------------------------------------------------------------------------
// I/O shutdown
// ---------------------------------------------------------------------------
#if FVMP == FVMP_STDIO
  int shutdown(struct Fvm *fvm) {
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
#elif FVMP == FVMP_ARDUINO_IDE
  // TODO store stdhold and rom
  int shutdown(struct Fvm *fvm) {
    Serial.print("Ran VM. Exit code: ");  // FIXME
    Serial.println(excode);  // FIXME
    Serial.flush();
    return SUCCESS;
  }
#endif


// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int run() {
    assert(WD_BYTES == 4);
    assert(sizeof(WORD) == WD_BYTES);
    assert(PMI <= MAX_PMI);
    assert(DM_WORDS <= MAX_DM_WORDS);
    assert(RM_WORDS <= MAX_RM_WORDS);
    assert(HD_WORDS <= MAX_HD_WORDS);
    assert(startup(&fvm) == SUCCESS);
    if (!setjmp(exc_env)) {
      runProgram();
      assert(shutdown(&fvm) == SUCCESS);
      return excode;
    } else {
      return FAILURE;
    }
}

#if FVMP == FVMP_STDIO
  int main() {
    return run();
  }
#elif FVMP == FVMP_ARDUINO_IDE
  void setup() {
    run();
  }
  void loop() {}
#endif

// ---------------------------------------------------------------------------
// Programming language -- very early experiments.
//
// Aim is a simple language equally suitable for native compilation
// and for bytecode compilation, without requiring changes to source code,
// and needing only a few kilobytes of RAM for the bytecode compiler.
// This is just a bootstrapping language for initial freedom.
// It will be used to write a self-hosted compiler.
//
// By convention the programmer shall use only 1 forward reference (m0_x0).
// All other references are to be backward references. This allows
// a bytecode compiler to function with mere kilobytes of RAM,
// which is important for hardware freedom in development.
//
// z symbols (z0..zff) are imported modules, mapped by #defines
// x symbols (x0..xff) are exported from a module by export()
// u symbols (u0..uff) are local to a module and represent units or atoms
// s symbols (s0..sff) are local to a unit or atom
//
// By convention the 0 symbols (m0,x0,u0,s0) are never used except:
//    - m0 for the main module and x0 for its run unit (the entry point)
//    - special reserved purposes to be determined in the future
//
// An atom is a unit that performs no side-effects (that is, which uses only
// the stacks and performs no I/O and no memory access). Currently this
// is by convention and is not enforced. Atoms represent extremely
// reusable software to be easily used in composition.
//
// Preferred syntax would be something like this:
//
//  as(m4)
//  module(math)
//    atom
//      PUB(x1,add)
//        add
//        done
//    endat
//  endmod
//
//
//  as(m0)
//  use(z1,m1,foo)
//  use(z2,m2,bar)
//  use(z3,m3,prn)
//  module(main)
//    unit
//      PUB(x0,run):
//        do(z1.x1,prnIdent)
//        do(z2.x1,prnIdent)
//        do(z3.x1,prnIdent)
//        halt
//    endun
//  endmod
//
//
// To enable this scheme, sed and m4 are being used. See 'exampleProgram.fp2'
// ---------------------------------------------------------------------------
#define ulabels u0,u1,u2,u3,u4,u5,u6,u7;
#define slabels s0,s1,s2,s3,s4,s5,s6,s7;
#define module(name) { __label__ ulabels /*name is just documentation*/
#define unit { __label__ slabels
#define endu ; } // See also 'endmod.c' and 'exampleProgram.fp2'
#define atom { __label__ slabels
#define enda ; } // Note: atom is by convention for now, not yet enforced.

// ---------------------------------------------------------------------------
// Program
// ---------------------------------------------------------------------------

#if FVMI == FVMI_NATIVE
void runProgram() {
  #include "exampleProgram.c"
}
#elif FVMI == FVMI_INTERPRETED

  // FIXME opcode order
  #define ADD   0x01000000
  #define TRON  0x02000000
  #define HALT  0x7f000000
  #define IM    0x80000000

void inline incPc() {
  fvm.pc = pmsafe(++(fvm.pc));
}

WORD inline wordAtPc() {
  return fvm.pm[fvm.pc];
}

void runProgram() {

  // An example program
  WORD program[] = {
    TRON,IM|3,IM|5,ADD,HALT
  };

  // Load program
  for (int i=0; i<(sizeof(program)/WD_BYTES); i++) {
    fvm.pm[i] = program[i];
  }

  // Reset program counter
  fvm.pc = 0;

  // Run interpreter
  while(1) {
    WORD instr = wordAtPc();
    incPc();

    // Handle immediates
    if (instr&FLIP_MASK) {
      Lit(instr);
      continue;
    }
    // Handle all other instructions
    WORD opcode = instr & OPCODE_MASK;
    switch(opcode) { // FIXME add range checks
      case ADD:   Add(); break;
      case TRON:  Tron(); break;
      case HALT:  excode = SUCCESS; return; break;
      default:    excode = ILLEGAL; return; break;
    }
  }

}
#endif
// ===========================================================================
