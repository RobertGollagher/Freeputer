/*
             QUALITY MINIMAL INSTRUCTION SET COMPUTER (QMISC)
               Core aims to be less than 200 lines of code
                 and totally free of undefined behaviour.

Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20170818+
Version:    pre-alpha-0.0.0.59+ for FVM 2.0

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
int exampleProgram();
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
void v_Nop()    { ; }
#define halt return enbyte(vA);
// ===========================================================================
// Convenient macros to save typing
#define add Add();
#define sub Sub();
#define or Or();
#define and And();
#define xor Xor();
#define not Not();
#define flip Flip();
#define neg Neg();
#define shl Shl();
#define shr Shr();
#define get Get();
#define put Put();
#define inc Inc();
#define dec Dec();
#define i(x) Imm(x);
#define set Set();
#define imma ImmA();
#define immr ImmR();
#define immt ImmT();
#define immv ImmV();
#define swap Swap();
#define tob Tob();
#define tot Tot();
#define tor Tor();
#define tov Tov();
#define fromt Fromt();
#define fromr Fromr();
#define fromv Fromv();
#define mdm Mdm();
#define nop Nop();
// ===========================================================================
// Opcodes for interpreter of child VM (mostly arbitrary values for now).
// Current scheme is FW32 (poor density but simple, portable).
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
#define iIMM   0x80000000  // DONE
#define iNEG   0x0e000000
#define iIMMR  0x12000000
#define iIMMT  0x13000000
#define iIMMV  0x14000000
#define iSWAP  0x20000000
#define iTOB   0x21000000
#define iTOR   0x22000000
#define iTOT   0x23000000
#define iFROMB 0x31000000
#define iFROMR 0x32000000
#define iFROMT 0x33000000
#define iFROMV 0x34000000
#define iJMPZ  0x40000000
#define iJMPE  0x41000000
#define iJMPS  0x42000000
#define iJMPU  0x43000000
#define iRPT   0x4f000000
#define iBR    0x50000000
#define iLINK  0x60000000
#define iMDM   0x61000000
#define iHALT  0x6f000000 // DONE
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WORD_SIZE);
  return exampleProgram();
}
// ===========================================================================
// Example: to be a small QMISC FW32 implementation (vm_ = parent, v_ = child)
int exampleProgram() {
/* // For native parent VM speed comparison:
i(0x10000000)
immr
foo:
  rpt(foo)
  return 0;
*/
#define vm_DM_WORDS 0x10000000
#define v_DM_WORDS  0x1000
#define v_PM_WORDS  0x1000
#define v_pm 0
#define v_dm v_PM_WORDS
#define v_rPC v_dm + v_DM_WORDS
#define v_instr v_rPC + 1
#define v_vA v_instr + 1
#define v_vB v_vA + 1
#define v_vL v_vB + 1
#define v_vT v_vL + 1
#define v_vR v_vT + 1
#define v_vV v_vR + 1
#define v_vI v_vV + 1
// ---------------------------------------------------------------------------
vm_init:
  br(assertParentSize)
  br(setupToClearParent)
  br(doFill)
  jump(program)
// ---------------------------------------------------------------------------
// Process next instruction (not optimized yet)
next:
// get dm[v_rPC]
    i(v_rPC)
    imma
    tov // store v_rPC addr in vV
    get // get val of v_rPC into vT
    tot

#ifdef DEBUG
printf("\n%08x ",vT); // print v_rPC value
#endif

// increment dm[v_rPC]
    inc
    swap
    put
// get current instr into vT
    fromt // retrieve val of v_rPC from vT
    get   // get instr at dm[v_rPC] into vT
    tot

#ifdef DEBUG
printf("%08x ",vT); // print instruction value
#endif

// case iIMM:
    fromt
    jmpn(v_Imm)
// case iNOP:
    fromt
    jmpz(v_Nop)
// case iADD:
    fromt
    i(iADD)
    jmpe(v_Add)
// case iTOR
    fromt
    i(iTOR)
    jmpe(v_ImmR)
// case iRPT
    fromt
    br(exOpcode)
    i(iRPT)
    jmpe(v_Rpt)
// case iHALT:
    fromt
    i(iHALT)
    jmpe(v_Halt)
// default:
    i(ILLEGAL)
    imma
    halt
// ---------------------------------------------------------------------------
v_Nop:

#ifdef DEBUG
printf("nop ");
#endif

  jump(next)
// ---------------------------------------------------------------------------
v_Imm:
  flip

#ifdef DEBUG
printf("flip v_vA: %08x ",vA);
#endif

  br(setB)
  jump(next)
// ---------------------------------------------------------------------------
v_Add:
  br(preAB)
  add

#ifdef DEBUG
printf("add  v_vA: %08x ",vA);
#endif

  br(setA)
  jump(next)
// ---------------------------------------------------------------------------
/* PERFORMANCE of CHILD VM (virtualized within the parent VM):
    0x10000000 empty repeats in 2.6 sec, 0x7fffffff in 20.6 sec.
    Native empty repeat is 0.18 sec, 1.4 sec respectively, forced by asm("").
    Thus the child VM is about 15 times slower than its parent.
*/
v_Rpt:
  br(getR)

#ifdef DEBUG
printf("rpt  v_vR: %08x ",vA);
#endif

  dec
  tor       // Here using vR merely as a temporary register.
  br(setR)  // Here setting v_vR from vA (nothing to do with vR).
  fromr     // So now vA contains decremented value of v_vR from vR.
  jmpz(v_Repeat_end)
    fromt
    br(exMeta24)
    br(setPC)
  v_Repeat_end:
    jump(next)
// ---------------------------------------------------------------------------
v_ImmR:
  br(getB)

#ifdef DEBUG
printf("immr v_vR: %08x ",vA);
#endif

  br(setR)
  jump(next)
// ---------------------------------------------------------------------------
v_Halt:
  //i(SUCCESS)
  br(getA) // FIXME a hack to show v_vA value as exit code of parent VM

#ifdef DEBUG
printf("halt v_vA: %08x ",vA);
#endif

  halt
// ---------------------------------------------------------------------------
// Extract 8-bit opcode from vA into vA by masking vA.
// Note: not relevant for iIMM (which is indicated by msbit being set).
exOpcode:
  i(0xff000000)
  and
  link
// ---------------------------------------------------------------------------
// Extract 24-bit metadata from vA into vA by masking vA
exMeta24:
  i(0x00ffffff)
  and
  link
// ---------------------------------------------------------------------------
// Get v_vA into vA and v_vB into vB prior to AB operation
preAB:
  i(v_vA)
  imma
  get
  tot
  i(v_vB)
  imma
  get
  tob
  fromt
  link
// ---------------------------------------------------------------------------
// Get v_rPC into vA
getPC:
  i(v_rPC)
  imma
  get
  link
// ---------------------------------------------------------------------------
// Save vA into v_rPC
setPC:
  tov
  i(v_rPC)
  imma
  put
  link
// ---------------------------------------------------------------------------
// Get v_vR into vA
getR:
  i(v_vR)
  imma
  get
  link
// ---------------------------------------------------------------------------
// Save vA into v_vA
setA:
  tov
  i(v_vA)
  imma
  put
  link
// ---------------------------------------------------------------------------
// Save vA into v_vB
setB:
  tov
  i(v_vB)
  imma
  put
  link
// ---------------------------------------------------------------------------
// Save vA into v_vR
setR:
  tov
  i(v_vR)
  imma
  put
  link
// ---------------------------------------------------------------------------
// Get v_vA into vA
getA:
  i(v_vA)
  imma
  get
  link
// ---------------------------------------------------------------------------
// Get v_vB into vB
getB:
  i(v_vB)
  imma
  get
  link
// ---------------------------------------------------------------------------
// Program child's program memory then run program
program:
  i(0)
  imma
/*
  i(iNOP)
  br(si)
  i(iIMM|3)
  br(si)
  i(iADD)
  br(si)
  i(iIMM|5)
  br(si)
  i(iADD)
  br(si)
*/
  i(iIMM|0x10000000) // Performance test (these repeats take about 2.6 sec)
  br(si)
  i(iTOR)
  br(si)
  i(iNOP) // This is instruction 2 in this program.
  br(si)
  i(iRPT|2)
  br(si)
  i(iHALT)
  br(si)
  jump(next)
// ---------------------------------------------------------------------------
// Store instruction in vV to v_pm[vA++] in child's program memory
si:
  immv
  put
  inc
  link
// ---------------------------------------------------------------------------
// Fill vR words at dm[vA] with value in vV (fills 1 GB in about 0.63 seconds)
doFill:
  doFillLoop:
    put
    inc
    rpt(doFillLoop)
    link
// ---------------------------------------------------------------------------
// Set up to doFill so as to fill entire data memory of parent with zeros
setupToClearParent:
  i(0)
  immv
  i(DM_WORDS)
  immr
  link
// ---------------------------------------------------------------------------
// Assert that size of parent's data memory is exactly vm_DM_WORDS
assertParentSize:
  mdm
  i(vm_DM_WORDS)
  sub
  jmpz(assertedParentSize)
    i(FAILURE)
    imma
    halt
  assertedParentSize:
    link
// ---------------------------------------------------------------------------
// Increment variable at dm[vA]
Incr:
  tov
  get
  inc
  swap
  put
  link
// ---------------------------------------------------------------------------

} // end of exampleProgram
// ===========================================================================

