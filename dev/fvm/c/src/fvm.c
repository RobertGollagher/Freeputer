/*
                        FREEPUTER VIRTUAL MACHINE

Program:    fvm.c
Copyright © Robert Gollagher 2015
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150822
Updated:    20150913:2230
Version:    0.1.0.0 alpha for FVM 1.0

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
[at your option] any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

                              This Edition:
                               Portable C 
                                 for gcc

                               ( ) [ ] { }

==============================================================================

WARNING: This is alpha software. It is somewhat incomplete and
relatively unreliable. For a more complete and more reliable implementation
of an FVM 1.0 please use the x86 assembly language implementation
found in 'dev/fvm/x86/src/fvm.s'. That is the reference implementation.
Where the behaviour of this implementation differs from that of
the reference implementation, the behaviour of the reference implementation
(unless obviously a bug) should be taken to be the correct behaviour.

WARNING: Due to constraints on development time, the arithmetic operations
in this implementation have not yet been fully tested and are probably 
at least partially incorrect. Specifically, this implementation contains
algorithms intended to replicate the behaviour of the x86 instruction set
with respect to detecting integer overflow; those algorithms have not
been extensively tested and are probably incorrect in some cases.
Indeed for optimal performance you should provide inline assembly language
for your target platform to improve the performance of arithmetic
operations while retaining correct and safe FVM behaviour.
Be careful to avoid the undefined behaviours of C!

WARNING: Performance may be very poor unless you compile with appropriate
optimization flags. In particular it is important to ensure that your compiler
converts the large switch statement into a jump table. Other than the large
switch statement, the code has generally been written in the spirit of
assembly language to permit reasonable performance and to keep this
implementation logically inline with the reference implementation.
In general the FVM is best implemented in assembly language.

NOTE: this implementation assumes that no integer width greater than 32 bits
is available on the target architecture. Overflow checks could be written
more efficiently if the target platform supports widening arithmetic
operations to 64 bits as a means of detecting overflow.

NOTE: This code uses binary constants for reasons of legibility.
See https://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html

PLATFORMS: This portable 'fvm.c' code is known to compile and run on the
following platforms. It should also compile and run on many other platforms
with little or no modification. Linux is not required.

  (1) gcc (Debian 4.7.2-5) 4.7.2 on 32-bit x86 Linux (Debian 7.8)
      running on Intel i5 CPU (typical desktop computer)
  (2) gcc (Debian 4.6.3-14+rPi1) 4.6.3 on ARM Linux (Raspbian GNU/Linux 7)
      running on ARM11 CPU (Raspberry Pi Rev2 Model B, 512MB RAM)
  (3) mingw32-gcc.exe (tdm-1) 4.7.1 in Code::Blocks 13.12 on 64-bit Windows 8
      running on Microsoft Surface Pro 3 (tablet)

==============================================================================

To compile on any platform use:

  gcc -o fvm fvm.c

Or simply:

  make

However, that would result in very poor performance since it uses
no optimizations whatsoever. Therefore it is better to use one of the below.

To compile for best performance on 32-bit x86 use:

  gcc -march=native -m32 -mfpmath=sse -Ofast -flto -funroll-loops -o fvm fvm.c

Or simply:

  make x86

To compile for best performance on ARM11 (Raspberry Pi) use:

  gcc -march=armv6 -mfpu=vfp -mfloat-abi=hard -O3 -o fvm fvm.c

Or simply:

  make pi

To compile for reasonably good performance on any platform use:

  gcc -O3 -o fvm fvm.c

Or simply:

  make any

============================================================================*/
 
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WORD int32_t
#define WORD_AS_BITS uint32_t
#define BYTE uint8_t
#define NEG_INT_MAX (-2147483647 - 1) // C compiler quirk = -2147483648
#define POS_INT_MAX 2147483647

void clearMem();
void clearState();

// ===========================================================================
//                         COMPILER DIRECTIVES
// ===========================================================================
#define DEFINED 0 // Arbitrary value for defined symbols here

// For faster performance comment out the next line. The assembled FVM will
// then not check (when it executes each instruction in a program) whether or
// not it should produce trace output. The improvement in performance is
// typically very significant. The drawback is that TRON cannot then be
// usefully used to debug programs since it will have no effect.
#define TRON_ENABLED DEFINED

// Version stamp for FVM binary
#define version "fvm c version 0.1.0.0 alpha for FVM 1.0"

// ===========================================================================
//                             CONSTANTS
// ===========================================================================

// The following example VM sizings are arbitrary.
// You can have a much larger or much smaller VM instance as desired.
// However, maximum address space (in which RAM, ROM and MAP must fit)
// is 2147483648 bytes. Maximum STDBLK_SIZE is 2147483648 bytes.
// This would of course be addressed as 0 to 2147483647.

// ---------------------------------------------------------------------------
// This section specifies a medium-sized VM instance ('fvm16-16MB') having:
//   16 MB RAM, 16 MB ROM, 16 MB stdblk, no memory-mapped devices.
// Suitable for:
//   * Running the Freelang compiler; or
//   * Running the FreeLine text editor; or
//   * Running other programs having moderate requirements.
// Has:
//   ARBITRARY_MEMORY_SIZE = 16777216 bytes
//   STDBLK_SIZE = 16777216 bytes
// Uncomment the next four lines to use this size of VM instance:
//  #define ARBITRARY_MEMORY_SIZE           0b00000001000000000000000000000000
//  #define PROG_MEMORY_MASK                0b11111110000000000000000000000000
//  #define STDBLK_SIZE                     0b00000001000000000000000000000000
//  #define STDBLK_MEMORY_MASK              0b11111110000000000000000000000000
// Create std.blk file at Linux command line by:
//   head -c 16777216 /dev/zero > std.blk
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// This section specifies a medium-sized VM instance ('fvm16-0MB') having:
//   16 MB RAM, 16 MB ROM, 0-sized stdblk, no memory-mapped devices.
// Suitable for:
//   * Running the Freelang compiler; or
//   * Running other programs that do not use stdblk.
// Has:
//   ARBITRARY_MEMORY_SIZE = 16777216 bytes
//   STDBLK_SIZE = 0 bytes
// Uncomment the next four lines to use this size of VM instance:
//  #define ARBITRARY_MEMORY_SIZE           0b00000001000000000000000000000000
//  #define PROG_MEMORY_MASK                0b11111110000000000000000000000000
//  #define STDBLK_SIZE                     0b00000000000000000000000000000000
//  #define STDBLK_MEMORY_MASK              0b11111111111111111111111111111111
// Create 0-sized std.blk file at Linux command line by:
//   touch std.blk
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// This section specifies a small VM instance ('fvm16-0kB') having:
//   16 kB RAM, 16 kB ROM, 0-sized stdblk, no memory-mapped devices.
// Suitable for:
//   * Running the Freelang decompiler (far more than sufficient!); or
//   * Running other programs having small requirements.
// Has:
//   ARBITRARY_MEMORY_SIZE = 16384 bytes
//   STDBLK_SIZE = 0 bytes
// Uncomment the next four lines to use this size of VM instance:
    #define ARBITRARY_MEMORY_SIZE         0b00000000000000000100000000000000
    #define PROG_MEMORY_MASK              0b11111111111111111000000000000000
    #define STDBLK_SIZE                   0b00000000000000000000000000000000
    #define STDBLK_MEMORY_MASK            0b11111111111111111111111111111111
// Create 0-sized std.blk file at Linux command line by:
//   touch std.blk
// ---------------------------------------------------------------------------

// The following do not change
#define HALF_WORD_SIZE 2               // bytes
#define WORD_SIZE 4                    // bytes
#define TWO_WORDS_SIZE 8               // bytes
#define THREE_WORDS_SIZE 12            // bytes
#define FOUR_WORDS_SIZE 16             // bytes
#define MAX_DEPTH_DS 32                // elements (words) (power of 2)
#define MAX_DEPTH_RS 32                // elements (words) (power of 2)
#define MAX_DEPTH_SS 32                // elements (words) (power of 2)
// This VM implementation happens to make ROM and RAM the same size
//   as each other but that is not at all mandatory. Furthermore,
//   STDBLK_SIZE does not have to be related to these sizes
//   but must be a word-multiple or zero.
#define ROM_SIZE ARBITRARY_MEMORY_SIZE // ROM, RAM, MAP must be word-multiples
#define RAM_SIZE ARBITRARY_MEMORY_SIZE
#define ROM_SIZE_WDS ROM_SIZE / WORD_SIZE
// This VM implementation does not provide any memory-mapped device,
//   therefore we set MAP_SIZE to 0 here
#define MAP_SIZE 0                     // MAP immediately follows RAM
#define LOWEST_WRITABLE_BYTE ROM_SIZE  // RAM immediately follows ROM
#define HIGHEST_WRITABLE_BYTE ROM_SIZE + RAM_SIZE + MAP_SIZE - 1
#define HIGHEST_WRITABLE_WORD ROM_SIZE + RAM_SIZE + MAP_SIZE - WORD_SIZE
#define STDBLK 0
#define STDIN 1
#define STDIMP 2
#define STDOUT -1
#define STDEXP -2
#define FALSE 0
#define TRUE 1
#define LOWEST_SIMPLE_OPCODE iEXIT
#define OPCODE_MASK 0b11111111111111111111111100000000
#define LAST_RESTART_CODE_RESET 1     // Indicates program-requested RESET
#define LAST_RESTART_CODE_REBOOT 2    // Indicates program-requested REBOOT
#define stdblkFilename         "std.blk"   // Name of file for standard block
#define romFilename         "rom.fp"       // Name of file for system ROM
#define stdtrcFilename         "std.trc"   // Name of file for standard trace
#define stdexpFilename         "std.exp"   // Name of file for standard export
#define stdimpFilename         "std.imp"   // Name of file for standard import
// Error messages for traps (these go to stdtrc)
#define msgTrapIllegalOpcode              "ILLEGAL OPCODE    "
#define msgTrapMathOverflow               "MATH OVERFLOW     "
#define msgTrapDsOverflow                 "DS OVERFLOW       "
#define msgTrapDsUnderflow                "DS UNDERFLOW      "
#define msgTrapRsOverflow                 "RS OVERFLOW       "
#define msgTrapRsUnderflow                "RS UNDERFLOW      "
#define msgTrapSsOverflow                 "SS OVERFLOW       "
#define msgTrapSsUnderflow                "SS UNDERFLOW      "
#define msgTrapXsBitshift                 "XS BITSHIFT       "
#define msgTrapMemBounds                  "BEYOND MEM BOUNDS "
#define msgTrapRAMBounds                  "BEYOND RAM BOUNDS "
#define msgTrapCantOpenStdblk             "CAN'T OPEN STDBLK "
#define msgTrapCantCloseStdblk            "CAN'T CLOSE STDBLK"
#define msgTrapCantOpenRom                "CAN'T OPEN ROM    "
#define msgTrapCantCloseRom               "CAN'T CLOSE ROM   "
#define msgTrapCantReadRom                "CAN'T READ ROM    "
#define msgTrapCantOpenStdexp             "CAN'T OPEN STDEXP "
#define msgTrapCantCloseStdexp            "CAN'T CLOSE STDEXP"
#define msgTrapCantOpenStdimp             "CAN'T OPEN STDIMP "
#define msgTrapCantCloseStdimp            "CAN'T CLOSE STDIMP"
#define msgTrapDivideByZero               "DIVIDE BY ZERO    "
#define msgTrapWall                       "HIT WALL          "
#define msgTrapData                       "HIT DATA          "
#define msgTrapPcOverflow                 "PC OVERFLOW       "
#define msgBefore                         " just before:     "

// ===========================================================================
//                               VARIABLES
// ===========================================================================

WORD rchannel;                     // Channel for READOR
WORD wchannel;                     // Channel for WRITOR
WORD gchannel;                     // Channel for GETOR
WORD pchannel;                     // Channel for PUTOR
WORD readBuf;                      // Tiny buffer for READOR
WORD writeBuf;                     // Tiny buffer for WRITOR
WORD getBuf;                       // Tiny buffer for GETOR
WORD putBuf;                       // Tiny buffer for PUTOR
FILE *stdblkHandle;                // File handle for stdblk file
FILE *romHandle;                   // File handle for ROM file
FILE *stdtrcHandle;                // File handle for stdtrc file
FILE *stdexpHandle;                // File handle for stdexp file
FILE *stdimpHandle;                // File handle for stdimp file

WORD pcTmp;                        // Only used when need to park pc

WORD *rsp;                         // Return stack pointer
#define rsStop MAX_DEPTH_RS        // rs index that bookends its start
WORD rs[rsStop+1];  // Return stack plus one word of empty space for safety
#define RS_EMPTY &rs[rsStop]
#define RS_HAS_ONE &rs[rsStop-1]
#define RS_FULL &rs[0]

// TODO consider if these arrays should be static and, along with everything
// else, moved inside runfvm
WORD *ssp;                         // Software stack pointer
#define ssStop MAX_DEPTH_SS        // ss index that bookends its start
WORD ss[ssStop+1];  // Software stack plus one word of empty space for safety
#define SS_EMPTY &ss[ssStop]
#define SS_HAS_ONE &ss[ssStop-1]
#define SS_HAS_TWO &ss[ssStop-2]
#define SS_HAS_THREE &ss[ssStop-3]
#define SS_HAS_FOUR &ss[ssStop-4]
#define SS_FULL &ss[0]
#define SS_ONE_LESS_FULL &ss[1]
#define SS_TWO_LESS_FULL &ss[2]
#define SS_THREE_LESS_FULL &ss[3]
#define SS_FOUR_LESS_FULL &ss[4]

WORD *dsp;                         // Data stack pointer
#define dsStop MAX_DEPTH_DS        // ds index that bookends its start
WORD ds[dsStop+1];  // Data stack plus one word of empty space for safety
BYTE memory[ROM_SIZE + RAM_SIZE + MAP_SIZE]; // System memory (power of 2)
WORD *memoryWords = (WORD *)memory;
  // Note that memory is the last real variable other than
  //   lastExitCode and lastRestartCode below
#define DS_EMPTY &ds[dsStop]
#define DS_HAS_ONE &ds[dsStop-1]
#define DS_HAS_TWO &ds[dsStop-2]
#define DS_HAS_THREE &ds[dsStop-3]
#define DS_HAS_FOUR &ds[dsStop-4]
#define DS_FULL &ds[0]
#define DS_ONE_LESS_FULL &ds[1]
#define DS_TWO_LESS_FULL &ds[2]
#define DS_THREE_LESS_FULL &ds[3]
#define DS_FOUR_LESS_FULL &ds[4]

WORD lastExitCode;             // Last automated exit code (if any),
                                    //   not preserved after a hard reset.
WORD eMark;                    // Merely marks end of FVM memory space
                                    //   except for lastRestartCode.
WORD lastRestartCode;          // Program-requested restart code (if any)
                                    //   preserved even after a hard reset.

// ===========================================================================
//        VARIABLES FOR TRACING (optional for production VM)
// ===========================================================================
BYTE vmFlags = 0;   // Flags -------1 = trace on

// ===========================================================================
//                          FVM RUNTIME LOGIC
//       In C this has to be packaged into a runfvm() function
//              to allow use of goto and labels for speed  
// ===========================================================================
int runfvm() {

#ifdef TRON_ENABLED // traceTable: 
const char *traceTable[] = { // Must be in same order as opcodeTable
  "===     ",
  "lit     ",
  "call    ",
  "go      ",
  "go[>0]  ",
  "go[>=0] ",
  "go[==0] ",
  "go[!=0] ",
  "go[<=0] ",
  "go[<0]  ",
  "go[>]   ",
  "go[>=]  ",
  "go[==]  ",
  "go[!=]  ",
  "go[<=]  ",
  "go[<]   ",
  "go>0    ",
  "go>=0   ",
  "go==0   ",
  "go!=0   ",
  "go<=0   ",
  "go<0    ",
  "go>     ",
  "go>=    ",
  "go==    ",
  "go!=    ",
  "go<=    ",
  "go<     ",
  "reador  ",
  "writor  ",
  "tracor  ",
  "getor   ",
  "putor   ",
  "readorb ",
  "writorb ",
  "tracorb ",
  "getorb  ",
  "putorb  ",
  "        ", // Start of 107 empty fillers in blocks of 10...
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           // 10 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",           //  7 empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ", // End of 107 empty fillers in blocks of 10
  "ret     ",
  "invoke  ",
  "[invoke]",
  "fly     ",
  "swap    ",
  "over    ",
  "rot     ",
  "tor     ",
  "leap    ",
  "nip     ",
  "tuck    ",
  "rev     ",
  "rpush   ",
  "rpop    ",
  "drop    ",
  "drop2   ",
  "drop3   ",
  "drop4   ",
  "dup     ",
  "dup2    ",
  "dup3    ",
  "dup4    ",
  "hold    ",
  "hold2   ",
  "hold3   ",
  "hold4   ",
  "speek   ",
  "speek2  ",
  "speek3  ",
  "speek4  ",
  "spush   ",
  "spush2  ",
  "spush3  ",
  "spush4  ",
  "spop    ",
  "spop2   ",
  "spop3   ",
  "spop4   ",
  "dec     ",
  "decw    ",
  "dec2w   ",
  "inc     ",
  "incw    ",
  "inc2w   ",
  "@       ",
  "!       ",
  "[@]     ",
  "@b      ",
  "!b      ",
  "[@b]    ",
  "@@      ",
  "@!      ",
  "[@@]    ",
  "@@b     ",
  "@!b     ",
  "[@@b]   ",
  "+       ",
  "-       ",
  "*       ",
  "/       ",
  "%       ",
  "/%      ",
  "[+]     ",
  "[-]     ",
  "[*]     ",
  "[/]     ",
  "[%]     ",
  "[/%]    ",
  "neg     ",
  "abs     ",
  "&       ",
  "|       ",
  "^       ",
  "[&]     ",
  "[|]     ",
  "[^]     ",
  "<<      ",
  ">>      ",
  "[<<]    ",
  "[>>]    ",
  "move    ",
  "fill    ",
  "find    ",
  "match   ",
  "moveb   ",
  "fillb   ",
  "findb   ",
  "matchb  ",
  "homio   ",
  "rchan   ",
  "wchan   ",
  "gchan   ",
  "pchan   ",
  "ecode?  ",
  "rcode?  ",
  "rom?    ",
  "ram?    ",
  "map?    ",
  "stdblk? ",
  "ds?     ",
  "ss?     ",
  "rs?     ",
  "dsn?    ",
  "ssn?    ",
  "rsn?    ",
  "tron    ",
  "troff   ",
  "reset   ",
  "reboot  ",
  "halt    ",
  "data    " };
#endif // .ifdef TRON_ENABLED

  goto start;

// Note: The FVM is a stack machine. It has no exposed registers.
// The registers referred to below are physical or virtual registers of the
// target architecture for which this FVM implementation is to be compiled.
// Note: This C implementation of the FVM is a port from the reference
// implementation which was written in x86 assembly language.
register WORD pc = 0; // Program counter (was %esi) in bytes
register WORD rA = 0; // Register A      (was %eax)
register WORD rB = 0; // Register B      (was %ebx)
register WORD rC = 0; // Register C      (was %ecx)
register WORD rD = 0; // Register D      (was %edx)
register WORD rE = 0; // Register E      (was %esi)
register WORD rF = 0; // Register F      (was %edi)

// ===========================================================================
//          MACROS for tracing (optional for production VM)
// ===========================================================================

// Trace if trace flag set in vmFlags
#define optTrace \
  if ((vmFlags & 0b00000001) == 1) { \
    traceInfo \
    traceStacks \
  }

// ===========================================================================
//                               MACROS
// ===========================================================================

// Increments the program counter
#define incPc \
  pc = pc + WORD_SIZE; \
  if ((PROG_MEMORY_MASK & pc) != 0) { \
    goto trapPcOverflow; \
  }

// Puts word at current program cell into rA
#define wordAtPc \
  rA = *(WORD *)&memory[pc];

// Puts word at current program cell into rA
#define wordAtPcWORKS \
  rA = memoryWords[pc/WORD_SIZE];

// Branches to address in the next program cell
// Note: the check below ensures branch address lies within program memory
#define branch \
  pc = *(WORD *)&memory[pc]; \
  if ((PROG_MEMORY_MASK & pc) != 0) { \
    goto trapPcOverflow; \
  }

// Skips the next program cell (rather than branching)
#define dontBranch \
  incPc

// Reset to using default IO channels
#define setIOdefaults \
  gchannel = 0 ; \
  pchannel = 0 ; \
  rchannel = 1 ; \
  wchannel = -1 ;

// Pushes value in rA onto the return stack
#define pushRs \
  if (rsp <= RS_FULL) { \
    goto trapRsOverflow; \
  } \
  rsp--; \
  *rsp = rA; \

// Pushes value in rA onto the return stack
#define pushSs \
  if (ssp <= SS_FULL) { \
    goto trapSsOverflow; \
  } \
  ssp-- ; \
  *ssp = rA; \

// Pushes value in rA onto the data stack
#define pushDs \
  if (dsp <= DS_FULL) { \
    goto trapDsOverflow; \
  } \
  dsp--; \
  *dsp = rA; \

// Peeks at the top of the data stack into rA
#define peekDs \
  if (dsp > DS_HAS_ONE) { \
    goto trapDsUnderflow; \
  } \
  rA = *dsp;

// Peeks at the second element from the top of the data stack into rB
#define peekSecondDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  rB = *(dsp+1);

// Peeks at the third element from the top of the data stack into rC
#define peekThirdDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  rC = *(dsp+2);

// Peeks twice from the data stack, first into rA and second into rB.
// WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
#define twoPeekDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  rA = *dsp; \
  rB = *(dsp+1);

// Peeks thrice from the data stack, first into rA, second into rB
// and third into rC.
// WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO threePopDS.
#define threePeekDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  rA = *dsp; \
  rB = *(dsp+1); \
  rC = *(dsp+2);

// Peeks four times from the data stack, first into rA, second into rB,
// third into rC and fourth into rD.
// WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO fourPopDS.
#define fourPeekDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  rA = *dsp; \
  rB = *(dsp+1); \
  rC = *(dsp+2); \
  rD = *(dsp+3);

// Replaces the value on top of the data stack with value in %eax
#define replaceDs \
  if (dsp > DS_HAS_ONE) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rA;

// Replaces the second value from top of the data stack with value in rB
#define replaceSecondDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  dsp[1] = rB;

// Replaces the third value from top of the data stack with value in rC
#define replaceThirdDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
    movl dsp, %eax \
  dsp[3] = rC;

// Replaces the value on top of the data stack with value in rA
//   and second-top value on the data stack with value in rB
// WARNING: THIS IS INTENTIONALLY OPPOSITE ORDER TO twoPopDS.
#define twoReplaceDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rA; \
  dsp[1] = rB;

// Pushes first rB and second rA onto the data stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
#define twoPushDs \
  if (dsp < DS_TWO_LESS_FULL) { \
    goto trapDsOverflow; \
  } \
    (--dsp)[0] = rB; \
    (--dsp)[0] = rA;

// Pushes first rB and second rA onto the software stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER TO twoPopSS.
#define twoPushSs \
  if (ssp < SS_TWO_LESS_FULL) { \
    goto trapSsOverflow; \
  } \
    (--ssp)[0] = rB; \
    (--ssp)[0] = rA;

// Pushes first rC and second rB and third rA onto the data stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER TO threePopSS.
#define threePushDs \
  if (dsp < DS_THREE_LESS_FULL) { \
    goto trapDsOverflow; \
  } \
    (--dsp)[0] = rC; \
    (--dsp)[0] = rB; \
    (--dsp)[0] = rA;

// Pushes first rD and second rC and third rB and fourth rA
//   onto the data stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
#define fourPushDs \
  if (dsp < DS_FOUR_LESS_FULL) { \
    goto trapDsOverflow; \
  } \
    *(--dsp) = rD; \
    *(--dsp) = rC; \
    *(--dsp) = rB; \
    *(--dsp) = rA;

// Pushes first rC and second rB and third rA onto the software stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER AS threePopSS.
#define threePushSs \
  if (ssp < SS_THREE_LESS_FULL) { \
    goto trapSsOverflow; \
  } \
    *(--ssp) = rC; \
    *(--ssp) = rB; \
    *(--ssp) = rA;

// Pushes first rD and second rC and third rB and fourth rA
//   onto the software stack
// WARNING: THIS IS INTENTIONALLY SAME ORDER AS fourPopSS.
#define fourPushSs \
  if (ssp < SS_FOUR_LESS_FULL) { \
    goto trapSsOverflow; \
  } \
    *(--ssp) = rD; \
    *(--ssp) = rC; \
    *(--ssp) = rB; \
    *(--ssp) = rA;

// Replaces the value on top of the data stack with value in rC
//   and second-top value on the data stack with value in rA
//   and third-top value on the data stack with value in rB
// WARNING: THIS IS INTENTIONALLY DIFFERENT TO threePeekDs.
#define threeReplaceDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rC; \
  dsp[1] = rA; \
  dsp[2] = rB; \

// Replaces the value on top of the data stack with value in rB
//   and second-top value on the data stack with value in rC
//   and third-top value on the data stack with value in rA
// WARNING: THIS IS INTENTIONALLY DIFFERENT TO threeReplaceDs.
#define threeRReplaceDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rB; \
  dsp[1] = rC; \
  dsp[2] = rA; \

// Pop from the data stack into rA.
#define popDs \
  if (dsp > DS_HAS_ONE) { \
    goto trapDsUnderflow; \
  } \
  rA = *dsp; \
  dsp++;

// Pop from the return stack into rA.
#define popRs \
  if (rsp > RS_HAS_ONE) { \
    goto trapRsUnderflow; \
  } \
  rA = *rsp; \
  rsp++;

// Peek from the software stack into rA.
#define peekSs \
  if (ssp > SS_HAS_ONE) { \
    goto trapSsUnderflow; \
  } \
  rA = *ssp; \

// Peeks twice from the software stack, first into rB and second into rA.
#define twoPeekSs \
  if (ssp > SS_HAS_TWO) { \
    goto trapSsUnderflow; \
  } \
  rB = *ssp; \
  rA = *(ssp+1);

// Peeks thrice from the software stack, first into rC and second into rB
// and third into rA.
#define threePeekSs \
  if (ssp > SS_HAS_THREE) { \
    goto trapSsUnderflow; \
  } \
  rC = *ssp; \
  rB = *(ssp+1); \
  rA = *(ssp+2);

// Peeks 4 times from the software stack, first into rD and 2nd into rC
// and third into rB and fourth into rA.
#define fourPeekSs \
  if (ssp > SS_HAS_FOUR) { \
    goto trapSsUnderflow; \
  } \
  rD = *ssp; \
  rC = *(ssp+1); \
  rB = *(ssp+2); \
  rA = *(ssp+3);

// Pop from the software stack into rA.
#define popSs \
  if (ssp > SS_HAS_ONE) { \
    goto trapSsUnderflow; \
  } \
  rA = *ssp; \
  ssp++;

// Pops twice from the software stack, first into rB and second into rA.
#define twoPopSs \
  if (ssp > SS_HAS_TWO) { \
    goto trapSsUnderflow; \
  } \
  rB = *ssp; \
  ssp++; \
  rA = *ssp; \
  ssp++;

// Pops thrice from the software stack, first into rC and second into rB
// and third into rA.
#define threePopSs \
  if (ssp > SS_HAS_THREE) { \
    goto trapSsUnderflow; \
  } \
  rC = *ssp; \
  ssp++; \
  rB = *ssp; \
  ssp++; \
  rA = *ssp; \
  ssp++;

// Pops 4 times from the software stack, first into rD and second into rC
// and third into rB and fourth into rA.
#define fourPopSs \
  if (ssp > SS_HAS_FOUR) { \
    goto trapSsUnderflow; \
  } \
  rD = *ssp; \
  ssp++; \
  rC = *ssp; \
  ssp++; \
  rB = *ssp; \
  ssp++; \
  rA = *ssp; \
  ssp++;

// Pops twice from the data stack, first into rB and second into rA.
// WARNING: INTENTIONALLY OPPOSITE ORDER TO twoPeekDS and twoReplaceDs.
#define twoPopDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  rB = *dsp; \
  dsp++; \
  rA = *dsp; \
  dsp++;

// Pops thrice from the data stack, first into rC and second into rB
// and third into rA.
// WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs.
#define threePopDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  rC = *dsp; \
  dsp++; \
  rB = *dsp; \
  dsp++; \
  rA = *dsp; \
  dsp++;

// Pops 4 times from the data stack, first into rD and second into rC
// and third into rB and fourth into rA.
// WARNING: INTENTIONALLY OPPOSITE ORDER TO threeReplaceDs.
#define fourPopDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  rD = *dsp; \
  dsp++; \
  rC = *dsp; \
  dsp++; \
  rB = *dsp; \
  dsp++; \
  rA = *dsp; \
  dsp++;

// Drops from the data stack
#define dropDs \
  if (dsp > DS_HAS_ONE) { \
    goto trapDsUnderflow; \
  } \
  dsp++;

// Drops 2 elements from the data stack
#define twoDropDs \
  if (dsp > DS_HAS_TWO) { \
    goto trapDsUnderflow; \
  } \
  dsp = dsp + 2;

// Drops 3 elements from the data stack
#define threeDropDs \
  if (dsp > DS_HAS_THREE) { \
    goto trapDsUnderflow; \
  } \
  dsp = dsp + 3;

// Drops 4 elements from the data stack
#define fourDropDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  dsp = dsp + 4;

#define ensureByteAddrWritable(reg) \
  if ((reg < LOWEST_WRITABLE_BYTE) || (reg > HIGHEST_WRITABLE_BYTE) ) { \
    goto trapRAMBounds; \
  }

#define ensureWordAddrWritable(reg) \
  if ((reg < LOWEST_WRITABLE_BYTE) || (reg > HIGHEST_WRITABLE_WORD) ) { \
    goto trapRAMBounds; \
  }

#define ensureByteAddressable(reg) \
  if ((reg < 0) || (reg > HIGHEST_WRITABLE_BYTE) ) { \
    goto trapMemBounds; \
  }

#define ensureWordAddressable(reg) \
  if ((reg < 0) || (reg > HIGHEST_WRITABLE_WORD) ) { \
    goto trapMemBounds; \
  }

// ===========================================================================
//                 TRACING (optional for production VM)
// ===========================================================================
#define traceInfo \
  { \
    WORD opcode = *(WORD *)&memory[pc]; \
    if (opcode >= 0 && opcode <=256) {\
      fprintf(stdtrcHandle, "%08x %-8s ", pc, traceTable[opcode]); \
    } else { \
        fprintf(stdtrcHandle, "%08x  ", pc); \
    } \
    if (opcode < LOWEST_SIMPLE_OPCODE && pc < (HIGHEST_WRITABLE_WORD - WORD_SIZE)) { \
      WORD cellValue = *(WORD *)&memory[pc+WORD_SIZE]; \
      fprintf(stdtrcHandle, "%08x ", cellValue); \
    } else { \
      fprintf(stdtrcHandle, "         "); \
    } \
  } 
#define traceStacks \
  { \
  fprintf(stdtrcHandle, "( "); \
  int i = 1; \
  int numElems = (DS_EMPTY-dsp); \
  for (i; i<=numElems; i++) {fprintf(stdtrcHandle, "%08x ", ds[dsStop-i]);}  \
  fprintf(stdtrcHandle, ") "); \
  fprintf(stdtrcHandle, "[ "); \
  i = 1; \
  numElems = (SS_EMPTY-ssp); \
  for (i; i<=numElems; i++) {fprintf(stdtrcHandle, "%08x ", ss[ssStop-i]);}  \
  fprintf(stdtrcHandle, "] "); \
  fprintf(stdtrcHandle, "{ "); \
  i = 1; \
  numElems = (RS_EMPTY-rsp); \
  for (i; i<=numElems; i++) {fprintf(stdtrcHandle, "%08x ", rs[rsStop-i]);}  \
  fprintf(stdtrcHandle, "} \n"); \
  } \

// ===========================================================================
//                           INSTRUCTION SET
// ===========================================================================
// opcodeTable
  // haltOpcode (1)
    #define iWALL 0 // WALL must be zero
  // complexOpcodes (37)
    #define iLIT 1
    #define iCALL 2
    #define iJMP 3
    #define iBRGZ 4
    #define iBRGEZ 5
    #define iBRZ 6
    #define iBRNZ 7
    #define iBRLEZ 8
    #define iBRLZ 9
    #define iBRG 10
    #define iBRGE 11
    #define iBRE 12
    #define iBRNE 13
    #define iBRLE 14
    #define iBRL 15
    #define iJGZ 16
    #define iJGEZ 17
    #define iJZ 18
    #define iJNZ 19
    #define iJLEZ 20
    #define iJLZ 21
    #define iJG 22
    #define iJGE 23
    #define iJE 24
    #define iJNE 25
    #define iJLE 26
    #define iJL 27
    #define iREADOR 28
    #define iWRITOR 29
    #define iTRACOR 30
    #define iGETOR 31
    #define iPUTOR 32
    #define iREADORB 33
    #define iWRITORB 34
    #define iTRACORB 35
    #define iGETORB 36
    #define iPUTORB 37
  // complexOpcodesEnd
  // simpleOpcodes (111)
    #define iEXIT 145
    #define iDCALL 146
    #define iRDCALL 147
    #define iDJMP 148
    #define iSWAP 149
    #define iOVER 150
    #define iROT 151
    #define iTOR 152
    #define iLEAP 153
    #define iNIP 154
    #define iTUCK 155
    #define iREV 156
    #define iRPUSH 157
    #define iRPOP 158
    #define iDROP 159
    #define iDROP2 160
    #define iDROP3 161
    #define iDROP4 162
    #define iDUP 163
    #define iDUP2 164
    #define iDUP3 165
    #define iDUP4 166
    #define iHOLD 167
    #define iHOLD2 168
    #define iHOLD3 169
    #define iHOLD4 170
    #define iSPEEK 171
    #define iSPEEK2 172
    #define iSPEEK3 173
    #define iSPEEK4 174
    #define iSPUSH 175
    #define iSPUSH2 176
    #define iSPUSH3 177
    #define iSPUSH4 178
    #define iSPOP 179
    #define iSPOP2 180
    #define iSPOP3 181
    #define iSPOP4 182
    #define iDEC 183
    #define iDECW 184
    #define iDEC2W 185
    #define iINC 186
    #define iINCW 187
    #define iINC2W 188
    #define iLOAD 189
    #define iSTORE 190
    #define iRLOAD 191
    #define iLOADB 192
    #define iSTOREB 193
    #define iRLOADB 194
    #define iPLOAD 195
    #define iPSTORE 196
    #define iRPLOAD 197
    #define iPLOADB 198
    #define iPSTOREB 199
    #define iRPLOADB 200
    #define iADD 201
    #define iSUB 202
    #define iMUL 203
    #define iDIV 204
    #define iMOD 205
    #define iDIVMOD 206
    #define iRADD 207
    #define iRSUB 208
    #define iRMUL 209
    #define iRDIV 210
    #define iRMOD 211
    #define iRDIVMOD 212
    #define iNEG 213
    #define iABS 214
    #define iAND 215
    #define iOR 216
    #define iXOR 217
    #define iRAND 218
    #define iROR 219
    #define iRXOR 220
    #define iSHL 221
    #define iSHR 222
    #define iRSHL 223
    #define iRSHR 224
    #define iMOVE 225
    #define iFILL 226
    #define iFIND 227
    #define iMATCH 228
    #define iMOVEB 229
    #define iFILLB 230
    #define iFINDB 231
    #define iMATCHB 232
    #define iHOMIO 233
    #define iRCHAN 234
    #define iWCHAN 235
    #define iGCHAN 236
    #define iPCHAN 237
    #define iECODE 238
    #define iRCODE 239
    #define iROM 240
    #define iRAM 241
    #define iMAP 242
    #define iSTDBLK 243
    #define iDS 244
    #define iSS 245
    #define iRS 246
    #define iDSN 247
    #define iSSN 248
    #define iRSN 249
    #define iTRON 250
    #define iTROFF 251
    #define iRESET 252
    #define iREBOOT 253
    #define iHALT 254
    #define iDATA 255
  // simpleOpcodesEnd
// opcodeTableEnd

// ===========================================================================
//                   EXAMPLE OF INDIRECT THREADED PROGRAM
// ===========================================================================
//  Uncomment this section, along with systemCopyProgram below, to copy
//  the below hardcoded example program into ROM rather than loading a program
//  into ROM from the ROM file (as systemLoadProgram normally does).
//  You must also comment out systemLoadProgram.
//
//  Note: either way, performance is the same since the actual program
//  (once copied or loaded into ROM) executes from ROM just the same.
//  To obtain faster performance one could in theory either write a
//  just-in-time compiler for Freeputer programs that would nicely compile
//  FVM instructions into native machine code or use hardcoded direct
//  threading rather than hardcoded indirect threading. However,
//  one must bear in mind that the safety and lack of undefined behaviour
//  of the FVM must be retained (not discarded in favour of speed).
/*
  #define COUNTER 2147483647        // number from which to count down to 0
  #define PROGRAM_SIZE 7 
  static WORD itProg[PROGRAM_SIZE] = {
    iLIT, COUNTER,
    iDEC,                   // countdown:
    iBRGZ, 8,               // branch to byte address of countdown label
    iDROP,
    iHALT
  };
*/

// ===========================================================================
//                          PRIVATE SERVICES
// ===========================================================================

/* Open stdtrc

   Normally this should do:
            stdtrcHandle = fopen(stdtrcFilename, "w")
   but for fvm16-16MB-sr-append for fvmtest change this to:
            stdtrcHandle = fopen(stdtrcFilename, "a")            
*/
#define openStdtrc \
  stdtrcHandle = fopen(stdtrcFilename, "w"); \
  if (!stdtrcHandle) { \
    goto trapCantOpenStdtrc; \
  }

/* Close stdtrc */
#define closeStdtrc \
    if (fclose(stdtrcHandle) == EOF) { \
      goto trapCantCloseStdtrc; \
  }

/* Open stdexp */
#define openStdexp \
  stdexpHandle = fopen(stdexpFilename, "w"); \
  if (!stdexpHandle) { \
    goto trapCantOpenStdexp; \
  }

/* Close stdexp */
#define closeStdexp \
    if (fclose(stdexpHandle) == EOF) { \
      goto trapCantCloseStdexp; \
  }

/* Open stdimp */
#define openStdimp \
  stdimpHandle = fopen(stdimpFilename, "r"); \
  if (!stdimpHandle) { \
    goto trapCantOpenStdimp; \
  }

/* Close stdimp */
#define closeStdimp \
    if (fclose(stdimpHandle) == EOF) { \
      goto trapCantCloseStdimp; \
  }

/* Open ROM */
#define openRom \
  romHandle = fopen(romFilename, "r"); \
  if (!romHandle) { \
    goto trapCantOpenRom; \
  }

/* Close ROM */
#define closeRom \
    if (fclose(romHandle) == EOF) { \
      goto trapCantCloseRom; \
  }

/* Open stdblk */
#define openStdblk \
  stdblkHandle = fopen(stdblkFilename, "r+b"); \
  if (!stdblkHandle) { \
    goto trapCantOpenStdblk; \
  }

/* Close stdblk */
#define closeStdblk \
    if (fclose(stdblkHandle) == EOF) { \
      goto trapCantCloseStdblk; \
  }

// ===========================================================================
//                            RESET POINTS
// ===========================================================================
/* Wipe entire FVM memory, stacks and variables */
systemHardReset:
  clearState();
  clearMem();
  goto systemInitDevices;

/* Wipe FVM stacks and variables but not memory */
systemSoftReset:
  clearState();
  goto systemInitDevices;

// ===========================================================================
//                              ENTRY POINT
// ===========================================================================
start:
  goto systemHardReset;
systemInitDevices:
  openStdtrc
  openStdblk
  openStdexp
  openStdimp
  setIOdefaults 

// ---------------------------------------------------------------------------
// Uncomment systemCopyProgram to run the example program hardcoded above
// (see "EXAMPLE OF INDIRECT THREADED PROGRAM" section above).
// This is an (unusual) alternative to systemLoadProgram below.
// You must of course also comment out systemLoadProgram.
/*
systemCopyProgram: ;   // Copy program into system memory
    int i = 0;
    for( i; i<PROGRAM_SIZE; i++) {
      memoryWords[i] = itProg[i];
    };
*/
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Uncomment systemLoadProgram to load program from ROM file (as usual).
// This is an alternative to (the much more unusual) systemCopyProgram above.
// You must of course also comment out systemCopyProgram.
systemLoadProgram:
    openRom
    if (fread(&memory,WORD_SIZE,ROM_SIZE_WDS,romHandle) == 0) {
      goto trapCantReadRom; // ROM read failed for some reason
    }
    closeRom
// ---------------------------------------------------------------------------

systemInitCore:
    dsp = DS_EMPTY;                 // Initialize data stack pointer
    rsp = RS_EMPTY;                 // Initialize return stack pointer
    ssp = SS_EMPTY;                 // Initialize software stack pointer
    pc = 0;                         // Set program counter to start of program

  nextInstruction: // Begin Freeputer program execution

    #ifdef TRON_ENABLED
      optTrace
    #endif

    rA = *(WORD *)&memory[pc];
    incPc

    if ((0b1111111111111111111111100000000 & rA) != 0) {
      goto trapIllegalOpcode;
    }

    // This large switch statement is portable but might be very slow
    // if your C compiler does not compile it into a jump table.
    // Be sure to compile with maximum optimization for speed!
    switch(rA) {
      case iTROFF:
        vmFlags = vmFlags & 0b11111111111111111111111111111110;
        break;
      case iTRON:
        vmFlags = vmFlags | 0b00000000000000000000000000000001;
        break;
      case iWCHAN:
        popDs
        wchannel = rA;
        break;
      case iRCHAN:
        popDs
        rchannel = rA;
        break;
      case iGCHAN:
        popDs
        gchannel = rA;
        break;
      case iPCHAN:
        popDs
        pchannel = rA;
        break;
      case iHOMIO:
        setIOdefaults
        break;
      case iECODE:
        rA = lastExitCode;
        pushDs
        break;
      case iRCODE:
        rA = lastRestartCode;
        pushDs
        break;
      case iROM:
        rA = ROM_SIZE;
        pushDs
        break;
      case iRAM:
        rA = RAM_SIZE;
        pushDs
        break;
      case iMAP:
        rA = MAP_SIZE;
        pushDs
        break;
      case iSTDBLK:
        rA = STDBLK_SIZE;
        pushDs
        break;
      case iDS:
        rA = MAX_DEPTH_DS;
        pushDs
        break;
      case iSS:
        rA = MAX_DEPTH_SS;
        pushDs
        break;
      case iRS:
        rA = MAX_DEPTH_RS;
        pushDs
        break;
      case iDSN:
        if (dsp < DS_EMPTY ) {
          // data stack is not empty
          rA = DS_EMPTY - dsp;
        } else {
          // data stack is empty
          rA = 0;
        }
        pushDs
        break;
      case iSSN:
        if (ssp < SS_EMPTY ) {
          // software stack is not empty
          rA = SS_EMPTY - ssp;
        } else {
          // software stack is empty
          rA = 0;
        }
        pushDs
        break;
      case iRSN:
        if (rsp < RS_EMPTY ) {
          // return stack is not empty
          rA = RS_EMPTY - rsp;
        } else {
          // return stack is empty
          rA = 0;
        }
        pushDs
        break;
      case iREV:   //  ( n1 n2 n3 -- n3 n2 n1 )
        if (dsp > DS_HAS_THREE) {
          goto trapDsUnderflow;
        }
        rA = dsp[0];    // n1
        rC = dsp[2];        // n3
        dsp[0] = rC;
        dsp[2] = rA;
        break;
      case iGETOR:
        switch(gchannel) {
          case STDBLK:
            getBuf = 0; // zero the buffer
            popDs
            if ((rA & STDBLK_MEMORY_MASK) != 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fseek(stdblkHandle,rA,SEEK_SET) !=0) {
              branch // seek failed
              break;
            }
            if (fread(&getBuf,WORD_SIZE,1,stdblkHandle) < 1) {
              branch // get failed
              break;
            }
            rA = getBuf;
            pushDs
            dontBranch
            break;
          default:
            branch // Unsupported gchannel
            break;
        }
        break;
      case iGETORB:
        switch(gchannel) {
          case STDBLK:
            getBuf = 0; // zero the buffer
            popDs
            if ((rA & STDBLK_MEMORY_MASK) != 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fseek(stdblkHandle,rA,SEEK_SET) !=0) {
              branch // seek failed
              break;
            }
            if (fread(&getBuf,1,1,stdblkHandle) < 1) {
              branch // get failed
              break;
            }
            rA = getBuf;
            pushDs
            dontBranch
            break;
          default:
            branch // Unsupported gchannel
            break;
        }
        break;
      case iPUTOR:
        switch(pchannel) {
          case STDBLK:
            twoPopDs     // Address in now in rB
            putBuf = rA; // Value to be put is now in putBuf
            if ((rB & STDBLK_MEMORY_MASK) != 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fseek(stdblkHandle,rB,SEEK_SET) !=0) {
              branch // seek failed
              break;
            }
            if (fwrite(&putBuf,WORD_SIZE,1,stdblkHandle) < 1) {
              branch // put failed
              break;
            }
            dontBranch
            break;
          default:
            branch // Unsupported pchannel
            break;
        }
        break;
      case iPUTORB:
        switch(pchannel) {
          case STDBLK:
            twoPopDs     // Address in now in rB
            putBuf = rA; // Value to be put is now in putBuf
            if ((rB & STDBLK_MEMORY_MASK) != 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fseek(stdblkHandle,rB,SEEK_SET) !=0) {
              branch // seek failed
              break;
            }
            if (fwrite(&putBuf,1,1,stdblkHandle) < 1) {
              branch // put failed
              break;
            }
            dontBranch
            break;
          default:
            branch // Unsupported pchannel
            break;
        }
        break;
      case iREADOR: ;
        readBuf = 0; // zero the buffer
        switch(rchannel) {
          case STDIN:
            if (fread(&readBuf,WORD_SIZE,1,stdin) < 1) {
              branch // read failed
              break;
            } else {
              rA = readBuf;
              pushDs
              dontBranch
              break;
            }
          break;
          case STDIMP:
            if (fread(&readBuf,WORD_SIZE,1,stdimpHandle) < 1) {
              branch // read failed
              break;
            } else {
              rA = readBuf;
              pushDs
              dontBranch
              break;
            }
          break;
          default:
            branch // Unsupported rchannel
            break;
        }
        break;
      case iREADORB: ;
        readBuf = 0; // zero the buffer
        switch(rchannel) {
          case STDIN:
            if (fread(&readBuf,1,1,stdin) < 1) {
              branch // read failed
              break;
            }
            rA = readBuf;
            pushDs
            dontBranch
            break;
          case STDIMP:
            if (fread(&readBuf,1,1,stdimpHandle) < 1) {
              branch // read failed
              break;
            }
            rA = readBuf;
            pushDs
            dontBranch
            break;
          default:
            branch // Unsupported rchannel
            break;
        }
        break;
      case iWRITOR:
        switch(wchannel) {
          case STDOUT:
            popDs
            writeBuf = rA;
            if (fwrite(&writeBuf,WORD_SIZE,1,stdout) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
            }
            break;
          case STDEXP:
            popDs
            writeBuf = rA;
            if (fwrite(&writeBuf,WORD_SIZE,1,stdexpHandle) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
            }
            break;
          default:
            branch // Unsupported wchannel
            break;
        }
        break;
      case iWRITORB:
        switch(wchannel) {
          case STDOUT:
            popDs
            writeBuf = rA;
            if (fwrite(&writeBuf,1,1,stdout) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
              break;
            }
          case STDEXP:
            popDs
            writeBuf = rA;
            if (fwrite(&writeBuf,1,1,stdexpHandle) < 1) {
              branch  // write failed
            } else {
              dontBranch
              break;
            }
          default:
            branch // Unsupported wchannel
            break;
        }
        break;
      case iTRACOR:
        popDs
        writeBuf = rA;
        if (fwrite(&writeBuf,WORD_SIZE,1,stdtrcHandle) < 1) {
          branch // trace failed
          break;
        } else {
          dontBranch
          break;
        }
        break;
      case iTRACORB:
        popDs
        writeBuf = rA;
        if (fwrite(&writeBuf,1,1,stdtrcHandle) < 1) {
          branch // trace failed
          break;
        } else {
          dontBranch
          break;
        }
        break;
      case iFIND:     //  ( numWords n src -- wordIndex ) find 1st instance of n from src
                      //      onwards but look at no more than numWords
        threePopDs    // rA = numWords, rB = n, rC = src
        ensureWordAddressable(rC)
        if (rA >= 0) { // FIXME this overflow protection is untested
          if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
            goto trapMemBounds;
          } 
        } else {
          if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
            goto trapMemBounds;
          } 
        }
        ensureWordAddressable(rC+(rA*WORD_SIZE))
        if (rA == 0 ) {
          // numWords is zero
          rA = -1; // -1 means not found
          pushDs;
          break;
        } else if (rA < 0 ) {
          // numWords is negative, do descending search
          int i = 0;
          int addr;
          int result = -1; // -1 means not found
          for (i; i>rA; i--) {
            addr = rC+(i*WORD_SIZE);
            if (rB == *(WORD *)&memory[addr]) {
              result = (0-i);
              break;
            }
          }
          rA = result;
          pushDs
          break;
        } else {
          // numWords is positive, do ascending search
          int i = 0;
          int addr;
          int result = -1; // -1 means not found
          for (i; i<rA; i++) {
            addr = rC+(i*WORD_SIZE);
            if (rB == *(WORD *)&memory[addr]) {
              result = i;
              break;
            }
          }
          rA = result;
          pushDs
          break;
        }
        break;
      case iFINDB:    //  ( numBytes b src -- byteIndex ) find 1st instance of b from src
                      //      onwards but look at no more than numBytes
        threePopDs    // rA = numBytes, rB = n, rC = src
        ensureByteAddressable(rC)
        if (rA >= 0) { // FIXME this overflow protection is untested
          if ((POS_INT_MAX-rC)<rA) {
          goto trapMemBounds;
          } 
        } else {
          if ((NEG_INT_MAX+rC)>rA) {
          goto trapMemBounds;
          } 
        }
        ensureByteAddressable(rC+rA)
        if (rA == 0 ) {
          // numBytes is zero
          rA = -1; // -1 means not found
          pushDs;
          break;
        } else if (rA < 0 ) {
          // numBytes is negative, do descending search
          int i = 0;
          int addr;
          int result = -1; // -1 means not found
          for (i; i>rA; i--) {
            addr = rC+i;
            if (memory[addr] == rB) {
              result = (0-i);
              break;
            }
          }
          rA = result;
          pushDs
          break;
        } else {
          // numBytes is positive, do ascending search
          int i = 0;
          int addr;
          int result = -1; // -1 means not found
          for (i; i<rA; i++) {
            addr = rC+i;
            if (memory[addr] == rB) {
              result = i;
              break;
            }
          }
          rA = result;
          pushDs
          break;
        }
        break;
      case iFILL:     //  ( numWords n dest -- ) fill numWords with n at dest onwards
        threePopDs    // rA = numWords, rB = n, rC = dest
        ensureWordAddrWritable(rC)
        if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) { // FIXME this overflow protection is untested
          goto trapMemBounds;
        }
        ensureWordAddrWritable(rC+(rA*WORD_SIZE))
        if (rA <= 0 ) {
          // numWords is zero or less, so do nothing
          break;
        } else {
            int i = 0;
            int addr;
            for (i; i<rA; i++) {
              addr = rC+(i*WORD_SIZE);
              *(WORD *)&memory[addr] = rB;
            }
            break;
        }
        break;
      case iFILLB:    //  ( numBytes b dest -- ) fill numBytes with b at dest onwards
        threePopDs    // rA = numBytes, rB = n, rC = dest
        ensureByteAddrWritable(rC)
        if ((POS_INT_MAX-rC)<rA) { // FIXME this overflow protection is untested
          goto trapMemBounds;
        }
        ensureByteAddrWritable(rC+rA)
        if (rA <= 0 ) {
          // numBytes is zero or less, so do nothing
          break;
        } else {
            int i = 0;
            int addr;
            for (i; i<rA; i++) {
              addr = rC+i;
              memory[addr] = rB;
            }
            break;
        }
        break;
      case iMATCH: ;  //  ( numWords src dest -- TRUE/FALSE ) see if strings match
        {
          threePopDs    // rA = numWords, rB = src, rC = dest    
          ensureWordAddressable(rB)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if (((POS_INT_MAX-rB)/ WORD_SIZE)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if (((NEG_INT_MAX+rB)/ WORD_SIZE)>rA) {
            goto trapMemBounds;
            } 
          }
          ensureWordAddressable(rB+(rA*WORD_SIZE))
          ensureWordAddressable(rC)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
            goto trapMemBounds;
            } 
          }
          ensureWordAddressable(rC+(rA*WORD_SIZE))
          int i = 0;
          int w1addr;
          int w2addr;
          int result = TRUE;
          for (i; i<rA; i=i++) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            if ((*(WORD *)&memory[w1addr]) != (*(WORD *)&memory[w2addr])) {
              result = FALSE;
              break; // mismatch
            }
          }
          rA = result;
          pushDs;
        }
        break;
      case iMATCHB: ; //   ( numBytes src dest -- TRUE/FALSE ) see if strings match
        threePopDs    // rA = numBytes, rB = src, rC = dest
        {      
          ensureByteAddressable(rB)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if ((POS_INT_MAX-rB)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if ((NEG_INT_MAX+rB)>rA) {
            goto trapMemBounds;
            } 
          }
          ensureByteAddressable(rB+rA)
          ensureByteAddressable(rC)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if ((POS_INT_MAX-rC)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if ((NEG_INT_MAX+rC)>rA) {
            goto trapMemBounds;
            } 
          }
          ensureByteAddressable(rC+rA)
          int i = 0;
          int b1addr;
          int b2addr;
          int result = TRUE;
          for (i; i<rA; i=i++) {
            b1addr = rB+i;
            b2addr = rC+i;
            if (memory[b1addr] != memory[b2addr]) {
              result = FALSE;
              break; // mismatch
            }
          }
          rA = result;
          pushDs;
        }
        break;
      case iMOVEB: ;  //   ( numBytes src dest -- ) copy byte at src addr to dest addr
        threePopDs    // rA = numBytes, rB = src, rC = dest
        ensureByteAddressable(rB)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if ((POS_INT_MAX-rB)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if ((NEG_INT_MAX+rB)>rA) {
            goto trapMemBounds;
            } 
          }
        ensureByteAddressable(rB+rA)
        ensureByteAddrWritable(rC)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if ((POS_INT_MAX-rC)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if ((NEG_INT_MAX+rC)>rA) {
            goto trapMemBounds;
            } 
          }
        ensureByteAddrWritable(rC+rA)
        if (rA == 0) {
          // Nothing to do, numBytes is 0
          break;
        } else if (rB == rC) {
          // Nothing to do, src and dest are the same
          break;
        } else if (rA > 0) {
          // numBytes is positive, do ascending move
          int i = 0;
          int b1addr;
          int b2addr;
          for (i; i<rA; i=i++) {
            b1addr = rB+i;
            b2addr = rC+i;
            memory[b2addr] = memory[b1addr];
          }
          break;
        } else {
          // numBytes is negative, do descending move
          int i = 0;
          int b1addr;
          int b2addr;
          for (i; i>rA; i=i--) {
            b1addr = rB+i;
            b2addr = rC+i;
            memory[b2addr] = memory[b1addr];
          }
        }
        break;
      case iMOVE: ;   //   ( numWords src dest -- ) copy word at src addr to dest addr
        threePopDs    // rA = numWords, rB = src, rC = dest
        ensureWordAddressable(rB)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if (((POS_INT_MAX-rB)/ WORD_SIZE)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if (((NEG_INT_MAX+rB)/ WORD_SIZE)>rA) {
            goto trapMemBounds;
            } 
          }
        ensureWordAddressable(rB+(rA*WORD_SIZE))
        ensureWordAddrWritable(rC)
          if (rA >= 0) { // FIXME this overflow protection is untested
            if (((POS_INT_MAX-rC)/ WORD_SIZE)<rA) {
            goto trapMemBounds;
            } 
          } else {
            if (((NEG_INT_MAX+rC)/ WORD_SIZE)>rA) {
            goto trapMemBounds;
            } 
          }
        ensureWordAddrWritable(rC+(rA*WORD_SIZE))
        if (rA == 0) {
          // Nothing to do, numWords is 0
          break;
        } else if (rB == rC) {
          // Nothing to do, src and dest are the same
          break;
        } else if (rA > 0) {
          // numWords is positive, do ascending move
          int i = 0;
          int w1addr;
          int w2addr;
          for (i; i<rA; i=i++) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            *(WORD *)&memory[w2addr] = *(WORD *)&memory[w1addr];
          }
          break;
        } else {
          // numWords is negative, do descending move
          int i = 0;
          int w1addr;
          int w2addr;
          for (i; i>rA; i=i--) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            *(WORD *)&memory[w2addr] = *(WORD *)&memory[w1addr];
          }
          break;
        }
        break;
      case iLOADB:    //  ( a -- byte )
        popDs
        ensureByteAddressable(rA)
        rA = memory[rA]; // Retrieve byte starting at specified byte
        pushDs
        break;
      case iRLOADB:   //  ( a -- a byte )
        peekDs
        ensureByteAddressable(rA)
        rA = memory[rA]; // Retrieve byte starting at specified byte
        pushDs
        break;
      case iLOAD:     //  ( a -- word )
        popDs
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve word starting at specified byte
        pushDs
        break;
      case iRLOAD:    //  ( a -- a word )
        peekDs
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve word starting at specified byte
        pushDs
        break;
      case iPLOAD: ;  //  ( p -- word ) Like load but assumes address is a pointer
                      //                and therefore loads word from address stored at p
        popDs
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve addr starting at specified byte
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA];
        pushDs
        break;
      case iRPLOAD:  //  ( p -- p word ) Like rload but assumes address is a pointer
        peekDs
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve addr starting at specified byte
        ensureWordAddressable(rA)
        rA = *(WORD *)&memory[rA];
        pushDs
        break;        break;
      case iPLOADB: ; //  ( p -- p word ) Like loadb but assumes address is a pointer
                      //                and therefore loads byte from address stored at p
        popDs
        ensureByteAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve addr starting at specified byte
        ensureByteAddressable(rA)
        rA = memory[rA];
        pushDs
        break;
      case iRPLOADB: ; // ( p -- p word ) Like rloadb but assumes address is a pointer
                       //               and therefore loads byte from address stored at p
        peekDs
        ensureByteAddressable(rA)
        rA = *(WORD *)&memory[rA]; // Retrieve addr starting at specified byte
        ensureByteAddressable(rA)
        rA = memory[rA];
        pushDs
        break;
      case iSTORE: ;  //  ( n a -- )
        popDs
        rB = rA;                  // Address is in rB
        popDs                     // Value to store is in rA
        ensureWordAddrWritable(rB)
        *(WORD *)&memory[rB] = rA;
        break;
      case iSTOREB: ; //  ( n a -- )
        popDs
        rB = rA;                  // Address is in rB
        popDs                     // Value to store is in rA
        ensureByteAddrWritable(rB)
        memory[rB] = rA;
        break;
      case iPSTORE: ; //  ( n p -- )      Like STORE but assumes p is a pointer
                      //                 and therefore stores word to address stored at p
        popDs
        rB = rA;                  // Pointer is in rB
        popDs                     // Value to store is in rA
        ensureWordAddressable(rB)
        rB = *(WORD *)&memory[rB];
        ensureWordAddrWritable(rB)
        *(WORD *)&memory[rB] = rA;
        break;
      case iPSTOREB: ; // ( n p -- )      Like STOREB but assumes p is a pointer
                       //                and therefore stores word to address stored at p
        popDs
        rB = rA;                  // Pointer is in rB
        popDs                     // Value to store is in rA
        ensureWordAddressable(rB)
        rB = *(WORD *)&memory[rB];
        ensureByteAddrWritable(rB)
        memory[rB] = rA;
        break;
      case iREBOOT:
        lastRestartCode = LAST_RESTART_CODE_REBOOT;
        goto systemHardReset;     // Program requested hard reset
        break;
      case iRESET:
        lastRestartCode = LAST_RESTART_CODE_RESET;
        goto systemReset;     // Program requested soft reset
        break;
      case iCALL: ;   //  (  -- )
        rA = pc;                  // rA now contains return address
        rA = rA + WORD_SIZE;
        pushRs                    // Return address is now on return stack
        branch
        break;
      case iDCALL: ;  //  ( a -- )
        popDs
        rB = rA;                  // rB now contains call address
        // Ensure dynamic call address lies within program memory
        if ((PROG_MEMORY_MASK & rB) != 0) {
          goto trapPcOverflow;
        }
        rA = pc;                  // rA now contains return address  
        pushRs                    // Return address is now on return stack
        pc = rB;                   // pc now contains call address
        break;
      case iRDCALL: ; //  ( a -- a ) ONLY SAFE FOR NON-CONSUMING FUNCTIONS!
        peekDs
        rB = rA;                  // rB now contains call address
        // Ensure dynamic call address lies within program memory
        if ((PROG_MEMORY_MASK & rB) != 0) {
          goto trapPcOverflow;
        }
        rA = pc;                  // rA now contains return address  
        pushRs                    // Return address is now on return stack
        pc = rB;                  // pc now contains call address
        break;
      case iEXIT:
        popRs
        // Ensure return address lies within program memory
        if ((PROG_MEMORY_MASK & rA) != 0) {
          goto trapPcOverflow;
        }
        pc = rA;
        break;
      case iSPOP:     //  ( -- n ) [ n -- ]
        popSs
        pushDs
        break;
      case iSPEEK:    //  ( -- n ) [ n -- n ]
        peekSs
        pushDs
        break;
      case iSPEEK2:   //  ( -- n1 n2 ) [ n1 n2 -- n1 n2 ] Note: NOT same as spop spop
        twoPeekSs
        rD = rA;
        rA = rB;
        rB = rD;
        twoPushDs
        break;
      case iSPEEK3:   //  ( -- n1 n2 n3 ) [ n1 n2 n3 -- n1 n2 n3 ] NOT same as spop spop spop
        threePeekSs
        rD = rC;
        rC = rA;
        rA = rD;
        threePushDs
        break;
      case iSPEEK4:   //  ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- n1 n2 n3 n4 ] NOT like spop x 4
        fourPeekSs
        {
          WORD buf = rA;
          rA = rD;
          rD = buf;
          buf = rB;
          rB = rC;
          rC = buf;
        }
        fourPushDs
        break;
      case iSPOP2:    //  ( -- n1 n2 ) [ n1 n2 -- ] Note: NOT same as spop spop
        twoPopSs
        rD = rA;
        rA = rB;
        rB = rD;
        twoPushDs
        break;
      case iSPOP3:    //  ( -- n1 n2 n3 ) [ n1 n2 n3 -- ] Note: NOT same as spop spop spop
        threePopSs
        rD = rC;
        rC = rA;
        rA = rD;
        threePushDs
        break;
      case iSPOP4:    //  ( -- n1 n2 n3 n4 ) [ n1 n2 n3 n4 -- ] Note: NOT like spop x 4
        fourPopSs
        {
          WORD buf = rA;
          rA = rD;
          rD = buf;
          buf = rB;
          rB = rC;
          rC = buf;
        }
        fourPushDs
        break;
      case iSPUSH:    //  ( n -- ) [ -- n ]
        popDs
        pushSs
        break;
      case iSPUSH2:   //  ( n1 n2 -- ) [ -- n1 n2 ] Note: NOT same as spush spush
        twoPopDs
        // Reverse rA, rB 'order'
        rD = rA;
        rA = rB;
        rB = rD;
        twoPushSs
        break;
      case iSPUSH3:   //  ( n1 n2 n3 -- ) [ -- n1 n2 n3 ] Note: NOT same as spush spush spush
        threePopDs
        // Reverse rA, rB, rC 'order'
        rD = rA;
        rA = rC;
        rC = rD;
        threePushSs
        break;
      case iSPUSH4:   //  ( n1 n2 n3 n4 -- ) [ -- n1 n2 n3 n4 ] Note: NOT like spush x 4
        fourPopDs
        {
          WORD buf = rA;
          rA = rD;
          rD = buf;
          buf = rB;
          rB = rC;
          rC = buf;
        }
        fourPushSs
        break;
      case iHOLD:     //  ( n -- n ) [ -- n ]
        peekDs
        pushSs
        break;
      case iHOLD2:    //  ( n1 n2 -- n1 n2 ) [ -- n1 n2 ]
        twoPeekDs
        twoPushSs
        break;
      case iHOLD3:    //  ( n1 n2 n3 -- n1 n2 n3 ) [ -- n1 n2 n3 ]
        threePeekDs
        threePushSs
        break;
      case iHOLD4:    //  ( n1 n2 n3 n4 -- n1 n2 n3 n4 ) [ -- n1 n2 n3 n4 ]
        fourPeekDs
        fourPushSs
        break;
      case iRPOP:     //  ( -- n ) { n -- }
        popRs
        pushDs
        break;
      case iRPUSH:    //  ( n -- ) { -- n }
        popDs
        pushRs
        break;
      case iBRGZ:     //  ( n -- n )
        peekDs
        if (rA <= 0) { dontBranch } else { branch };
        break;
      case iBRGEZ:    //  ( n -- n )
        peekDs
        if (rA < 0) { dontBranch } else { branch };
        break;
      case iBRLZ:     //  ( n -- n )
        peekDs
        if (rA >= 0) { dontBranch } else { branch };
        break;
      case iBRLEZ:    //  ( n -- n )
        peekDs
        if (rA > 0) { dontBranch } else { branch };
        break;
      case iBRNZ:     //  ( n -- n )
        peekDs
        if (rA == 0) { dontBranch } else { branch };
        break;
      case iBRZ:      //  ( n -- n )
        peekDs
        if (rA != 0) { dontBranch } else { branch };
        break;
      case iBRG:      //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB <= rA) { dontBranch } else { branch };
        break;
      case iBRGE:     //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB < rA) { dontBranch } else { branch };
        break;
      case iBRL:      //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB >= rA) { dontBranch } else { branch };
        break;
      case iBRLE:     //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB > rA) { dontBranch } else { branch };
        break;
      case iBRNE:     //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB == rA) { dontBranch } else { branch };
        break;
      case iBRE:      //  ( n1 n2 -- n1 )
        peekSecondDs
        popDs
        if (rB != rA) { dontBranch } else { branch };
        break;
      case iJGZ:      //  ( n -- )
        popDs
        if (rA <= 0) { dontBranch } else { branch };
        break;
      case iJGEZ:     //  ( n -- )
        popDs
        if (rA < 0) { dontBranch } else { branch };
        break;
      case iJLZ:      //  ( n -- )
        popDs
        if (rA >= 0) { dontBranch } else { branch };
        break;
      case iJLEZ:     //  ( n -- )
        popDs
        if (rA > 0) { dontBranch } else { branch };
        break;
      case iJZ:       //  ( n -- )
        popDs
        if (rA != 0) { dontBranch } else { branch };
        break;
      case iJNZ:      //  ( n -- )
        popDs
        if (rA == 0) { dontBranch } else { branch };
        break;
      case iJG:       //  ( n1 n2 -- )
        twoPopDs
        if (rA <= rB) { dontBranch } else { branch };
        break;
      case iJGE:      //  ( n1 n2 -- )
        twoPopDs
        if (rA < rB) { dontBranch } else { branch };
        break;
      case iJL:       //  ( n1 n2 -- )
        twoPopDs
        if (rA >= rB) { dontBranch } else { branch };
        break;
      case iJLE:      //  ( n1 n2 -- )
        twoPopDs
        if (rA > rB) { dontBranch } else { branch };
        break;
      case iJE:       //  ( n1 n2 -- )
        twoPopDs
        if (rA != rB) { dontBranch } else { branch };
        break;
      case iJNE:      //  ( n1 n2 -- )
        twoPopDs
        if (rA == rB) { dontBranch } else { branch };
        break;
      case iJMP:      //  ( -- )
        branch
        break;
      case iDJMP:     //  ( a -- )
        popDs                     // rA now contains jump address             
        // Ensure dynamic jump address lies within program memory
        if ((PROG_MEMORY_MASK & rA) != 0) {
          goto trapPcOverflow;
        }
        pc = rA;
        break;
      case iDEC: ;    //  ( n -- n-1 )
        peekDs
        if (rA == NEG_INT_MAX) {
          goto trapMathOverflow;
        }
        rA--;
        replaceDs
        break;
      case iDECW:     //  ( n -- n-WORD_SIZE )
        peekDs
        if (rA < NEG_INT_MAX + WORD_SIZE ) {
          goto trapMathOverflow;
        }
        rA = rA - WORD_SIZE;
        replaceDs
        break;
      case iDEC2W:    //  ( n -- n-TWO_WORDS_SIZE )
        peekDs
        if (rA < NEG_INT_MAX + TWO_WORDS_SIZE ) {
          goto trapMathOverflow;
        }
        rA = rA - TWO_WORDS_SIZE;
        replaceDs
        break;
      case iINC:      //  ( n -- n+1 )
        peekDs
        if (rA == POS_INT_MAX) {
          goto trapMathOverflow;
        }
        rA++;
        replaceDs
        break;
      case iINCW:     //  ( n -- n+WORD_SIZE )
        peekDs
        if (rA > POS_INT_MAX - WORD_SIZE ) {
          goto trapMathOverflow;
        }
        rA = rA + WORD_SIZE;
        replaceDs
        break;
      case iINC2W:    //  ( n -- n+TWO_WORDS_SIZE )
        peekDs
        if (rA > POS_INT_MAX - TWO_WORDS_SIZE ) {
          goto trapMathOverflow;
        }
        rA = rA + TWO_WORDS_SIZE;
        replaceDs
        break;
      case iDROP:     //  ( n -- )
        dropDs
        break;
      case iDROP2:    //  ( n1 n2 -- )
        twoDropDs
        break;
      case iDROP3:    //  ( n1 n2 n3 -- )
        threeDropDs
        break;
      case iDROP4:    //  ( n1 n2 n3 n4 -- )
        fourDropDs
        break;
      case iDUP:      //  ( n -- n n )
        peekDs
        pushDs
        break;
      case iDUP2:     //  ( n1 n2 -- n1 n2 n1 n2 )
        twoPeekDs
        twoPushDs
        break;
      case iDUP3:     //  ( n1 n2 n3 -- n1 n2 n3 n1 n2 n3 )
        threePeekDs
        threePushDs
        break;
      case iDUP4:     //  ( n1 n2 n3 n4 -- n1 n2 n3 n4 n1 n2 n3 n4 )
        fourPeekDs
        fourPushDs
        break;
      case iSWAP:     //  ( n1 n2 -- n2 n1 )
        twoPeekDs
        rC = rA;
        rA = rB;
        rB = rC;
        twoReplaceDs
        break;
      case iOVER:     //  ( n1 n2 -- n1 n2 n1 )
        peekSecondDs
        rA = rB;
        pushDs
        break;
      case iNIP:      //  ( n1 n2 -- n2 )
        if (dsp > DS_HAS_TWO) {
          goto trapDsUnderflow;
        }
        peekDs
        dsp++;
        *(dsp) = rA;
        break;
      case iTUCK:     //  ( n1 n2 -- n2 n1 n2 )
        twoPeekDs
        rC = rA;
        rA = rB;
        rB = rC;
        twoReplaceDs
        peekSecondDs
        rA = rB;
        pushDs
        break;
      case iROT:      //  ( n1 n2 n3 -- n2 n3 n1 )
        threePeekDs
        threeReplaceDs
        break;
      case iLEAP:     //  ( n1 n2 n3 -- n1 n2 n3 n1 )
        peekThirdDs
        rA = rC;
        pushDs
        break;
      case iTOR:      //  ( n1 n2 n3 -- n3 n1 n2 )
        threePeekDs
        threeRReplaceDs
        break;
      case iHALT:     //  ( -- )
        goto exitSuccess;
        break;
      case iWALL:     //  ( -- )
        goto trapWall;
        break;
      case iDATA:     //  ( -- )
        goto trapData;
        break;
      case iLIT:      //  ( -- n )
        wordAtPc
        pushDs
        incPc
        break;
      case iAND:      //  ( n1 n2 -- n1&n2 )
        twoPopDs
        rA = rA & rB;
        pushDs
        break;
      case iRAND:     //  ( n1 n2 -- n1 n2 n2&n1 )
        twoPeekDs
        rA = rA & rB;
        pushDs
        break;
      case iOR:       //  ( n1 n2 -- n1|n2 )
        twoPopDs
        rA = rA | rB;
        pushDs
        break;
      case iROR:      //  ( n1 n2 -- n1 n2 n2|n1 )
        twoPeekDs
        rA = rA | rB;
        pushDs
        break;
      case iXOR: ;    //  ( n1 n2 -- n1^n2 )
        twoPopDs
        rA = rA ^ rB;
        pushDs
        break;
      case iRXOR:     //  ( n1 n2 -- n1 n2 n2^n1 )
        twoPeekDs
        rA = rA ^ rB;
        pushDs
        break;
      case iADD:      //  ( n1 n2 -- n1+n2 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        if ((rA > 0) == (rB > 0)) {
          // Overflow is possible (both same sign)
          if (rA > 0) {
            // Both positive and > 0
            if ((POS_INT_MAX - rA) < rB) {
              goto trapMathOverflow;
            }
          } else {
            // Both negative or 0
            if ((NEG_INT_MAX - rA) > rB) {
              goto trapMathOverflow;
            }
          }
        }
        rA = rA + rB;
        pushDs
        break;
      case iRADD:     //  ( n1 n2 -- n1 n2+n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        if ((rA > 0) == (rB > 0)) {
          // Overflow is possible (both same sign)
          if (rA > 0) {
            // Both positive and > 0
            if ((POS_INT_MAX - rA) < rB) {
              goto trapMathOverflow;
            }
          } else {
            // Both negative or 0
            if ((NEG_INT_MAX - rA) > rB) {
              goto trapMathOverflow;
            }
          }
        }
        rA = rA + rB;
        twoReplaceDs
        break;
      case iDIV:      //  ( n1 n2 -- n1/n2 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rA = rA / rB;
        pushDs
        break;
      case iRDIV:     //  ( n1 n2 -- n1 n2/n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rA = rA / rB;
        twoReplaceDs
        break;
      case iMOD:      //  ( n1 n2 -- n1%n2 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rA = rA % rB;
        pushDs
        break;
      case iRMOD:     //  ( n1 n2 -- n1 n2%n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rA = rA % rB;
        twoReplaceDs
        break;
      case iDIVMOD:   //  ( n1 n2 -- n1/n2 n1%n2 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rC = rA; // Preserve original rA
        rA = rA / rB;
        pushDs
        rA = rC; // Restore original rA
        rA = rA % rB;
        pushDs
        break;
      case iMUL:      //  ( n1 n2 -- n1*n2 ) ---------------------------------
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        if (rA == 0) {
          pushDs
          break;
        } else if (rB == 0 ) {
            rA = 0;
            pushDs
            break;
        } else if (rA == 1 ) {
            rA = rB;
            pushDs
            break;
        } else if (rB == 1) {
            pushDs
            break;          
        } else if (rA > 0) { // At this point neither rA nor rB are 0 or 1
            if (rB > 0 ) {
              // Both >= 2
              if ((POS_INT_MAX/rB) < rA) {
                goto trapMathOverflow;
              } else {
                rA = rA * rB;
                pushDs
                break;
              }
            } else {
              // rA >= 2, rB negative
              if ((0-(POS_INT_MAX/rB)) < rA) {
                goto trapMathOverflow;
              } else {
                rA = rA * rB;
                pushDs
                break;
              }
            }
        } else if (rB < 0) {
          // Both negative
          if ((NEG_INT_MAX == rA) || (NEG_INT_MAX == rB)) {
            goto trapMathOverflow;
          } else if ((POS_INT_MAX/rB) > rA) {
            goto trapMathOverflow;
          } else {
            rA = rA * rB;
            pushDs
            break;
          }
        } else {
          // rB >=2, rA negative
          if ((0-(POS_INT_MAX/rA)) < rB) {
            goto trapMathOverflow;
          } else {
            rA = rA * rB;
            pushDs
            break;
          }
        } 
        break;
        // -------------------------------------------------------------------
      case iRMUL:     //  ( n1 n2 -- n1 n2*n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        if (rA == 0) {
          twoReplaceDs
          break;
        } else if (rB == 0 ) {
            rA = 0;
            twoReplaceDs
            break;
        } else if (rA == 1 ) {
            rA = rB;
            twoReplaceDs
            break;
        } else if (rB == 1) {
            twoReplaceDs
            break;          
        } else if (rA > 0) { // At this point neither rA nor rB are 0 or 1
            if (rB > 0 ) {
              // Both >= 2
              if ((POS_INT_MAX/rB) < rA) {
                goto trapMathOverflow;
              } else {
                rA = rA * rB;
                twoReplaceDs
                break;
              }
            } else {
              // rA >= 2, rB negative
              if ((0-(POS_INT_MAX/rB)) < rA) {
                goto trapMathOverflow;
              } else {
                rA = rA * rB;
                twoReplaceDs
                break;
              }
            }
        } else if (rB < 0) {
          // Both negative
          if ((NEG_INT_MAX == rA) || (NEG_INT_MAX == rB)) {
            goto trapMathOverflow;
          } else if ((POS_INT_MAX/rB) > rA) {
            goto trapMathOverflow;
          } else {
            rA = rA * rB;
            twoReplaceDs
            break;
          }
        } else {
          // rB >=2, rA negative
          if ((0-(POS_INT_MAX/rA)) < rB) {
            goto trapMathOverflow;
          } else {
            rA = rA * rB;
            twoReplaceDs
            break;
          }
        } 
        break;
        // -------------------------------------------------------------------
      case iSUB:      //  ( n1 n2 -- n1-n2 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPopDs
        if ((rA > 0) != (rB > 0)) {
          // Overflow is possible (signs are opposite)
          if (rA > 0) {
            // rA positive, rB <=0
            if (((POS_INT_MAX - rA) + rB) < 0) {
              goto trapMathOverflow;
            }
          } else {
            // rB positive, rA <=0
            if ((rA - NEG_INT_MAX) < rB) {
              goto trapMathOverflow;
            }
          }
        }
        rA = rA - rB;
        pushDs
        break;
      case iRSUB:     //  ( n1 n2 -- n1 n2-n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        if ((rA > 0) != (rB > 0)) {
          // Overflow is possible (signs are opposite)
          if (rA > 0) {
            // rA positive, rB <=0
            if (((POS_INT_MAX - rA) + rB) < 0) {
              goto trapMathOverflow;
            }
          } else {
            // rB positive, rA <=0
            if ((rA - NEG_INT_MAX) < rB) {
              goto trapMathOverflow;
            }
          }
        }
        rA = rA - rB;
        twoReplaceDs
        break;
      case iSHL:      //  ( n1 n2 -- n1<<n2 )
          twoPopDs
          if (rB > 0x1f ) {
            goto trapXsBitshift;
          }
          rA = rA<<rB;
          pushDs
        break;
      case iRSHL:     //  ( n1 n2 -- n1 n2 n1<<n2 )
        twoPeekDs
        if (rA > 0x1f ) {
          goto trapXsBitshift;
        }
        rB = rB<<rA;
        rA = rB;
        pushDs
        break;
      case iSHR:      //  ( n1 n2 -- n1>>n2 )
        {
          twoPopDs
          if (rB > 0x1f ) {
            goto trapXsBitshift;
          }
          WORD_AS_BITS rAbits = (WORD_AS_BITS) rA;
          rA = rAbits>>rB;
          pushDs
        }
        break;
      case iRSHR:     //  ( n1 n2 -- n1 n2 n1>>n2 )
        {
          twoPeekDs
          if (rA > 0x1f ) {
            goto trapXsBitshift;
          }
          WORD_AS_BITS rBbits = (WORD_AS_BITS) rB;
          rB = rBbits>>rA;
          rA = rB;
          pushDs
        }
        break;
      case iNEG:      //  ( n1 -- n1*-1 )
        popDs
        // Do not allow negation of NEG_INT_MAX
        if (rA == NEG_INT_MAX) {
          goto trapMathOverflow;
        }
        rA = rA * -1;
        pushDs
        break;
      case iABS:      //  ( n1 -- |n1| )
        peekDs
        if ( rA < 0 ) {
          // Do not allow negation of NEG_INT_MAX
          if (rA == NEG_INT_MAX) {
            goto trapMathOverflow;
          }
          rA = rA * -1;
          replaceDs
        }
        break;
      case iRDIVMOD:   //  ( n1 n2 -- n1 n2/n1 n2%n1 )
      //   FIXME Needs further testing and further optimization.
      // WARNING: This arithmetic operation has not been comprehensively
      // tested and therefore might possibly be incorrect for some
      // inputs. However, it has passed the fvmtest 1.0.0.0 suite.
      // NOTE: For decent performance this should be replaced by
      // inline assembly language for your target platform!
        twoPeekDs
        // Do not allow division by zero
        if (rB == 0) {
          goto trapDivideByZero;
        }
        // Do not allow division of NEG_INT_MAX by -1
        if ((rA == NEG_INT_MAX) && (rB == -1)) {
          goto trapMathOverflow;
        }
        rC = rA; // Preserve original rA
        rA = rA / rB;
        replaceDs
        rA = rC; // Restore original rA
        rA = rA % rB;
        pushDs
        break;
      default:
        goto trapIllegalOpcode;
        break;
    } 
    goto nextInstruction;

// ===========================================================================
//                              SYSTEM RESET
// ===========================================================================
systemReset:
  lastExitCode = rB;          // Save lastExitCode (passed in here in rB)
if (stdblkHandle) {
  closeStdblk                 // Close the standard block device
}
if (stdexpHandle) {
  closeStdexp                 // Close stdexp
}
if (stdimpHandle) {
  closeStdimp                 // Close stdimp
}
if (stdtrcHandle) {
  closeStdtrc                 // Close stdtrc
}
  // The next line should normally be the uncommented one for most FVMs
     goto exitFail;           // Uncomment for exit with specific failure code
  // goto exitFailGeneric;    // Uncomment for exit with generic failure code
  // The next line should be uncommented for fvm16-16MB-sr-append for fvmtest
  // goto systemSoftReset;    // Uncomment for soft reset
  // goto systemHardReset;    // Uncomment for hard reset

// ===========================================================================
//                              EXIT POINTS
// ===========================================================================
exitSuccess:
  rB = 0;                     // exitCode for success
  goto systemExit;
exitFail:
  if (rB != 0 ) {
    goto systemExit;          // use exit code for specific failure (if any)
  }
  exitFailGeneric:
    rB = 1;                   // exitCode for generic failure
    goto systemExit;
systemExit:                   // Exit using exitCode in rB
  exit(rB);

// ===========================================================================
//                              EXIT TRACING
// ===========================================================================
// Send an error message to stdtrc
// along with information regarding current program state.
#ifdef TRON_ENABLED
  #define traceExit(msg) \
    fprintf(stdtrcHandle, "%s\n", msg); \
    fprintf(stdtrcHandle, "%s\n", msgBefore); \
    traceInfo \
    traceStacks
  #define traceExitMsg(msg) \
    fprintf(stdtrcHandle, "%s\n", msg); \
    fprintf(stdtrcHandle, "PC %08x \n", pc );
#else
  #define traceExit(msg) \
    fprintf(stdtrcHandle, "%s\n", msg);
  #define traceExitMsg(msg) \
    fprintf(stdtrcHandle, "%s\n", msg);
#endif // .ifdef TRON_ENABLED

// ===========================================================================
//                                TRAPS
// ===========================================================================
//
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL PROGRAM FLOW
//----------------------------------------------------------------------------
trapWall:   
  rB = 1;
  traceExit(msgTrapWall)
  goto systemReset;
trapData:
  rB = 2;
  traceExit(msgTrapData)
  goto systemReset;
trapPcOverflow:
  rB = 3;
  traceExitMsg(msgTrapPcOverflow)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL OPCODES
//----------------------------------------------------------------------------
iNONE:
trapIllegalOpcode:
  rB = 11;
  traceExit(msgTrapIllegalOpcode)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL MATHEMATICAL OPERATIONS
//----------------------------------------------------------------------------
trapMathOverflow:
  rB = 21;
  traceExit(msgTrapMathOverflow)
  goto systemReset;
trapDivideByZero:
  rB = 22;
  traceExit(msgTrapDivideByZero)
  goto systemReset;
trapXsBitshift:
  rB = 23;
traceExit(msgTrapXsBitshift)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL STACK OPERATIONS
//----------------------------------------------------------------------------
trapDsUnderflow:
  rB = 31;
  traceExit(msgTrapDsUnderflow)
  goto systemReset;
trapDsOverflow:
  rB = 32;
  traceExit(msgTrapDsOverflow)
  goto systemReset;
trapRsUnderflow:
  rB = 33;
  traceExit(msgTrapRsUnderflow)
  goto systemReset;
trapRsOverflow:
  rB = 34;
  traceExit(msgTrapRsOverflow)
  goto systemReset;
trapSsUnderflow:
  rB = 35;
  traceExit(msgTrapSsUnderflow)
  goto systemReset;
trapSsOverflow:
  rB = 36;
  traceExit(msgTrapSsOverflow)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL MEMORY ACCESS
//----------------------------------------------------------------------------
trapMemBounds:
  rB = 41;
  traceExitMsg(msgTrapMemBounds)
  goto systemReset;
trapRAMBounds:
  rB = 42;
  traceExitMsg(msgTrapRAMBounds)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ROM
//----------------------------------------------------------------------------
//Note: a ROM file ('rom.fp') can be created using a Freelang compiler
trapCantOpenRom:
  rB = 51;
  traceExitMsg(msgTrapCantOpenRom)
  goto systemReset;
trapCantCloseRom:
  rB = 52;
  traceExitMsg(msgTrapCantCloseRom)
  goto systemReset;
trapCantReadRom:
  rB = 53;
  traceExitMsg(msgTrapCantReadRom)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: STDBLK
//----------------------------------------------------------------------------
//Note: a suitable zero-filled stdblk file ('std.blk') can be created on Linux
//by the following command (assuming STDBLK_SIZE is 16777216 bytes):
//           head -c 16777216 /dev/zero > std.blk
//Note: to create a 'std.blk' file of 0 size on Linux simply use:
//           touch std.blk
trapCantOpenStdblk:
  rB = 61;
  traceExitMsg(msgTrapCantOpenStdblk)
  goto systemReset;
trapCantCloseStdblk:
  rB = 62;
  traceExitMsg(msgTrapCantCloseStdblk)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: STREAMS
//----------------------------------------------------------------------------
//Note: this FVM will automatically create (or recreate) a 'std.trc' file
//as it starts up; any previous data in that file will be lost
trapCantOpenStdtrc:
  rB = 71;
  goto exitFail;
trapCantCloseStdtrc:
  rB = 72;
  goto exitFail;
trapCantWriteToStdtrc:
  rB = 73;
  goto exitFail;

//Note: this FVM will automatically create (or recreate) a 'std.exp' file
//as it starts up; any previous data in that file will be lost
trapCantOpenStdexp:
  rB = 74;
  traceExitMsg(msgTrapCantOpenStdexp)
  goto systemReset;
trapCantCloseStdexp:
  rB = 75;
  traceExitMsg(msgTrapCantCloseStdexp)
  goto systemReset;

//Note: to create a 'std.imp' file of 0 size on Linux simply use:
//           touch std.imp
trapCantOpenStdimp:
  rB = 77;
  traceExitMsg(msgTrapCantOpenStdimp)
  goto systemReset;
trapCantCloseStdimp:
  rB = 78;
  traceExitMsg(msgTrapCantCloseStdimp)
  goto systemReset;
// ===========================================================================

} // end of runfvm()

/* Zero-fill all variables holding FVM state
     except system memory and lastExitCode */
void clearState() {

  rchannel = 0;
  wchannel = 0;
  gchannel = 0;
  pchannel = 0;
  readBuf = 0;
  writeBuf = 0;
  getBuf = 0;
  putBuf = 0;
  stdblkHandle = NULL;
  romHandle = NULL;
  stdtrcHandle = NULL;
  stdexpHandle = NULL;
  stdimpHandle = NULL;

  pcTmp = 0;

  rsp = NULL;
  memset(rs, 0, WORD_SIZE + (MAX_DEPTH_RS * WORD_SIZE));

  ssp = NULL;
  memset(ss, 0, WORD_SIZE + (MAX_DEPTH_SS * WORD_SIZE));  
  ss[MAX_DEPTH_SS * WORD_SIZE];

  dsp = NULL;
  memset(ds, 0, WORD_SIZE + (MAX_DEPTH_DS * WORD_SIZE));
}

/* Zero-fill all FVM system memory */
void clearMem() {
  memset(memory, 0, ROM_SIZE + RAM_SIZE + MAP_SIZE);  
}

int main() {
  runfvm();
  return 0;
}
// ===========================================================================

