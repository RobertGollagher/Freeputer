/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170910+
Version:    pre-alpha-0.0.5.2+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.

  Returned to 32-bit. Unless content to accept 256 kB standard size
  (16-bit word-addressed space), masking is required. That is a little
  large for hardware freedom in 2017 (to not require any OS) but may
  be acceptable in future. Perhaps ROM/RAM split should be considered.

  Idea: no limit on the parent, but standard child size(s).
  If so, obvious child sizes would be 24-26, 16, 8 bit spaces
    which means 64-256 Mb, 256 kB, 1 kB.
  Was van neumann for child a mistake?

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
//#define TRACING_ENABLED // Comment out unless debugging

#include <stdio.h>
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
#define MAX_DM_WORDS 0x10000000 // <= 2^(WD_BITS-4) due to C limitations.
#define DM_WORDS  0x10000  // Must be some power of 2 <= MAX_DM_WORDS.
#define DM_MASK   DM_WORDS-1
#define nopasm "nop" // The name of the native hardware nop instruction
// There are only 4 accessible registers:
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
LNKT vL = 0; // link register (not accessible)
WORD vZ = 0; // program counter (not accesible) (maybe it should be)
WORD dm[DM_WORDS]; // data memory (Harvard architecture of parent)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
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
void inline Get()    { vA = dm[safe(vB)]; }
void inline Put()    { dm[safe(vB)] = vA; }

void inline Geti()   { WORD sB = safe(vB); vA = dm[safe(dm[sB])]; }
void inline Puti()   { WORD sB = safe(vB); dm[safe(dm[sB])] = vA; } // untested

void inline Decm()   { --dm[safe(vB)]; }
void inline Incm()   { ++dm[safe(vB)]; }

void inline At()     { vB = dm[safe(vB)]; }
void inline Copy()   { dm[safe(vB+vA)] = dm[safe(vB)]; } // a smell?
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
void inline Mdm()    { vA = DM_WORDS; }
// Other
void inline Nop()    { ; }
void inline Halt()   { vA = enbyte(vA); } // Exit manually in switch
// Jumps (static only), maybe reduce these back to jump and jmpe only
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
void inline In(CELL a)   { vA = getchar(); } // If fail goto label
void inline Out(CELL a)  { putchar(vA); } // If fail goto label
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
#define MDM   0x1a000000
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
  IM|3,ADD,IM|5,ADD,IM|0x7fffffff,FROMB,TOR,NOP,RPT|7,HALT
};
void loadProgram() {
  for (int i=0; i<(sizeof(program)/WD_BYTES); i++) {
    dm[i] = program[i];
  }
}
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  loadProgram();
  while(1) {
    WORD instr = dm[vZ];
#ifdef TRACING_ENABLED
printf("\n%08x %08x vA:%08x vB:%08x vT:%08x vR:%08x vL:%08x ",
        vZ, instr, vA, vB, vT, vR, vL);
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
      case JMPA:   JmpA(instr&DM_MASK); break;
      case JMPB:   JmpB(instr&DM_MASK); break;
      case JMPE:   JmpE(instr&DM_MASK); break;
      case JMPN:   JmpN(instr&DM_MASK); break;
      case JMPS:   JmpS(instr&DM_MASK); break;
      case JMPU:   JmpU(instr&DM_MASK); break;
      case JUMP:   Jump(instr&DM_MASK); break;
      case RPT:    Rpt(instr&DM_MASK);  break;
      case BR:     Br(instr&DM_MASK);   break;
      case LINK:   Link(); break;
      case MDM:    Mdm(); break;
      case IN:     In(instr&DM_MASK); break;
      case OUT:    Out(instr&DM_MASK); break;
      case HALT:   Halt(); return vA; break;
      default: return ILLEGAL; break;
    }
  }
}

// ===========================================================================


