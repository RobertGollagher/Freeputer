/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170815+
Version:    pre-alpha-0.0.0.43+ for FVM 2.0

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  Removed most notes so as not to prejudice lateral thinking during design.
  Harvard architecture. Data memory: 32-bit address space.
  Program memory: 24-bit program space if interpreted.
  Program memory: no limit if native.

    20170806: THIS NOW LOOKS VERY PROMISING AS THE BASIC CORE OF Plan G.

      FIXME NEXT: need to add a call/return solution as otherwise
        native implementation is impractical (link insufficient).
        Also a register to hold a PC for child would be good.

      NOTE: vS, vD and vP could all be abolished if desired;
        instead could use get and put with just vA, vV.

==============================================================================
 WARNING: This is pre-alpha software and as such may well be incomplete,
 unstable and unreliable. It is considered to be suitable only for
 experimentation and nothing more.
============================================================================*/
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#define WORD uint32_t
#define WORD_SIZE 4
#define NEG_MASK 0x80000000 // If negative or MAX_NEG in two's complement.
#define BIG_MASK 0x40000000 // If absolute value > 0x3fffffff.
#define LNKT uintptr_t
#define METADATA WORD
#define METADATA_MASK 0x00ffffff // 24 bits
#define LSB_MASK      0x000000ff
#define MSB_MASK      0xff000000
#define SHIFT_MSB     0x00000018
#define SHIFT_MASK    0x0000001f
#define SUCCESS 0
#define FAILURE 1
#define DM_WORDS 0x10000000 // Must be power of 2
#define DM_MASK  0x0fffffff
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
LNKT vL = 0; // link register
WORD vT = 0; // temporary register
WORD vV = 0; // value register (only used by put)
WORD vR = 0; // repeat register
WORD vS = 0; // source register
WORD vD = 0; // destination register
// Note: vE treats vS and vD as a single duplicated register
WORD vP = 0; // peek register
WORD v1 = 0; // peek parking register 1
WORD v2 = 0; // peek parking register 2
WORD v3 = 0; // peek parking register 3
WORD v4 = 0; // peek parking register 4
WORD rSwap = 0; // not exposed, supports Swap() instruction
WORD dm[DM_WORDS]; // data memory (Harvard architecture)
int exampleProgram();
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & LSB_MASK; }
METADATA enmsb(METADATA x)   { return enbyte(x)<<SHIFT_MSB; }
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
void Not()    { vA=~vA; }
void Neg()    { vA=~vA; ++vA; } // MAX_NEG unchanged (an advantage)
// Shifts
void Shl()    { vA<<=enshift(vB); }
void Shr()    { vA>>=enshift(vB); }
// A moves (for inbound indirection use load,get or peek,get)
void Load()   { vA = dm[safe(vS)]; }
void Store()  { dm[safe(vD)] = vA; }
void Get()    { vA = dm[safe(vA)]; } 
void Peek()   { vA = dm[safe(vP)]; }
void Keep()   { dm[safe(vP)] = vA; }
// V moves (for outbound indirection use tov,load,put or tov,peek,put)
void Put()    { dm[safe(vA)] = vV; }
// Increments
void Inc()    { ++vA; }
void Dec()    { --vA; }
void IncS()   { ++vS; }
void DecS()   { --vS; }
void IncD()   { ++vD; }
void DecD()   { --vD; }
void IncE()   { ++vS; ++vD; }
void DecE()   { --vS; --vD; }
void IncP()   { ++vP; }
void DecP()   { --vP; }
// Immediates
void ImmA(METADATA x)  { enrange(x); vA = x; }
void ImmB(METADATA x)  { enrange(x); vB = x; }
void ImmR(METADATA x)  { enrange(x); vR = x; }
void ImmS(METADATA x)  { enrange(x); vS = x; }
void ImmD(METADATA x)  { enrange(x); vD = x; }
void ImmE(METADATA x)  { enrange(x); vS = x; vD = x; }
void ImmP(METADATA x)  { enrange(x); vP = x; }
void MsbA(METADATA x)  { vA |= enmsb(x); }
void MsbB(METADATA x)  { vB |= enmsb(x); }
void MsbR(METADATA x)  { vR |= enmsb(x); }
void MsbS(METADATA x)  { vS |= enmsb(x); }
void MsbD(METADATA x)  { vD |= enmsb(x); }
void MsbE(METADATA x)  { vS |= enmsb(x); vD |= enmsb(x); }
void MsbP(METADATA x)  { vP |= enmsb(x); }
// Transfers
void Swap()   { rSwap = vA; vA = vB; vB = rSwap; }
void Tob()    { vB = vA; }
void Tot()    { vT = vA; }
void Tor()    { vR = vA; }
void Tos()    { vS = vA; }
void Tod()    { vD = vA; }
void Top()    { vP = vA; }
void Tov()    { vV = vA; }
void Fromb()  { vA = vB; }
void Fromt()  { vA = vT; }
void Fromr()  { vA = vR; }
void Froms()  { vA = vS; }
void Fromd()  { vA = vD; }
void Fromp()  { vA = vP; }
void Fromv()  { vA = vV; }
// Stack parking
void Use1()   { vP = v1; }
void Use2()   { vP = v2; }
void Use3()   { vP = v3; }
void Use4()   { vP = v4; }
void Pto1()   { v1 = vP; }
void Pto2()   { v2 = vP; }
void Pto3()   { v3 = vP; }
void Pto4()   { v4 = vP; } // ? Dynamic jumps
// Jumps (static only) (an interpreter would enforce a 24-bit program space)
#define jmpz(label) if (vA == 0) { goto label; } // ZERO
#define jmpm(label) if (vA == NEG_MASK) { goto label; } // MAX_NEG
#define jmpn(label) if ((vA & NEG_MASK) == NEG_MASK) { goto label; } // NEG
#define jmpb(label) if ((vA & BIG_MASK) == BIG_MASK) { goto label; } // BIG
#define jmpeq(label) if (vA == vB) { goto label; } // ==
#define jmpne(label) if (vA != vB) { goto label; } // !=
#define jmple(label) if (vA <= vB) { goto label; } // <= (unsigned)
#define jmpge(label) if (vA >= vB) { goto label; } // >= (unsigned)
#define jmplt(label) if (vA < vB) { goto label; } // < (unsigned)
#define jmpgt(label) if (vA > vB) { goto label; } // > (unsigned)
#define jump(label) goto label; // UNCONDITIONAL
#define repeat(label) if (--vR != 0) { goto label; }
#define br(label) { __label__ lr; vL = (LNKT)&&lr; goto label; lr: ; }
#define link goto *vL;
// Machine metadata
void Mdm()    { vA = DM_WORDS; }
// Other
void Nop()    { ; }
#define halt(x) return x;
// ===========================================================================
// Convenient macros to save typing
#define add Add();
#define sub Sub();
#define or Or();
#define and And();
#define xor Xor();
#define not Not();
#define neg Neg();
#define shl Shl();
#define shr Shr();
#define load Load();
#define store Store();
#define get Get();
#define putd PutD();
#define putp PutP();
#define peek Peek();
#define keep Keep();
#define swap Swap();
#define inc Inc();
#define dec Dec();
#define incs IncS();
#define decs DecS();
#define incd IncD();
#define decd DecD();
#define ince IncE();
#define dece DecE();
#define incp IncP();
#define decp DecP();
#define imma(x) ImmA(x);
#define immb(x) ImmB(x);
#define immr(x) ImmR(x);
#define imms(x) ImmS(x);
#define immd(x) ImmD(x);
#define imme(x) ImmE(x);
#define immp(x) ImmP(x);
#define msba(x) MsbA(x);
#define msbb(x) MsbB(x);
#define msbr(x) MsbR(x);
#define msbs(x) MsbS(x);
#define msbd(x) MsbD(x);
#define msbe(x) MsbE(x);
#define msbp(x) MsbP(x);
#define tob Tob();
#define tot Tot();
#define tor Tor();
#define tos Tos();
#define tod Tod();
#define top Top();
#define tov Tov();
#define fromb Fromb();
#define fromt Fromt();
#define fromr Fromr();
#define froms Froms();
#define fromd Fromd();
#define fromp Fromp();
#define fromv Fromv();
#define use1 Use1();
#define use2 Use2();
#define use3 Use3();
#define use4 Use4();
#define pto1 Pto1();
#define pto2 Pto2();
#define pto3 Pto3();
#define pto4 Pto4();
#define mdm Mdm();
#define nop Nop();
// ===========================================================================
// Opcodes for interpreter
#define iNOP  0x00
#define iHALT 0x7f
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WORD_SIZE);
  return exampleProgram();
}
// ===========================================================================
// Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
int exampleProgram() {
#define vm_DM_WORDS 0x10000000
#define v_DM_WORDS  0x1000
#define v_PM_WORDS  0x1000
#define v_pm 0
#define v_dm v_PM_WORDS
// Actually not using v_rPC any more (using v1 instead)
#define v_rPC v_dm + v_DM_WORDS
#define v_instr v_rPC + 1
#define v_vA v_instr + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vT v_vL + 1
#define v_vR v_vT + 1
#define v_vS v_vR + 1
#define v_vD v_vS + 1
#define v_vP v_vD + 1
#define v_v1 v_vP + 1
#define v_v2 v_v1 + 1
#define v_v3 v_v2 + 1
#define v_v4 v_v3 + 1
// ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
// ---------------------------------------------------------------------------
// Using vP and v1 for program counter (actually this is cheating badly)
begin:
    immp(0)
// Process next instruction
next:
    peek
    incp
    tot
// case iNOP:
    jmpz(Nop)
// case iHALT:
    fromt
    immb(iHALT)
    jmpeq(Halt)
// default:
    halt(8)
// ---------------------------------------------------------------------------
Nop:
  link
// ---------------------------------------------------------------------------
Halt:
  halt(9)
// ---------------------------------------------------------------------------
// Program child's program memory then run program
program:
  immd(0)
  imma(iNOP)
  br(istore)
  imma(iHALT)
  br(istore)
  jump(run)
// ---------------------------------------------------------------------------
// Store instruction in vA to v_pm[vD++] in child's program memory
istore:
  store
  incd
  link
// ---------------------------------------------------------------------------
// Run child's program
run:
  br(next)
  jump(run)
// ---------------------------------------------------------------------------
// Fill vR words at v_dst with value in vA (fills 1 GB in about 0.36 seconds)
doFill:
  doFillLoop:
    store
    incd
    repeat(doFillLoop)
    link
// ---------------------------------------------------------------------------
// Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  immd(0)
  immr(DM_WORDS)
  link
// ---------------------------------------------------------------------------
// Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  immb(vm_DM_WORDS)
  sub
  jmpz(assertedParentSize)
    halt(FAILURE)
  assertedParentSize:
    link
// ---------------------------------------------------------------------------

} // end of exampleProgram
// ===========================================================================
