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
Version:    pre-alpha-0.0.5.0+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.

  Currently sadly moving to a bytecode interpreter for the parent.
  Needed because gcc local labels and thus vL are not reliably portable.

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
#define TRACING_ENABLED // Comment out unless debugging

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t
#define WD_BYTES 4
#define WD_BITS WD_BYTES*8
#define MSb 0x80000000 // Bit mask for most significant bit
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
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
WORD dm[DM_WORDS]; // data memory (Harvard architecture of parent)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// CURRENTLY 34+ OPCODES
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
void Noop()   { ; }
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
#define out(label) putchar(vA); // If fail goto label
// ===========================================================================
// Opcodes are sequential for now, with complex ones highest
#define iNOOP  0x00000000
#define iADD   0x01000000
#define iSUB   0x02000000
#define iOR    0x03000000
#define iAND   0x04000000
#define iXOR   0x05000000
#define iSHL   0x06000000
#define iSHR   0x07000000
#define iGET   0x08000000
#define iPUT   0x09000000
#define iGETI  0x0a000000
#define iPUTI  0x0b000000
#define iINCM  0x0c000000
#define iDECM  0x0d000000
#define iAT    0x0e000000
#define iCOPY  0x0f000000
#define iINC   0x10000000
#define iDEC   0x11000000
#define iFLIP  0x12000000
#define iSWAP  0x13000000
#define iTOB   0x14000000
#define iTOR   0x15000000
#define iTOT   0x16000000
#define iFROMB 0x17000000
#define iFROMR 0x18000000
#define iFROMT 0x19000000
#define iMDM   0x1a000000
#define iLINK  0x1b000000
#define iHALT  0x1c000000
// Above here = complex
#define iJMPA  0x1d000000
#define iJMPB  0x1e000000
#define iJMPE  0x1f000000
#define iJUMN  0x20000000
#define iJMPS  0x21000000
#define iJMPU  0x22000000
#define iJUMP  0x23000000
#define iRPT   0x24000000
#define iBR    0x25000000
#define iIN    0x26000000
#define iOUT   0x27000000
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  return 99;
}

// ===========================================================================


