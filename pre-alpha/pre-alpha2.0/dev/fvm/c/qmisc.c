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
Version:    pre-alpha-0.0.5.1+ for FVM 2.0
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
// CURRENTLY 34+ OPCODES
/*
// Arithmetic
void Add()    { vA+=vB; }
void Sub()    { vA-=vB; }
// Logic
void Or()     { vA|=vB; }
void And()    { vA&=vB; }
void Xor()    { vA^=vB; } // Maybe add NOT and NEG too
// Shifts
void Shl()    { vA<<=enshift(vB); }
void Shr()    { vA>>=enshift(vB); }
// Moves
void Get()    { vA = dm[safe(vB)]; }
void Put()    { dm[safe(vB)] = vA; }

void Geti()   { WORD sB = safe(vB); vA = dm[safe(dm[sB])]; }
void Puti()   { WORD sB = safe(vB); dm[safe(dm[sB])] = vA; } // untested

void Decm()   { --dm[safe(vB)]; }
void Incm()   { ++dm[safe(vB)]; }

void At()     { vB = dm[safe(vB)]; }
void Copy()   { dm[safe(vB+vA)] = dm[safe(vB)]; } // a smell?
// Increments for addressing
void Inc()    { ++vB; }
void Dec()    { --vB; }
// Immediates
void Imm(METADATA x)    { vB = enrange(x); } // bits 31..0
void Flip()             { vB = vB^MSb; }     // bit  32 (NOT might be better)
// Transfers (maybe expand these)
void Swap()   { vB = vB^vA; vA = vA^vB; vB = vB^vA; }
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Fromb()  { vA = vB; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Nop()    { ; }
void Halt()   { vA = enbyte(vA); } // Exit manually in switch
// Jumps (static only), maybe reduce these back to jump and jmpe only
void JmpA(CELL a) { if (vA == 0) { vZ = a; } }
void JmpB(CELL a) { if (vB == 0) { vZ = a; } }
void JmpE(CELL a) { if (vA == vB) { vZ = a; } }
void JmpN(CELL a) { if (MSb == (vA&MSb)) { vZ = a; } } // MSb set in vA
void JmpS(CELL a) { if (vB == (vA&vB)) { vZ = a; } } // all vB 1s set in vA
void JmpU(CELL a) { if (vB == (vA|vB)) { vZ = a; } } // all vB 0s unset in vA
void Jump(CELL a) { vZ = a; }
void Rpt(CELL a)  { if ( vR != 0) { --vR; vZ = a; } }
void Br(CELL a)   { vL = vZ; vZ = a; }
void Link()       { vZ = vL; }
void In(CELL a)   { vA = getchar(); } // If fail goto label
void Out(CELL a)  { putchar(vA); } // If fail goto label
*/
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
  // Incredibly even the below inlined switch is equally slow,
  // nothing was gained by doing that; might as well return to the above.
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
      vB = enrange(instr);
      continue;
    }
    // Handle all other instructions
    WORD opcode = instr & OPCODE_MASK;
    switch(opcode) { // TODO Fix order
      case NOP:    break;
      case ADD:    vA+=vB; break;
      case SUB:    vA-=vB; break;
      case OR:     vA|=vB; break;
      case AND:    vA&=vB; break;
      case XOR:    vA^=vB; break; // Maybe add NOT and NEG too
      case FLIP:   vB = vB^MSb; break;
      case SHL:    vA<<=enshift(vB); break;
      case SHR:    vA>>=enshift(vB); break;
      case GET:    vA = dm[safe(vB)]; break;
      case PUT:    dm[safe(vB)] = vA; break;
      case GETI:   { WORD sB = safe(vB); vA = dm[safe(dm[sB])]; } break;
      case PUTI:   { WORD sB = safe(vB); dm[safe(dm[sB])] = vA; } break;
      case INCM:   ++dm[safe(vB)]; break;
      case DECM:   --dm[safe(vB)]; break;
      case AT:     vB = dm[safe(vB)]; break;
      case COPY:   dm[safe(vB+vA)] = dm[safe(vB)]; break; // a smell?
      case INC:    ++vB; break;
      case DEC:    --vB; break;
      case SWAP:   vB = vB^vA; vA = vA^vB; vB = vB^vA; break;
      case TOB:    vB = vA; break;
      case TOR:    vR = vA; break;
      case TOT:    vT = vA; break;
      case FROMB:  vA = vB; break;
      case FROMR:  vA = vR; break;
      case FROMT:  vA = vT; break;
      case JMPA:   if (vA == 0) { vZ = instr&DM_MASK; } break;
      case JMPB:   if (vB == 0) { vZ = instr&DM_MASK; } break;
      case JMPE:   if (vA == vB) { vZ = instr&DM_MASK; } break;
      case JMPN:   if (MSb == (vA&MSb)) { vZ = instr&DM_MASK; } break;
      case JMPS:   if (vB == (vA&vB)) { vZ = instr&DM_MASK; } break;
      case JMPU:   if (vB == (vA|vB)) { vZ = instr&DM_MASK; } break;
      case JUMP:   vZ = instr&DM_MASK; break;
      case RPT:    if ( vR != 0) { --vR; vZ = instr&DM_MASK; } break;
      case BR:     vL = vZ; vZ = instr&DM_MASK; break;
      case LINK:   vZ = vL; break;
      case MDM:    vA = DM_WORDS; break;
      case IN:     vA = getchar(); break; // TODO branch on failure
      case OUT:    putchar(vA); break; // TODO branch on failure
      case HALT:   vA = enbyte(vA); return vA; break;
      default: return ILLEGAL; break;
    }
  }
}

// ===========================================================================


