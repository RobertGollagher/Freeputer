/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170819+
Version:    pre-alpha-0.0.0.65+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.
  This is a meta-machine: its purpose is to safely virtualize other VM types.
  No call/return, no stack pointer: thus correct and robust without trapping.
  Supports branch/link: uses a standalone, inaccessible link register.
  No undefined behaviour: everything is unsigned, no <= >= operators.
  Harvard architecture: allows easy native implementation.

  20170806/20170818: STILL LOOKS VERY PROMISING AS THE BASIC CORE OF Plan G.
  20170819: DECISION MADE TO GO INTERPRETED. REASONS:

    1. Minimizes gcc/binutils dependency and keeps this implementation tiny.
    2. Ensures design is OPTIMIZED FOR INTERPRETER (thus for VIRTUALIZATION).
    3. If follow same principles should be easy to natively compile anyway:
          - Harvard architecture with defined cell size in data space only
          - No runtime dependency allowed on cell sizes in program space
          - No dynamic jumps and cannot read or write in program space

  TODO: Maybe add more temp registers to more easily support stack pointers
  if virtualized child VMs wish to implement stack pointers. Of course,
  the parent VM does not have any stack pointer (see note above).

  TODO: Better optimize for interpreter.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
// #define DEBUG // Comment out unless debugging

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t
#define WORD_SIZE 4
#define MSb 0x80000000 // Bit mask for most significant bit
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x7fffffff // 31 bits
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2
#define DM_WORDS 0x10000000 // Must be power of 2
#define DM_MASK  0x0fffffff
WORD vA = 0; // accumulator
WORD vB = 0; // operand register (which is also the immediate register)
LNKT vL = 0; // link register
WORD vT = 0; // temporary register
WORD vV = 0; // value register (for put)
WORD vR = 0; // repeat register
WORD rSwap = 0; // not exposed, supports Swap() instruction
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int run();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
// ---------------------------------------------------------------------------
// Arithmetic
void Add()    { vA+=vB; }
void Sub()    { vA-=vB; }
// Logic
void Or()     { vA|=vB; }
void And()    { vA&=vB; }
void Xor()    { vA^=vB; }
void Not()    { vA=~vA; }  // Note: Not(), Inc() = Neg() (MAX_NEG unchanged)
void Flip()   { vA^=MSb; } // Flips value of msbit
// Shifts
void Shl()    { vA<<=enshift(vB); }
void Shr()    { vA>>=enshift(vB); }
// Moves
void Get()    { vA = dm[safe(vA)]; }
void Put()    { dm[safe(vA)] = vV; }
// Increments
void Inc()    { ++vA; }
void Dec()    { --vA; }
// Immediates
void Imm(METADATA x)    { enrange(x); vB = x; } // bits 31..0
void Neg()    { vB=~vB; ++vB; }                 // bit  32 (via negation!)
void ImmA()   { vA = vB; }
void ImmR()   { vR = vB; }
void ImmT()   { vT = vB; }
void ImmV()   { vV = vB; }
// Transfers (maybe expand these)
void Swap()   { rSwap = vA; vA = vV; vV = rSwap; }
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Tov()    { vV = vA; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
void Fromv()  { vA = vV; }

// Jumps (static only) (an interpreter would enforce a 24-bit program space)
#define jmpz(label) if (vA == 0)       { goto label; } // vA is zero
#define jmpe(label) if (vB == vA)      { goto label; } // vA equals vB
#define jmpm(label) if (vA == MSb)     { goto label; } // vA only msbit set
#define jmpn(label) if (MSb == (vA&MSb)) { goto label; } // vA msbit set
#define jmps(label) if (vB == (vA&vB)) { goto label; } // vA has all 1s of vB
#define jmpu(label) if (vB == (vA|vB)) { goto label; } // vA has all 0s of vB
#define jump(label) goto label; // UNCONDITIONAL
#define rpt(label) if (--vR != 0)  { asm(""); goto label; } // prevents optmzn
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Nop()    { asm("nop"); } // prevents optmzn (works on x86 at least)
#define halt return enbyte(vA);
// ===========================================================================
// Opcodes for interpreter (mostly arbitrary values for now).
// Current scheme is FW32 (very poor density but simple and portable).
#define iIMM   0x80000000 // DONE
#define iNOP   0x00000000 // DONE
#define iADD   0x01000000 // DONE
#define iSUB   0x02000000
#define iOR    0x03000000
#define iAND   0x04000000
#define iXOR   0x05000000
#define iNOT   0x06000000
#define iFLIP  0x07000000
#define iSHL   0x08000000
#define iSHR   0x09000000
#define iGET   0x0a000000
#define iPUT   0x0b000000
#define iINC   0x0c000000
#define iDEC   0x0d000000
#define iNEG   0x0e000000
#define iIMMA  0x0f000000
#define iIMMR  0x10000000
#define iIMMT  0x11000000
#define iIMMV  0x12000000
#define iSWAP  0x13000000
#define iTOB   0x14000000
#define iTOR   0x15000000
#define iTOT   0x16000000
#define iFROMR 0x17000000
#define iFROMT 0x18000000
#define iFROMV 0x19000000
#define iJMPZ  0x1a000000
#define iJMPE  0x1b000000
#define iJMPM  0x1c000000
#define iJMPN  0x1d000000
#define iJMPS  0x1e000000
#define iJMPU  0x1f000000
#define iJUMP  0x20000000
#define iRPT   0x21000000
#define iBR    0x22000000
#define iLINK  0x23000000
#define iMDM   0x24000000
#define iHALT  0x25000000 // DONE
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WORD_SIZE);
  return run();
}
// ===========================================================================
#define OPCODE_MASK 0xff000000
#define LABEL_MASK  0x00ffffff
WORD vPC = 0;
WORD prg[] = {iIMM|0x10000000, iIMMR, iNOP, iRPT|2, iHALT};
void JmpZ() {}; // TODO implement these
void JmpE() {};
void JmpM() {};
void JmpN() {};
void JmpS() {};
void JmpU() {};
void Jump() {}; // Note: all cell addresses need to be limited to 24 bits
void Rpt(WORD cell) { if (--vR != 0) { vPC = cell; } };
void Br() {};
void Link() {};

int run() {
  WORD instr;
  WORD opcode;
  WORD m;
  while(1) {
    #ifdef DEBUG
    printf("\n%08x ",vPC);
    #endif
    instr = prg[vPC++];
    opcode = (instr&OPCODE_MASK) >> 24;
    #ifdef DEBUG
    printf("%08x %02x ",instr,opcode);
    #endif

    if (0x80 == (opcode&0x80)) {
      m = instr&METADATA_MASK;
      Imm(m);
      #ifdef DEBUG
      printf("imm   m: %08x vB: %08x",m,vB);
      #endif
    } else {
      switch(opcode) {
        case 0x10:
          ImmR();
          #ifdef DEBUG
          printf("immr vR: %08x",m,vR);
          #endif
          break;
        case 0x21: //iRPT
          m = instr&LABEL_MASK;
          #ifdef DEBUG
          printf("rpt label: %08x vR: %08x",m,vR);
          #endif
        Rpt(m);
          break;
        case 0x00:
          asm("nop");
          #ifdef DEBUG
          printf("nop");
          #endif
          break;
        case 0x25: // iHALT
          return SUCCESS;
          break;
        default:
          return ILLEGAL;
          break;
      } // switch
    } // else
  } // while(true)
}
// ===========================================================================

