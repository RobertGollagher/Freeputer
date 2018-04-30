/*
Copyright © 2017, Robert Gollagher.
SPDX-License-Identifier: GPL-3.0+

Program:    qmisc.c
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20170729
Updated:    20180429+
Version:    pre-alpha-0.0.7.1+ for FVM 2.0
=======

                              This Edition:
                               Portable C
                            for Linux and gcc

                               ( ) [ ] { }

  20180430 DECISION: QMISC is REJECTED. The 4-stack-machine plan
  currently in README.md really is the best compromise overall.
  The performance of QMISC is too slow when having to emulate stacks.
  The 4-stack-machine is a better generalist machine.

  MAJOR CHANGE:
    There is now a link stack rather than a link register.
    The terminology is go and resume (analagous to call and return).
    The current number of elements on the link stack is held in vL.
    The link stack is intentionally small and of a fixed size.

  NOTES:
    Blank std.hld and std.rom files can be created thus:
      head -c 67108864 /dev/zero > std.hld
      head -c 262144 /dev/zero > std.rom

    TODO endianness considerations, word/byte loading and I/O considerations
    - Reconsider display/input thoughts

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
#define GRACEFUL SUCCESS
#define MAX_DM_WORDS 0x10000000 // <= 2^(WD_BITS-4) due to C limitations.
#define DM_WORDS  0x10000  // Must be some power of 2 <= MAX_DM_WORDS.
#define DM_MASK   DM_WORDS-1
#define MAX_RM_WORDS 0x10000000 // <= 2^(WD_BITS-4) due to C limitations.
#define RM_WORDS  0x10000  // Must be some power of 2 <= MAX_RM_WORDS.
#define RM_MASK   RM_WORDS-1
#define MAX_HD_WORDS 0x10000000 // <= 2^(WD_BITS-4) due to C limitations.
#define HD_WORDS  0x1000000  // Must be some power of 2 <= MAX_HD_WORDS.
#define HD_MASK   HD_WORDS-1
#define LS_LNKTS  0x100
#define vL_MAX    LS_LNKTS-1
#define nopasm "nop" // The name of the native hardware nop instruction
// There are only 4 accessible registers:
WORD vA = 0; // accumulator
WORD vB = 0; // operand register
WORD vT = 0; // temporary register
WORD vR = 0; // repeat register
WORD vL = 0; // link counter (not accessible)
LNKT ls[LS_LNKTS]; // link stack (not accessible)
WORD dm[DM_WORDS]; // ram data memory (Harvard architecture)
WORD rm[RM_WORDS]; // rom data memory (Harvard architecture)
WORD hd[HD_WORDS]; // stdhld
int startup();
int shutdown(int excode);
int exampleProgram();
// ---------------------------------------------------------------------------
#define rmFilename "std.rom"
FILE *rmHandle;
#define stdhldFilename "std.hld"
FILE *stdhldHandle;
// ---------------------------------------------------------------------------
METADATA safe(METADATA addr) { return addr & DM_MASK; }
METADATA enbyte(METADATA x)  { return x & BYTE_MASK; }
METADATA enrange(METADATA x) { return x & METADATA_MASK; }
METADATA enshift(METADATA x) { return x & SHIFT_MASK; }
WORD romsafe(WORD addr)      { return addr & RM_MASK; }
WORD hdsafe(WORD addr)       { return addr & HD_MASK; }
// ---------------------------------------------------------------------------
// TODO Hold() Give() Hw() nb? < > catch tracing nb?

// Arithmetic
void Add()    { vA+=vB; }
void Sub()    { vA-=vB; }
void Mul()    { vA*=vB; }
void Div()    { vA/=vB; }
void Mod()    { vA%=vB; }
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
void Lsa()    { vA = LS_LNKTS-vL; }
void Lse()    { vA = vL; }
void Hw()     { vA = HD_WORDS; }
// Other
#define nopasm "nop" // The name of the native hardware nop instruction
void Noop()   { __asm(nopasm); }
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
#define go(label) { \
  if (vL<vL_MAX) { \
      __label__ lr; ls[++vL] = (LNKT)&&lr; goto label; lr: ; \
  } else { \
      vA = FAILURE; halt \
  } \
}
#define rs { \
  if (vL>0) { \
    goto *(ls[vL--]); \
  } else { \
      vA = FAILURE; halt \
  } \
}
// Basic I/O (experimental)
#define in(label) vA = getchar(); // If fail goto label
#define out(label) putchar(vA); // If fail goto label
void Rom()    { vA = rm[romsafe(vB)]; } // Reconsider failure of everything
void Give()   { vA = hd[hdsafe(vB)]; }
void Hold()   { hd[hdsafe(vB)] = vA; }
// ===========================================================================
#define i(x) Imm(x);
#define add Add();
#define inc Inc();
#define dec Dec();
#define shl Shl();
#define shr Shr();
#define swap Swap();
#define tob Tob();
#define tor Tor();
#define tot Tot();

#define get Get();
#define put Put();
#define geti Geti();
#define puti Puti();
#define decm Decm();
#define incm Incm();


#define fromb Fromb();
#define fromt Fromt();
#define hw Hw();
#define rom Rom();
#define hold Hold();
#define give Give();
#define nop Noop();
// ===========================================================================
#define dbg \
{ __label__ pc; pc: \
  printf("pc:%08x vA:%08x vB:%08x vT:%08x vR:%08x vL:%08x ls[vL]:%08x -- \
ls[0]:%08x ls[1]:%08x ls[2]:%08x dm[0x1ff]:%08x dm[0x200]:%08x hd[0]:%08x rm[0]:%08x\n", \
&&pc, vA, vB, vT, vR, vL, ls[vL], ls[0], ls[1], ls[2], dm[0x1ff], dm[0x200], hd[0], rm[0]); }
// ===========================================================================
int main() {
  assert(sizeof(WORD) == WD_BYTES);
  assert(startup() == SUCCESS);
  int excode = exampleProgram();
  return shutdown(excode);
}
// ===========================================================================
int startup() {
  stdhldHandle = fopen(stdhldFilename, "r+b");
  if (!stdhldHandle) return FAILURE;
  if (fread(&hd,WD_BYTES,HD_WORDS,stdhldHandle) < HD_WORDS) {
    fclose(stdhldHandle);
    return FAILURE;
  }
  rmHandle = fopen(rmFilename, "rb");
  if (!rmHandle) {
    fclose(stdhldHandle);
    return FAILURE;
  }
  if (fread(&rm,WD_BYTES,RM_WORDS,rmHandle) < RM_WORDS) {
    fclose(rmHandle);
    fclose(stdhldHandle);
    return FAILURE;
  }
  return SUCCESS;
}

int shutdown(int excode) {
  int shutdown = GRACEFUL;
  if (fclose(rmHandle) == EOF) shutdown = FAILURE;
  if (fseek(stdhldHandle,0,SEEK_SET) !=0) {
    shutdown = FAILURE;
  } else {
    if (fwrite(&hd,WD_BYTES,HD_WORDS,stdhldHandle) < HD_WORDS) {
      shutdown = FAILURE;
    }
  }
  if (fclose(stdhldHandle) == EOF) shutdown = FAILURE;
  assert(shutdown == GRACEFUL);
  return excode;
}
// ===========================================================================
int exampleProgram() {

/*
go(begin)
done:
  halt
begin:
  i(0x7fffffff) // 4 seconds with -O3
countdown:    // This is too slow, FVM1 can do 9 seconds with boundary checks
  nop         //   and FVM1 is interpreted (not native like this 'qmisc.c').
  dec
  jmpb(done)
  jump(countdown)
*/

go(x0);
end:
  i(0)
  fromb
  halt
fail:
  i(1)
  fromb
  halt


// Stack emulation, init a SP
initds:
  i(0x200)
  fromb
  i(0x200) /*dsp*/
  put
  rs

dpush:
  i(0x200) /*dsp*/

  get // stack overflow check, max stack size 2 elems
  i(0x1fe)
  jmpe(fail)
  i(0x200)

  decm
  puti
  rs

dpop:
  i(0x200) /*dsp*/

  get // stack underflow check
  i(0x200)
  jmpe(fail)
  i(0x200)

  geti
  incm
  rs

x0:
  go(initds)
  i(0x12345678)
  fromb
                // PERFORMANCE TEST: repeat {push a value then pop it again}
  i(0x10000000) // 268 million =  10.1 sec without -O3, without boundary checks
  fromb         // 268 million =  21.6 sec without -O3, with boundary checks
  tor           // 268 million =   1.3 sec with -O3, without boundary checks
dbg             // 268 million =   1.7 sec with -O3, with boundary checks
  pploop:  // Although the above seems fast, in fact it is too slow,
    i(0x66666666) // considering FVM1 does 9 seconds non-natively for a 2 billion
    fromb         // countdown including boundary checks and full stack use.
    go(dpush)
    go(dpop)
    rpt(pploop)
dbg
  rs



/*
x0: // 4.2 seconds
  i(0x7fffffff) fromb tor loop: nop rpt(loop) rs
*/

/*
dbg
x0:
  i(0xffffff)
  fromb
  tor
  i(0x50)
  fromb
  i(0)
  loop:
    hold
    tot
    fromb
    i(1)
    add
    tob
    fromt
    rpt(loop)
  rs
*/

/*
// First use a hex editor to put the alphabet into the start of ROM, then
//   this little program will print out the alphabet from ROM:
testrom:
  i(16)
  fromb
  tor
  i(0)
  loop:
    rom
    swap
    tot
    swap
    out(end)
    i(8)
    shr
    out(end)
    shr
    out(end)
    shr
    out(end)
    fromt
    tob
    inc
    rpt(loop)
  rs

x0:
  go(testrom)
  rs
*/

/*
foo:
  i(4)
  dbg
  rs
bar:
  i(3)
  dbg
  go(foo)
  i(3)
  dbg
  rs
baz:
  i(2)
  dbg
  go(bar)
  i(2)
  dbg
  rs
x0:
  i(1)
  dbg
  go(baz)
  i(1)
  dbg
  rs
*/
}
// ===========================================================================


