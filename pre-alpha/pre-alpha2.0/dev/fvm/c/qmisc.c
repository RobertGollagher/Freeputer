/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170911+
Version:    pre-alpha-0.0.5.8+ for FVM 2.0
=======

                              This Edition:

                             Portable C for

        1. QVMP_STDIO: gcc using <stdio.h> for FILEs (eg Linux targets) 
        2. QVMP_ARDUINO_IDE: Arduino IDE (eg Arduino or chipKIT targets)

                               ( ) [ ] { }

   TODO:
   - Adopt standard sizes: XS, S, M, L, XL (1 kB, 16 kB, 256 kB, 4 MB, 64 MB)
   - Trap on read/write/jump out of bounds rather than masking
   - Bring 'fvm2.js', 'qmisc.c' and 'qmisc.s' into line


==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
// -- TRACING (comment out the next line unless debugging): ------------------
#define TRACING_ENABLED
// -- SUPPORTED PLATFORMS: ---------------------------------------------------
#define QVMP_STDIO 0 // gcc using <stdio.h> for FILEs (eg Linux targets)
#define QVMP_ARDUINO_IDE 1 // Arduino IDE (eg Arduino or chipKIT targets)
// -- SPECIFY YOUR TARGET PLATFORM HERE: -------------------------------------
#define QVMP QVMP_ARDUINO_IDE
// ---------------------------------------------------------------------------
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t // Unsigned to avoid undefined behaviour in C
#define BYTE uint8_t
#define WD_BYTES 4
#define WD_BITS WD_BYTES*8
#define MSb 0x80000000 // Bit mask for most significant bit
#define IM MSb
#define LNKT uintptr_t
#define CELL WORD // reconsider
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
#define OPCODE_MASK   0xff000000
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2
#define MAX_MEM_WORDS 0x1000
#define MEM_WORDS MAX_MEM_WORDS
#define MEM_MASK   MEM_WORDS-1
int runVM();
// ---------------------------------------------------------------------------
// There are only 4 accessible registers:
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
LNKT vL = 0; // link register (not accessible)
WORD vZ = 0; // program counter (not accesible) (maybe it should be)
WORD mem[MEM_WORDS]; // system memory (now using Von Neumann architecture)
// ---------------------------------------------------------------------------
#if QVMP == QVMP_STDIO
  #include <stdio.h>
  void inline In(CELL a)   { vA = getchar(); } // TODO If fail goto label
  void inline Out(CELL a)  { putchar(vA); } // TODO If fail goto label
  #define traceVM { \
    printf("\n%08x %08x vA:%08x vB:%08x vT:%08x vR:%08x vL:%08x ", \
            vZ, instr, vA, vB, vT, vR, vL); \
  }
  void initVM() {} // Nothing here yet
  int main() { runVM(); }
// ---------------------------------------------------------------------------
#elif QVMP == QVMP_ARDUINO_IDE
  #include <Arduino.h>
  void inline In(CELL a)   { vA = Serial.read(); } // TODO If fail goto label
  void inline Out(CELL a)  { Serial.write(vA); } // TODO If fail goto label
  char traceFmt[80]; // WARNING: only barely wide enough for traceFmt!
  #define traceVM { \
    sprintf(traceFmt, \
             "%08lx %08lx vA:%08lx vB:%08lx vT:%08lx vR:%08lx vL:%08lx ", \
              vZ, instr, vA, vB, vT, vR, vL); \
    Serial.println(traceFmt); \
  }
  void initVM() {} // Nothing here yet
  void setup() {
    Serial.begin(9600);
    while (!Serial) {}
    Serial.println("About to run VM...");
    Serial.flush();
    int exitCode = runVM();
    Serial.print("Ran VM. Exit code (vA): ");
    Serial.println(exitCode);
    Serial.flush();
  }
  void loop() {} // Nothing here since all done in setup()
// ---------------------------------------------------------------------------
#else
  #pragma GCC error "Problem: Invalid target platform for compilation. \
  Solution: You must set QVMP to QVMP_STDIO or QVMP_ARDUINO_IDE. \
  Details: See 'qmisc.c' (or similar) source."
#endif
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & MEM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// CURRENTLY 41+ OPCODES
// Arithmetic
void inline Add()    { vA+=vB; }
void inline Sub()    { vA-=vB; }
// Logic
void inline Or()     { vA|=vB; }
void inline And()    { vA&=vB; }
void inline Xor()    { vA^=vB; } // Maybe add NOT and NEG too
// Shifts
void inline Shl()    { vA<<=enshift(vB); }
void inline Shr()    { vA>>=enshift(vB); }
// Moves
void inline Get()    { vA = mem[safe(vB)]; }
void inline Put()    { mem[safe(vB)] = vA; }

void inline Geti()   { WORD sB = safe(vB); vA = mem[safe(mem[sB])]; }
void inline Puti()   { WORD sB = safe(vB); mem[safe(mem[sB])] = vA; } // untested

void inline Decm()   { --mem[safe(vB)]; }
void inline Incm()   { ++mem[safe(vB)]; }

void inline At()     { vB = mem[safe(vB)]; }
void inline Copy()   { mem[safe(vB+vA)] = mem[safe(vB)]; } // a smell?
// Increments for addressing
void inline Inc()    { ++vB; }
void inline Dec()    { --vB; }
// Immediates
void inline Imm(METADATA x)    { vB = enrange(x); } // bits 31..0
void inline Flip()             { vB = vB^MSb; }     // bit  32 (NOT might be better)
// Transfers (maybe expand these)
void inline Swap()   { vB = vB^vA; vA = vA^vB; vB = vB^vA; }
void inline Tob()    { vB = vA; }
void inline Tot()    { vT = vA; }
void inline Tor()    { vR = vA; }
void inline Fromb()  { vA = vB; }
void inline Fromt()  { vA = vT; }
void inline Fromr()  { vA = vR; }
// Machine metadata
void inline Mem()    { vA = MEM_WORDS; }
// Other
void inline Nop()    { ; }
void inline Halt()   { vA = enbyte(vA); } // Exit manually in switch
// Jumps (TODO add dynamic?), maybe reduce these back to jump and jmpe only
void inline JmpA(CELL a) { if (vA == 0) { vZ = a; } }
void inline JmpB(CELL a) { if (vB == 0) { vZ = a; } }
void inline JmpE(CELL a) { if (vA == vB) { vZ = a; } }
void inline JmpN(CELL a) { if (MSb == (vA&MSb)) { vZ = a; } } // MSb set in vA
void inline JmpS(CELL a) { if (vB == (vA&vB)) { vZ = a; } } // all vB 1s set in vA
void inline JmpU(CELL a) { if (vB == (vA|vB)) { vZ = a; } } // all vB 0s unset in vA
void inline Jump(CELL a) { vZ = a; }
void inline Rpt(CELL a)  { if ( vR != 0) { --vR; vZ = a; } }
void inline Br(CELL a)   { vL = vZ; vZ = a; }
void inline Link()       { vZ = vL; }
// ===========================================================================
// Opcodes are sequential for now, with complex ones highest
#define NOP   0x00000000 // Simple
#define ADD   0x01000000
#define SUB   0x02000000
#define OR    0x03000000
#define AND   0x04000000
#define XOR   0x05000000
#define SHL   0x06000000
#define SHR   0x07000000
#define GET   0x08000000
#define PUT   0x09000000
#define GETI  0x0a000000
#define PUTI  0x0b000000
#define INCM  0x0c000000
#define DECM  0x0d000000
#define AT    0x0e000000
#define COPY  0x0f000000
#define INC   0x10000000
#define DEC   0x11000000
#define FLIP  0x12000000
#define SWAP  0x13000000
#define TOB   0x14000000
#define TOR   0x15000000
#define TOT   0x16000000
#define FROMB 0x17000000
#define FROMR 0x18000000
#define FROMT 0x19000000
#define MEM   0x1a000000
#define LINK  0x1b000000
#define HALT  0x1c000000
#define JMPA  0x1d000000 // Complex
#define JMPB  0x1e000000
#define JMPE  0x1f000000
#define JMPN  0x20000000
#define JMPS  0x21000000
#define JMPU  0x22000000
#define JUMP  0x23000000
#define RPT   0x24000000
#define BR    0x25000000
#define IN    0x26000000
#define OUT   0x27000000
// ===========================================================================
WORD program[] = {
  // The NOP is cell 7 in this program.
  // Interpreted performance is poor, about 17 sec for 0x7fffffff nop repeats,
  // more than 10 times slower than 'qmisc-native.c' but truly portable.
  // Note: not rewriting the switch into a function-pointer table
  // for now, since previous experience suggests that would not
  // be properly portable to Arduino with benefits.
  // Will rewrite in assembly language later.
  IM|3,ADD,IM|5,ADD,IM|2,FROMB,TOR,NOP,RPT|7,HALT
};
void loadProgram() {
  for (int i=0; i<(sizeof(program)/WD_BYTES); i++) {
    mem[i] = program[i];
  }
}
// ===========================================================================
int runVM() {
  assert(sizeof(WORD) == WD_BYTES);
  initVM();
  loadProgram();
  while(1) {
    WORD instr = mem[vZ];
#ifdef TRACING_ENABLED
    traceVM
#endif
    vZ = safe(++vZ);
    // Handle immediates
    if (instr&MSb) {
      Imm(instr);
      continue;
    }
    // Handle all other instructions
    WORD opcode = instr & OPCODE_MASK;
    switch(opcode) { // TODO Fix order
      case NOP:    Nop(); break;
      case ADD:    Add(); break;
      case SUB:    Sub(); break;
      case OR:     Or(); break;
      case AND:    And(); break;
      case XOR:    Xor(); break;
      case FLIP:   Flip(); break;
      case SHL:    Shl(); break;
      case SHR:    Shr(); break;
      case GET:    Get(); break;
      case PUT:    Put(); break;
      case GETI:   Geti(); break;
      case PUTI:   Puti(); break;
      case INCM:   Incm(); break;
      case DECM:   Decm(); break;
      case AT:     At(); break;
      case COPY:   Copy(); break;
      case INC:    Inc(); break;
      case DEC:    Dec(); break;
      case SWAP:   Swap(); break;
      case TOB:    Tob(); break;
      case TOR:    Tor(); break;
      case TOT:    Tot(); break;
      case FROMB:  Fromb(); break;
      case FROMR:  Fromr(); break;
      case FROMT:  Fromt(); break;
      case JMPA:   JmpA(instr&MEM_MASK); break;
      case JMPB:   JmpB(instr&MEM_MASK); break;
      case JMPE:   JmpE(instr&MEM_MASK); break;
      case JMPN:   JmpN(instr&MEM_MASK); break;
      case JMPS:   JmpS(instr&MEM_MASK); break;
      case JMPU:   JmpU(instr&MEM_MASK); break;
      case JUMP:   Jump(instr&MEM_MASK); break;
      case RPT:    Rpt(instr&MEM_MASK);  break;
      case BR:     Br(instr&MEM_MASK);   break;
      case LINK:   Link(); break;
      case MEM:    Mem(); break;
      case IN:     In(instr&MEM_MASK); break;
      case OUT:    Out(instr&MEM_MASK); break;
      case HALT:   Halt(); return vA; break;
      default: return ILLEGAL; break;
    }
  }
}

// ===========================================================================


