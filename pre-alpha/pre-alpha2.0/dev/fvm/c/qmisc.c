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
Version:    pre-alpha-0.0.0.70+ for FVM 2.0

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
#define BYTE uint8_t   // (uint8_t)
#define SHIFT BYTE     // (uint5_t)
#define WORD uint32_t  // (uint32_t)
#define LIT WORD       // (uint31_t)
#define CELL WORD      // (uint24_t) Program memory consists of 'cells'
#define SLOT WORD      // (uint32_t) Data memory consists of 'slots'
#define MSb 0x80000000 // Bit mask for most significant bit
#define OPCODE_MASK   0xff000000
#define LIT_MASK      0x7fffffff // 31 bits
#define CELL_MASK     0x00ffffff // TODO reconsider these
#define BYTE_MASK     0x000000ff
#define SHIFT_MASK    0x0000001f
#define SLOTS         0x10000000
#define DM_MASK       0x0fffffff
#define CELLS         0x00001000
#define PM_MASK       0x00000fff
#define SUCCESS 0
#define FAILURE 1
#define ILLEGAL 2
WORD vP = 0; // program counter
WORD vA = 0; // accumulator
WORD vB = 0; // operand register (which is also the immediate register)
CELL vL = 0; // link register
WORD vT = 0; // temporary register
WORD vV = 0; // value register (for put)
WORD vR = 0; // repeat register
WORD rSwap = 0; // not exposed, supports Swap() instruction
WORD dm[SLOTS]; // data memory (Harvard architecture)
int run();
// ---------------------------------------------------------------------------
SLOT dmsafe(SLOT addr) { return addr & DM_MASK; } // TODO reconsider
CELL pmsafe(CELL addr) { return addr & PM_MASK; } // TODO reconsider
BYTE enbyte(WORD x)    { return x & BYTE_MASK; }
LIT enrange(LIT x) { return x & LIT_MASK; }
LIT enshift(WORD x) { return x & SHIFT_MASK; }
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
void Get()    { vA = dm[dmsafe(vA)]; }
void Put()    { dm[dmsafe(vA)] = vV; }
// Increments
void Inc()    { ++vA; }
void Dec()    { --vA; }
// Immediates
void Imm(LIT x) { enrange(x); vB = x; } // bits 31..0
void Neg()    { vB=~vB; ++vB; }         // bit  32 (via negation!)
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
// Instructions which optimize virtualization (dubious)
void Fetch(CELL v_vP) { vA = dm[dm[dmsafe(v_vP)]++]; vT = vA; }
void Opcode() { vA = vA&OPCODE_MASK; }
void Label()  { vA = vA&CELL_MASK; }
void Lit()    { vA = vA&LIT_MASK; }
// Jumps (static only)
void JmpZ(CELL a){if(vA == 0)         { vP = pmsafe(a); }} // zero
void JmpE(CELL a){if(vB == vA)        { vP = pmsafe(a); }} // equals vB
void JmpM(CELL a){if(vA == MSb)       { vP = pmsafe(a); }} // only msbit set
void JmpN(CELL a){if(MSb == (vA&MSb)) { vP = pmsafe(a); }} // msbit set
void JmpS(CELL a){if(vB == (vA&vB))   { vP = pmsafe(a); }} // all 1s of vB
void JmpU(CELL a){if(vB == (vA|vB))   { vP = pmsafe(a); }} // all 0s of vB
void Jump(CELL a)                     { vP = pmsafe(a); }  // UNCONDITIONAL
void Rpt(CELL a) {if(--vR != 0)       { vP = pmsafe(a); }} // repeat
void Br(CELL a)  { vL = vP; vP = a; }   // branch with link
void Link()      { vP = vL; }           // return by link
// Machine metadata
void Mdm()    { vA = SLOTS; }
// Other
void Nop()    { ; } // Consider eliminating this opcode
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
  return run();
}
// ===========================================================================
WORD prg[] = {iIMM|0x10000000, iIMMR, iNOP, iRPT|2, iHALT};
int run() {
  WORD instr = 0;
  WORD opcode = 0;
  LIT lit = 0;
  CELL cell = 0;
  while(1) {
    #ifdef DEBUG
    printf("\n%08x  ",vP);
    #endif
    instr = prg[vP++];
    #ifdef DEBUG
    printf("%08x vA:%08x vB:%08x vV:%08x vT:%08x vR:%08x vL:%08x ",
            instr, vA, vB, vV, vT, vR, vL);
    #endif
    if (iIMM == (instr&iIMM)) {
      lit = instr&LIT_MASK; Imm(lit);
    } else {
      opcode = instr&OPCODE_MASK;
      switch(opcode) {
        case iNOP:    Nop(); break;
        case iADD:    Add(); break;
        case iSUB:    Sub(); break;
        case iOR:     Or(); break;
        case iAND:    And(); break;
        case iXOR:    Xor(); break;
        case iNOT:    Not(); break;
        case iFLIP:   Flip(); break;
        case iSHL:    Shl(); break;
        case iSHR:    Shr(); break;
        case iGET:    Get(); break;
        case iPUT:    Put(); break;
        case iINC:    Inc(); break;
        case iDEC:    Dec(); break;
        case iNEG:    Neg(); break;
        case iIMMA:   ImmA(); break;
        case iIMMR:   ImmR(); break;
        case iIMMT:   ImmT(); break;
        case iIMMV:   ImmV(); break;
        case iSWAP:   Swap(); break;
        case iTOB:    Tob(); break;
        case iTOR:    Tor(); break;
        case iTOT:    Tot(); break;
        case iFROMR:  Fromr(); break;
        case iFROMT:  Fromt(); break;
        case iFROMV:  Fromv(); break;
        case iJMPZ:   cell = instr&CELL_MASK; JmpZ(cell); break;
        case iJMPE:   cell = instr&CELL_MASK; JmpE(cell); break;
        case iJMPM:   cell = instr&CELL_MASK; JmpM(cell); break;
        case iJMPN:   cell = instr&CELL_MASK; JmpN(cell); break;
        case iJMPS:   cell = instr&CELL_MASK; JmpS(cell); break;
        case iJMPU:   cell = instr&CELL_MASK; JmpU(cell); break;
        case iJUMP:   cell = instr&CELL_MASK; Jump(cell); break;
        case iRPT:    cell = instr&CELL_MASK; Rpt(cell);  break;
        case iBR:     cell = instr&CELL_MASK; Br(cell);   break;
        case iLINK:   cell = instr&CELL_MASK; Link(cell); break;
        case iMDM:    Mdm(); break;
        case iHALT:   return SUCCESS; break;
        default:      return ILLEGAL; break;
      } // switch
    } // else
  } // while(1)
}
// ===========================================================================


