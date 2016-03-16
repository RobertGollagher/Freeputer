/* 

                        FREEPUTER VIRTUAL MACHINE

Program:    fvm.c
Copyright © Robert Gollagher 2015, 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150822
Updated:    20160315:2346
Version:    pre-alpha-0.0.0.3 for FVM 1.1

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
                             for Linux and gcc
                          or for the Arduino IDE

                               ( ) [ ] { }

      This is the first, early, pre-alpha implementation of FVM 1.1.

==============================================================================

WARNING: This is pre-alpha software and as such may well be incomplete,
unstable and unreliable. It is considered to be suitable only for
experimentation and nothing more.

==============================================================================

PREREQUISITE KNOWLEDGE:
=======================

Please ensure you have read and understood the entire Quick Start
tutorial for FVM 1.0 in archive/1.0/README.md before attempting to use this
FVM 1.1 implementation, otherwise you will not understand the fundamental
concepts necessary to correctly build and use this FVM implementation.

==============================================================================

When using the FVMO_INCORPORATE_ROM option to incorporate a compiled
Freeputer program via 'rom.h' into the FVM executable, the preparation is
either to use the provided 'romMake.sh' script or the following 3 steps:

    1. Compile your Freelang program 'source.fl' to binary 'rom.fp':
          ./flx.rb source.fl rom.fp
    2. Convert binary 'rom.fp' into C header 'rom.h':
          xxd -i rom.fp > rom.h
    3. Change the declarations in 'rom.h' to read:
          const unsigned char prog[] PROGMEM = {
          int prog_size = 

Then run make (or compile using the Arduino IDE) as usual to build
the FVM executable itself. If you are not using the FVMO_INCORPORATE_ROM
option then you can skip the above preparation.

For now, since this is a pre-alpha implementation and thus performance is
a secondary consideration, the recommended command to build the FVM executable
with the provided Makefile is simply:

  make

Which is equivalent to:

  gcc -o fvm fvm.c

If your target is Arduino or chipKIT then of course you will need to use
the Arduino IDE to build the FVM executable rather than running make. To do
so, rename this 'fvm.c' file to 'fvm.ino' and open it with the Arduino IDE.
Alternatively, use appropriate symbolic links for convenience.

==============================================================================

  For reference: FVM 1.1 SIMPLE MULTIPLEXING

  FVM 1.1 may optionally use multiplexing to allow multiple virtual devices
  to share a single physical serial communication link. The simple multiplexing
  is that each data packet is preceded by a routing byte in which the
  most significant bit (bit 7) is always zero, the next two
  most significant bits (bits 6 and 5) identify the device type and
  the next bit (bit 4) identifies the packet size (essentially byte or word) and
  the four least significant bits (bit 3 to 0) identify the absolute value
  of the device ID (e.g. an ID of -1 is communicated as an ID of 1).
  Note that, by Freeputer convention, a device ID of 0 is only used for stdtrc
  and stdblk; an input stream never has a device ID of 0; and output streams
  have negative IDs. The crude multiplexing scheme used here only supports
  device IDs whose absolute value is between 0 and 15 inclusive.

  Most significant bits:

    0b0100 = device: output stream, packet size: 1 byte (byte)
    0b0101 = device: output stream, packet size: 4 bytes (word)
    0b0000 = device: input stream, packet size: 1 byte (byte)
    0b0001 = device: input stream, packet size: 4 bytes (word)
    0b0010 = device: block device, packet size: 5 bytes (word addr then byte)
    0b0011 = device: block device, packet size: 8 bytes (word addr then word)
    0b0110 = reserved (not used)
    0b0111 = reserved (not used) e.g. 0b01111111 unknown device
    0b1--- = reserved (not used)

  Thus byte communication uses:

    0b01000000 = stdtrc 0
    0b00000001 = stdin 1
    0b00000002 = stdimp 2
    0b00100000 = stdblk 0
    0b01000001 = stdout -1
    0b01000010 = stdexp -2
    0b0110----- = reserved (not used)

  Thus word communication uses:

    0b01010000 = stdtrc 0
    0b00010001 = stdin 1
    0b00010002 = stdimp 2
    0b00110000 = stdblk 0
    0b01010001 = stdout -1
    0b01010010 = stdexp -2
    0b0111----- = reserved (not used)

==============================================================================
                                KEY TO OPTIONS
==============================================================================

  SUPPORTED PLATFORMS
  ===================
  #define FVMP FVMP_STDIO // gcc using <stdio.h> for FILEs (eg Linux targets)
  #define FVMP FVMP_ARDUINO_IDE // Arduino IDE (eg Arduino or chipKIT targets)

  CONFIGURATION OPTIONS
  =====================
  #define FVMO_TRON // Enable tracing (degrades performance)
  #define FVMO_MULTIPLEX // Use multiplexing (see 'tape/tape.c')
  #define FVMO_SEPARATE_ROM // Use real (or at least separate) ROM memory
  #define FVMO_INCORPORATE_ROM // Incorporate 'rom.h' program in FVM executable
  #define FVMO_SAFE_ALIGNMENT // Target requires safely aligned memory access
  #define FVMO_NO_PROGMEM // Target does not understand the PROGMEM keyword
  #define FVMO_SERIALUSB // Target uses SerialUSB (Arduino Due native port)
  #define FVMO_SD // Target has SD card and uses Arduino SD library

  SIZING OPTIONS
  ==============
  
  You must define ROM_SIZE, RAM_SIZE and STDBLK_SIZE in bytes and these
  must be multiples of WORD size. Note that these are virtual sizes of the
  FVM instance, much smaller than the physical sizes of the target platform.
  Maximum address space (ROM_SIZE + RAM_SIZE) is 2147483648 bytes.
  Maximum STDBLK_SIZE is 2147483648 bytes.

// =========================================================================*/

// ===========================================================================
//                     SPECIFY FVM CONFIGURATION HERE:
// ===========================================================================
#define FVMC_LINUX_MINI_MUX

// ===========================================================================
//                SOME EXAMPLE CONFIGURATIONS TO CHOOSE FROM:
// ===========================================================================

/* A mini Linux FVM to use with the tape */
#ifdef FVMC_LINUX_MINI_MUX
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

/* A mini Arduino FVM to use with the tape.
   Suitable for Arduino Mega 2560 */
#ifdef FVMC_ARDUINO_MINI_MUX
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

/* A mini Arduino FVM to use with the tape.
   Suitable for chipKIT Max32 */
#ifdef FVMC_CHIPKIT_MINI_MUX
  #define FVMOS_CHIPKIT
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

// ===========================================================================
//                            SUPPORTED PLATFORMS:
// ===========================================================================
  #define FVMP_STDIO 0 // gcc using <stdio.h> for FILEs (eg Linux targets)
  #define FVMP_ARDUINO_IDE 1 // Arduino IDE (eg Arduino or chipKIT targets)

// ===========================================================================
//        SOME GENERIC OPTION SETS USED BY THE EXAMPLE CONFIGURATIONS:
// ===========================================================================
/* Generic option set: typical Linux options */
#ifdef FVMOS_LINUX
  #define FVMP FVMP_STDIO
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_ROM
  #define FVMO_NO_PROGMEM
#endif

/* Generic option set: typical Arduino options */
#ifdef FVMOS_ARDUINO
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_ROM
  #define FVMO_SAFE_ALIGNMENT
#endif

/* Generic option set: typical chipKIT options */
#ifdef FVMOS_CHIPKIT
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_ROM
  #define FVMO_SAFE_ALIGNMENT
  #define FVMO_NO_PROGMEM
#endif

/* Sizing: mini */
#ifdef FVMOS_SIZE_MINI
  #define ROM_SIZE 32768 
  #define RAM_SIZE 4096
  #define STDBLK_SIZE 0
#endif

// ===========================================================================
//                             CONSTANTS
// ===========================================================================
// The following do not change
#include <inttypes.h>
#define WORD int32_t
#define WORD_AS_BITS uint32_t
#define BYTE uint8_t
#define BYTE_MAX 255
#define NEG_INT_MAX (-2147483647 - 1) // C compiler quirk = -2147483648
#define POS_INT_MAX 2147483647
#define NONE NEG_INT_MAX
#define HALF_WORD_SIZE 2               // bytes
#define WORD_SIZE 4                    // bytes
#define TWO_WORDS_SIZE 8               // bytes
#define THREE_WORDS_SIZE 12            // bytes
#define FOUR_WORDS_SIZE 16             // bytes
#define MAX_DEPTH_DS 32                // elements (words) (power of 2)
#define MAX_DEPTH_RS 32                // elements (words) (power of 2)
#define MAX_DEPTH_SS 32                // elements (words) (power of 2)
#define ROM_SIZE_WDS ROM_SIZE / WORD_SIZE
// This VM implementation does not provide any memory-mapped device,
//   therefore we always set MAP_SIZE to 0 here
#define MAP_SIZE 0                     // MAP immediately follows RAM
#define LOWEST_WRITABLE_BYTE ROM_SIZE  // RAM immediately follows ROM
#define HIGHEST_WRITABLE_BYTE ROM_SIZE + RAM_SIZE + MAP_SIZE - 1
#define HIGHEST_WRITABLE_WORD ROM_SIZE + RAM_SIZE + MAP_SIZE - WORD_SIZE
#define PROG_MEMORY ROM_SIZE + RAM_SIZE
#define STDBLK 0
#define STDIN 1
#define STDIMP 2
#define STDOUT -1
#define STDEXP -2
#define FALSE 0
#define TRUE 1
#define LOWEST_SIMPLE_OPCODE iTRACE // FIXME
#define OPCODE_MASK 0b11111111111111111111111100000000
#define LAST_RESTART_CODE_RESET 1     // Indicates program-requested RESET
#define LAST_RESTART_CODE_REBOOT 2    // Indicates program-requested REBOOT
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
#define msgTrapDivideByZero               "DIVIDE BY ZERO    "
#define msgTrapWall                       "HIT WALL          "
#define msgTrapData                       "HIT DATA          "
#define msgTrapPcOverflow                 "PC OVERFLOW       "
#define msgTrap                           "APPLICATION TRAP  "
#define msgDied                           "APPLICATION DIED  "
#define msgBefore                         " just before:     "

#ifdef FVMO_MULTIPLEX
  #define STDIN_BYTE 0b00000001
  #define STDOUT_BYTE 0b01000001
  #define STDTRC_BYTE 0b01000000
#endif

#ifdef FVMO_NO_PROGMEM
   #define PROGMEM
#endif

void clearState();

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

WORD pcTmp;                        // Only used when need to park pc

WORD *rsp;                         // Return stack pointer
#define rsStop MAX_DEPTH_RS        // rs index that bookends its start
WORD rs[rsStop+1];  // Return stack plus one word of empty space for safety
#define RS_EMPTY &rs[rsStop]
#define RS_HAS_ONE &rs[rsStop-1]
#define RS_FULL &rs[0]
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

WORD iOntrapAddress = NONE;    // Address to branch to on trap after iMATH

WORD lastTrapAddress = NONE;   // Address at which last trap occurred

WORD lastExitCode;             // Last automated exit code (if any),
                               //   not preserved after a hard reset.
WORD eMark;                    // Merely marks end of FVM memory space
                               //   except for lastRestartCode.
WORD lastRestartCode;          // Program-requested restart code (if any)
                               //   preserved even after a hard reset.

WORD safePreviousAddress(WORD addr) {
  if (addr > (0+WORD_SIZE)) {
    return addr - WORD_SIZE;
  } else {
    return 0;
  }
}

// ============================================================================
//        VARIABLES FOR TRACING (optional for production VM)
// ============================================================================
BYTE vmFlags = 0  ;   // Flags -------1 = trace on

// ============================================================================

#if FVMP == FVMP_STDIO // -----------------------------------------------------

  #include <stdio.h>
  #include <string.h>
  // ==========================================================================
  //        SYSTEM MEMORY
  // ==========================================================================
  // WARNING: The below must only be used AFTER you have first ensured that
  // the address to be accessed is neither inappropriate nor out of bounds!
  // Such caution is essential since no overflow checking is performed here.
  #ifndef FVMO_SEPARATE_ROM
      #define byteAtAddr(addr) memory[addr]
      #define wordAtAddr(addr) *(WORD *)&memory[addr]
      #define setByteAtAddr(val,addr) memory[addr] = val;
      #define setWordAtAddr(val,addr) *(WORD *)&memory[addr] = val;
      // System memory (even multiple of WORD size)
      BYTE memory[ROM_SIZE + RAM_SIZE + MAP_SIZE];
      // Evaluates to word at specified address in system memory
      #define wordAtAddr(addr) *(WORD *)&memory[addr]
      // Evaluates to word at pc
      #define wordAtPc wordAtAddr(pc);
      // Evaluates to byte at specified address in system memory
      #define byteAtAddr(addr) memory[addr]
      // Zero-fill all FVM system memory
      void clearMem() {
        memset(memory, 0, ROM_SIZE + RAM_SIZE + MAP_SIZE);  
      }
  #else
      BYTE rom[ROM_SIZE];
      BYTE ram[RAM_SIZE+MAP_SIZE];
      BYTE byteAtAddr(WORD addr) {
        if (addr < ROM_SIZE) {
            return rom[addr];
        } else {
          return ram[addr-ROM_SIZE];
        }
      }
      WORD wordAtAddr(WORD addr) {
        WORD result;
        if (addr < ROM_SIZE) {
            result = *(WORD *)&rom[addr];
            return result;
        } else {
          result = *(WORD *)&ram[addr-ROM_SIZE];
          return result;
        }
      }
      #define setByteAtAddr(val,addr) ram[addr-ROM_SIZE] = val;
      #define setWordAtAddr(val,addr) *(WORD *)&ram[addr-ROM_SIZE] = val;
      #define wordAtPc wordAtAddr(pc);
      // Zero-fill all FVM system memory
      void clearMem() {
        memset(ram, 0, RAM_SIZE + MAP_SIZE);
        memset(rom, 0, ROM_SIZE);  
      }
  #endif

  // ==========================================================================
  //        DEVICES
  // ==========================================================================
  #define stdblkFilename         "std.blk"  // Name of file for standard block
  #define romFilename            "rom.fp"   // Name of file for system ROM
  #define stdtrcFilename         "std.trc"  // Name of file for standard trace
  #define stdexpFilename         "std.exp"  // Name of file for standard export
  #define stdimpFilename         "std.imp"  // Name of file for standard import
  #define msgTrapCantOpenStdblk             "CAN'T OPEN STDBLK "
  #define msgTrapCantCloseStdblk            "CAN'T CLOSE STDBLK"
  #define msgTrapCantOpenRom                "CAN'T OPEN ROM    "
  #define msgTrapCantCloseRom               "CAN'T CLOSE ROM   "
  #define msgTrapCantReadRom                "CAN'T READ ROM    "
  #define msgTrapCantOpenStdexp             "CAN'T OPEN STDEXP "
  #define msgTrapCantCloseStdexp            "CAN'T CLOSE STDEXP"
  #define msgTrapCantOpenStdimp             "CAN'T OPEN STDIMP "
  #define msgTrapCantCloseStdimp            "CAN'T CLOSE STDIMP"

  #ifdef FVMO_INCORPORATE_ROM
      #define PROGMEM
      #include "rom.h"
  #else
      FILE *romHandle;                   // File handle for ROM file
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
  #endif

  // FIXME reconsider FVMO_SEPARATE_ROM vs FVMO_INCORPORATE_ROM
  // strategy especially for Linux targets
  #ifdef FVMO_SEPARATE_ROM
    #define PROGMEM // FIXME
  #else
    // FIXME
  #endif
  #if STDBLK_SIZE > 0
    // FIXME
  #endif

  // ===========================================================================
  //                          PRIVATE SERVICES
  // ===========================================================================
  FILE *stdblkHandle;                // File handle for stdblk file
  FILE *stdtrcHandle;                // File handle for stdtrc file
  FILE *stdexpHandle;                // File handle for stdexp file
  FILE *stdimpHandle;                // File handle for stdimp file

  /* Open stdtrc

     Normally this should do:
              stdtrcHandle = fopen(stdtrcFilename, "w")
     but for fvm16-16MB-sr-append for fvmtest change this to:
              stdtrcHandle = fopen(stdtrcFilename, "a")

     FIXME add #ifdef logic for fvmtest mode
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

  void clearDevices() {
    stdblkHandle = NULL;
    #ifndef FVMO_INCORPORATE_ROM
      romHandle = NULL;
    #endif
    stdtrcHandle = NULL;
    stdexpHandle = NULL;
    stdimpHandle = NULL;
  }

  // ==========================================================================
  //        UTILITIES
  // ==========================================================================
    void fvmTraceChar(const char c) {
      #ifdef FVMO_MULTIPLEX
        putchar((char)STDTRC_BYTE);
        putchar(c);
        fflush(stdout);
      #else
        putc(c,stdtrcHandle);
        fflush(stdtrcHandle);
      #endif
    }

  #define fvmSeek(file,val) fseek(file,val,SEEK_SET) != 0
  #define fvmWrite(buf,unitSize,numUnits,file) fwrite(buf,unitSize,numUnits,file)
  #define fvmReadByte(buf,file) fread(buf,1,1,file)
  #define fvmReadWord(buf,file) fread(buf,WORD_SIZE,1,file)
  #define fvmReadByteStdin(buf) fread(buf,1,1,stdin)
  #define fvmReadWordStdin(buf) fread(buf,WORD_SIZE,1,stdin)
  #define fvmWriteByteStdout(buf) fwrite(buf,1,1,stdout)
  #define fvmWriteWordStdout(buf) fwrite(buf,WORD_SIZE,1,stdout)

#endif // #if FVMP == FVMP_STDIO

#if FVMP == FVMP_ARDUINO_IDE // -----------------------------------------------

  #include <Arduino.h>
  #include <SPI.h>
  #ifdef FVMO_SD
      #include <SD.h> // FIXME Could move depending on stdblk support
      #define SD_FILE_TYPE File // FIXME Arduino IDE needs this comment
  #endif

  #ifdef FVMO_SERIALUSB
    #define Serial SerialUSB
  #endif

  #ifdef FVMO_SEPARATE_ROM
    #include "rom.h"
  #else ifdef FVMO_INCORPORATE_ROM
    #include "rom.h"
  #endif

  // ===========================================================================
  //        SYSTEM MEMORY
  // ===========================================================================
  // WARNING: The below must only be used AFTER you have first ensured
  // that the address to be accessed is neither inappropriate nor out of bounds!
  // Such caution is essential since no overflow checking is performed here.
  #ifndef FVMO_SEPARATE_ROM
      #define byteAtAddr(addr) memory[addr]
      #define wordAtAddr(addr) *(WORD *)&memory[addr]
      #define setByteAtAddr(val,addr) memory[addr] = val;
      #define setWordAtAddr(val,addr) *(WORD *)&memory[addr] = val;
      // System memory (even multiple of WORD size)
      BYTE memory[ROM_SIZE + RAM_SIZE + MAP_SIZE];
      // Evaluates to word at specified address in system memory
      #define wordAtAddr(addr) *(WORD *)&memory[addr]
      // Evaluates to word at pc
      #define wordAtPc wordAtAddr(pc);
      // Evaluates to byte at specified address in system memory
      #define byteAtAddr(addr) memory[addr]
      // Zero-fill all FVM system memory
      void clearMem() {
        memset(memory, 0, ROM_SIZE + RAM_SIZE + MAP_SIZE);
      }
  #else // FIXME need overflow protection for ROM access?
      BYTE ram[RAM_SIZE+MAP_SIZE];
      inline BYTE byteAtAddr(WORD addr) {
        if (addr < ROM_SIZE) {
            #ifdef FVMO_NO_PROGMEM
              return prog[addr];
            #else
              return (BYTE)pgm_read_byte_far(prog+addr);
            #endif
        } else {
          return ram[addr-ROM_SIZE];
        }
      }
      inline WORD wordAtAddr(WORD addr) {
        if (addr < ROM_SIZE) { 
            #ifdef FVMO_SAFE_ALIGNMENT
              // FIXME NEXT roll this safe alignment out in all files
              // for all platforms and add test to fvm test
              #ifdef FVMO_NO_PROGMEM
                return (WORD)(prog[addr] |
                        prog[1+addr] << 8 |
                        prog[2+addr] << 16 | 
                        prog[3+addr] << 24);
              #else
                return (WORD)(pgm_read_byte_far(prog+addr) |
                        pgm_read_byte_far(1+prog[0]+addr) << 8 |
                        pgm_read_byte_far(2+prog[0]+addr) << 16 | 
                        pgm_read_byte_far(3+prog[0]+addr) << 24);
              #endif
            #else
              #ifdef FVMO_NO_PROGMEM
                return *(WORD *)&prog[addr];
              #else
                return (WORD)pgm_read_dword_far(prog+addr);
              #endif
            #endif
        } else {
            #ifdef FVMO_SAFE_ALIGNMENT
              return (WORD)(ram[addr-ROM_SIZE] |
                      ram[1+addr-ROM_SIZE] << 8 |
                      ram[2+addr-ROM_SIZE] << 16 | 
                      ram[3+addr-ROM_SIZE] << 24);
            #else
              return *(WORD *)&ram[addr-ROM_SIZE]; 
            #endif
        }
      }
      #define setByteAtAddr(val,addr) ram[addr-ROM_SIZE] = val;
      #ifdef FVMO_SAFE_ALIGNMENT
        inline void setWordAtAddr(WORD val, WORD addr) {
          ram[addr-ROM_SIZE] = val;
          ram[1+addr-ROM_SIZE] = val >> 8;
          ram[2+addr-ROM_SIZE] = val >> 16;
          ram[3+addr-ROM_SIZE] = val >> 24;
        }
      #else
        inline void setWordAtAddr(WORD val, WORD addr) {
          *(WORD *)&ram[addr-ROM_SIZE] = val;
        }
      #endif
      #define wordAtPc wordAtAddr(pc);

      /* Zero-fill all FVM system memory */
      void clearMem() {
        memset(ram, 0, RAM_SIZE + MAP_SIZE);
        // FIXME doco that Arduino only supports real ROM not virtual ROM
      }
  #endif

  // ===========================================================================
  //        DEVICES
  // ===========================================================================
  #define stdblkFilename         "std.blk"   // Name of file for standard block
  #define romFilename            "rom.fp"    // Name of file for system ROM
  #define stdtrcFilename         "std.trc"   // Name of file for standard trace
  #define stdexpFilename         "std.exp"   // Name of file for standard export
  #define stdimpFilename         "std.imp"   // Name of file for standard import

  #define msgTrapCantOpenStdblk             "CAN'T OPEN STDBLK "
  #define msgTrapCantCloseStdblk            "CAN'T CLOSE STDBLK"
  #define msgTrapCantOpenRom                "CAN'T OPEN ROM    "
  #define msgTrapCantCloseRom               "CAN'T CLOSE ROM   "
  #define msgTrapCantReadRom                "CAN'T READ ROM    "
  #define msgTrapCantOpenStdexp             "CAN'T OPEN STDEXP "
  #define msgTrapCantCloseStdexp            "CAN'T CLOSE STDEXP"
  #define msgTrapCantOpenStdimp             "CAN'T OPEN STDIMP "
  #define msgTrapCantCloseStdimp            "CAN'T CLOSE STDIMP"

  #ifdef FVMO_INCORPORATE_ROM
      #include <avr/pgmspace.h>
      // #include "rom.h"
  #else
      #ifdef FVMO_SD
        SD_FILE_TYPE *romHandle;             // File handle for ROM file
        /* Open ROM */ // FIXME note: might be used occasionally
        #define openRom
        /* Close ROM */ // FIXME note: might be used occasionally
        #define closeRom
      #endif
            #endif
            #ifdef FVMO_SEPARATE_ROM // FIXME Complex as SD must be initialized
              #include <avr/pgmspace.h>
      #ifdef FVMO_SD
              #include <SD.h>
      #endif
            #else // FIXME tidy up here and remove incorrect #else/#endifs?
      #ifdef FVMO_SD
            SD_FILE_TYPE romHandle;            // File handle for ROM file
            /* Open ROM */
            #define openRom \
            romHandle = SD.open(romFilename, FILE_READ); \
            if (!romHandle) { \
              goto trapCantOpenRom; \
            }

            /* Close ROM */
            #define closeRom romHandle.close();
      #endif
            #endif

            #if STDBLK_SIZE > 0
      #ifdef FVMO_SD // FIXME
          //#include <SD.h>
          SD_FILE_TYPE stdblkHandle;           // File handle for stdblk file
          /* Open stdblk */
          #define openStdblk \
            stdblkHandle = SD.open(stdblkFilename, FILE_WRITE); \
            if (!stdblkHandle) { \
              goto trapCantOpenStdblk; \
            }
          /* Close stdblk */
          #define closeStdblk stdblkHandle.close();
      #endif
  #endif

  // ==========================================================================
  //   PRIVATE SERVICES // FIXME make all these conditional and tidy up
  // ==========================================================================
  #ifdef FVMO_SD
        SD_FILE_TYPE stdtrcHandle;               // File handle for stdtrc file
        SD_FILE_TYPE stdexpHandle;               // File handle for stdexp file
        SD_FILE_TYPE stdimpHandle;               // File handle for stdimp file
  #endif

  /* Open stdtrc

     Normally this should do:
              stdtrcHandle = fopen(stdtrcFilename, "w")
     but for fvm16-16MB-sr-append for fvmtest change this to:
              stdtrcHandle = fopen(stdtrcFilename, "a")

     FIXME add #ifdef logic for fvmtest mode
  */

  // FIXME bogus for now as don't want to enforce SD card
  // requirement just for tracing
  #define openStdtrc
  #define closeStdtrc 

  /*
  #define openStdtrc \
  stdtrcHandle = SD.open(stdtrcFilename, FILE_WRITE); \
  if (!stdtrcHandle) { \
    goto trapCantOpenStdtrc; \
  }

  /* Close stdtrc *//*
  #define closeStdtrc stdtrcHandle.close();
  */

  /* Open stdexp */ // FIXME
  #define openStdexp

  /* Close stdexp */ // FIXME
  #define closeStdexp

  /* Open stdimp */ // FIXME
  #define openStdimp

  /* Close stdimp */ // FIXME
  #define closeStdimp

  void clearDevices() {
    // FIXME Anything to do here on Arduino?
    /*
    stdblkHandle = NULL;
    #ifndef FVMO_INCORPORATE_ROM
      romHandle = NULL;
    #endif
    stdtrcHandle = NULL;
    stdexpHandle = NULL;
    stdimpHandle = NULL;
    */
  }

  // ===========================================================================
  //        UTILITIES
  // ===========================================================================
  void fvmTraceChar(const char c) {
    #ifdef FVMO_MULTIPLEX
      Serial.print((char)STDTRC_BYTE);
    #endif
    Serial.print(c); // FIXME should never go to stdtrc?
    Serial.flush();
  }

  #ifdef FVMO_SD // FIXME
        #define fvmSeek(file,pos) file.seek(pos) == 0
  /*#ifdef FVMO_SD
        // FIXME NEXT These 3 SD_FILE_TYPEs are the Energia problem
        //   so temporarily commenting out
        #define fvmReadByte(buf,file) ardReadByte(buf,file)
        #define fvmReadWord(buf,file) ardReadWord(buf,file)
        int ardReadByte(WORD *buf, SD_FILE_TYPE file) {
          while (file.available() < 1) {};
          *buf = file.read();
          return 1;
        }
        int ardReadWord(WORD *buf, SD_FILE_TYPE file) {
          while (file.available() < 4) {};
          WORD b1 = file.read();
          WORD b2 = file.read();
          WORD b3 = file.read();
          WORD b4 = file.read();
          *buf = (b4 << 24) + (b3 <<16) + (b2<<8) + b1;
          return 4;
        }

        int ardWrite(SD_FILE_TYPE file, char *buf, int numBytes) {
           int numBytesWritten = file.write(buf,numBytes);
           // file.flush(); FIXME
           return numBytesWritten;
        }
  */
  #else 
        #define fvmReadByte(buf,file) 0
        #define fvmReadWord(buf,file) 0
  #endif

  int ardReadByteStdin(WORD *buf) {
    while (Serial.available() < 1) {};
    *buf = Serial.read();
    return 1;
  }
  int ardReadWordStdin(WORD *buf) {
    while (Serial.available() < 4) {};
    WORD b1 = Serial.read();
    WORD b2 = Serial.read();
    WORD b3 = Serial.read();
    WORD b4 = Serial.read();
    *buf = (b4 << 24) + (b3 <<16) + (b2<<8) + b1;
    return 4;
  }
  int ardReadByteStdin(WORD buf) {
    while (Serial.available() < 1) {};
    buf = Serial.read();
    return 1;
  }
  int ardWriteStdout(char *buf, int numBytes) {
     const uint8_t *bbuf = (const uint8_t *)buf; // FIXME this line is for Energia
     int numBytesWritten = Serial.write(bbuf,numBytes);
     // file.flush(); FIXME
     return numBytesWritten;
  }
  #define fvmWrite(buf,unitSize,numUnits,file) ardWriteStdout((char *)buf,numUnits) // FIXME NEXT
  #define fvmReadByteStdin(buf) ardReadByteStdin(buf)
  #define fvmReadWordStdin(buf) ardReadWordStdin(buf)
  #define fvmWriteByteStdout(buf) ardWriteStdout((char *)buf,1)

#endif // #if FVMP == FVMP_ARDUINO_IDE  // ------------------------------------

/*=============================================================================
  TRACING
=============================================================================*/
#ifdef FVMO_TRON
  const char hex[0x10] = {'0','1','2','3','4','5','6','7','8','9',
                      'a','b','c','d','e','f'};

  void fvmTraceNewline() {
    fvmTraceChar('\r');
    fvmTraceChar('\n');
  }

  /* For tracing: print a message up to 256 characters long */
  void fvmTrace(const char *msg) {
    BYTE i = 0;    
    while ((i <= BYTE_MAX) && (msg[i] != 0)) {
      fvmTraceChar(msg[i]);
      i++;
    }
  }

  /* For tracing: print a byte in hexadecimal */
  void fvmTraceByteHex(BYTE b) {
    BYTE h = (b >> 4) & 0x0f;
    fvmTraceChar(hex[h]);
    h = b & 0x0f;
    fvmTraceChar(hex[h]);
  }

  /* For tracing: print a byte in hexadecimal */
  void fvmTraceWordHex(WORD n) { /* TODO make generic */
    if (n == NONE) {
      fvmTrace("  NONE  ");
      return;
    }
    BYTE b = (n >> 24) & 0x000000ff;
    fvmTraceByteHex(b);
    b = (n >> 16) & 0x000000ff;
    fvmTraceByteHex(b);
    b = (n >> 8) & 0x000000ff;
    fvmTraceByteHex(b);
    b = n & 0x000000ff;
    fvmTraceByteHex(b);
  }

  /* For tracing: pretty print a mnemonic up to 8 characters long */
  void fvmTraceMnemonic(const char *msg) {
    int i = 0;
    int spaces = 8;      
    while ((i < 8) && (msg[i] != 0)) {
      fvmTraceChar(msg[i]);
      i++;
      spaces--;
    }
    while (spaces > 0) {
      fvmTraceChar(' ');
      spaces--;
    }
  }

  // =========================================================================
  //                 TRACING (optional for production VM)
  // =========================================================================
  #define traceInfo \
    { \
      WORD opcode = wordAtPc \
      if (opcode >= 0 && opcode <=256) {\
        fvmTraceWordHex(pc); \
        fvmTraceChar(' '); \
        fvmTraceMnemonic(traceTable[opcode]); \
      } else { \
        fvmTraceWordHex(pc); \
        fvmTrace("  "); \
      } \
      if (opcode < LOWEST_SIMPLE_OPCODE && pc \
          < (HIGHEST_WRITABLE_WORD - WORD_SIZE)) { \
        WORD cellValue = wordAtAddr(pc+WORD_SIZE); \
        fvmTraceWordHex(cellValue); \
        fvmTraceChar(' '); \
      } else { \
        fvmTrace("         "); \
      } \
    } 
  #define traceStacks \
    { \
    fvmTrace("( "); \
    int i = 1; \
    int numElems = (DS_EMPTY-dsp); \
    for (; i<=numElems; i++) { \
      fvmTraceWordHex(ds[dsStop-i]); \
      fvmTraceChar(' ');  \
    }  \
    fvmTrace(") "); \
    fvmTrace("[ "); \
    i = 1; \
    numElems = (SS_EMPTY-ssp); \
    for (; i<=numElems; i++) { \
      fvmTraceWordHex(ss[ssStop-i]); \
      fvmTraceChar(' ');  \
    }  \
    fvmTrace("] "); \
    fvmTrace("{ "); \
    i = 1; \
    numElems = (RS_EMPTY-rsp); \
    for (; i<=numElems; i++) { \
      fvmTraceWordHex(rs[rsStop-i]); \
      fvmTraceChar(' ');  \
    }  \
    fvmTraceChar('}'); \
    fvmTraceNewline(); \
    }

#endif

// ===========================================================================
//                          FVM RUNTIME LOGIC
//       In C this has to be packaged into a runfvm() function
//              to allow use of goto and labels for speed  
// ===========================================================================
int runfvm() {

#ifdef FVMO_TRON // traceTable: 
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
  "math    ", // Start of 107 empty fillers in blocks of 10...
  "trap    ",
  "die     ",
  "read?   ",
  "write?  ",
  "get?    ",
  "put?    ", // FIXME reconsider order of new instructions
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
  "        ",           // 10+ empty fillers
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "        ",
  "trace?  ", // FIXME reconsider order of new instructions
  "zoom    ",
  "rchan?  ",
  "wchan?  ",
  "gchan?  ",
  "pchan?  ",
  "pc?     ",
  "[fly]   ",
  "swap2   ",
  "rev4    ",
  "tor4    ",
  "rot4    ", // End of 107 empty fillers in blocks of 10
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
#endif // .ifdef FVMO_TRON

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

WORD iOntrapCount = 0; // Used to implement iOntrap instruction

#ifdef FVMO_INCORPORATE_ROM
  int romIndex = 0; // Used by systemIncorporateRom // FIXME not needed for arduino, also move these up
#endif

  goto start;

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
  if (pc >= HIGHEST_WRITABLE_WORD) { \
    goto trapPcOverflow; \
  } \
  pc = pc + WORD_SIZE; \
  if (pc >= PROG_MEMORY || pc < 0) { \
    goto trapPcOverflow; \
  }

// Branches to address in the next program cell
// Note: the check below ensures branch address lies within program memory
#define branch \
  pc = wordAtAddr(pc); \
  if (pc >= PROG_MEMORY || pc < 0) { \
    goto trapPcOverflow; \
  }

// Skips the next program cell (rather than branching)
#define dontBranch \
  incPc

// Branches to address in iOntrapAddress upon arithmetic trap (supports iMATH)
// Note: the check below ensures branch address lies within program memory
#define branchFromMath \
  pc = iOntrapAddress; \
  if (pc >= PROG_MEMORY || pc < 0) { \
    goto trapPcOverflow; \
  }

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

// Peeks at the fourth element from the top of the data stack into rD
#define peekFourthDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  rD = *(dsp+3);

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

// Replaces the value on top of the data stack with value in rC
//   and second-top value on the data stack with value in rA
//   and third-top value on the data stack with value in rB
//   and fourth-top value on the data stack with value in rD
// WARNING: THIS IS INTENTIONALLY DIFFERENT TO fourPeekDs.
#define fourReplaceDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rD; \
  dsp[1] = rA; \
  dsp[2] = rB; \
  dsp[3] = rC;

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

// Replaces the value on top of the data stack with value in rB
//   and second-top value on the data stack with value in rC
//   and third-top value on the data stack with value in rA
// WARNING: THIS IS INTENTIONALLY DIFFERENT TO fourReplaceDs.
#define fourRReplaceDs \
  if (dsp > DS_HAS_FOUR) { \
    goto trapDsUnderflow; \
  } \
  dsp[0] = rB; \
  dsp[1] = rC; \
  dsp[2] = rD; \
  dsp[3] = rA;

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
//                           INSTRUCTION SET
// ===========================================================================
// opcodeTable
  // haltOpcode (1)
    #define iWALL 0 // WALL must be zero
  // complexOpcodes (44)
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
    #define iMATH 38
    #define iTRAP 39
    #define iDIE 40
    #define iREAD 41
    #define iWRITE 42
    #define iGET 43
    #define iPUT 44
  // complexOpcodesEnd
  // simpleOpcodes (123)
    #define iTRACE 133
    #define iZOOM 134
    #define iRCHANN 135
    #define iWCHANN 136
    #define iGCHANN 137
    #define iPCHANN 138
    #define iPC 139
    #define iRDJMP 140
    #define iSWAP2 141
    #define iREV4 142
    #define iTOR4 143
    #define iROT4 144
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
  setIOdefaults

#if FVMP == FVMP_STDIO // ----------------------------------------------------
  //systemInitDevices:
    openStdtrc
    openStdblk
    openStdexp
    openStdimp
  //  setIOdefaults

  #ifdef FVMO_INCORPORATE_ROM
    #ifdef FVMO_SEPARATE_ROM
  // label not needed:    systemIncorporateRom:
        for(romIndex = 0; romIndex<(prog_size); romIndex++) {
          rom[romIndex] = prog[romIndex];
        };
    #else
  // label not needed:    systemIncorporateRom: // FIXME check for overflow in all Linux and Arduino impls here
        for(romIndex = 0; romIndex<(prog_size); romIndex++) {
          memory[romIndex] = prog[romIndex];
        };
    #endif
  #else
    #ifdef FVMO_SEPARATE_ROM
  // label not needed:    systemLoadProgram:
        openRom
        if (fread(&rom,WORD_SIZE,ROM_SIZE_WDS,romHandle) == 0) {
          goto trapCantReadRom; // ROM read failed for some reason
        }
        closeRom
    #else
  // label not needed:    systemLoadProgram:
        openRom
        if (fread(&memory,WORD_SIZE,ROM_SIZE_WDS,romHandle) == 0) {
          goto trapCantReadRom; // ROM read failed for some reason
        }
        closeRom
    #endif
  #endif
#endif // #if FVMP == FVMP_STDIO ----------------------------------------------

#if FVMP == FVMP_ARDUINO_IDE  // ----------------------------------------------
  // systemInitDevices: // moved label to fvm.c
  // FIXME suspect the below won't work when relevant
  #ifdef FVMO_SD
    openStdtrc
    #if STDBLK_SIZE > 0
      openStdblk
    #endif
    openStdexp
    openStdimp
  #endif
  // setIOdefaults  // moved label to fvm.c


  #ifndef FVMO_INCORPORATE_ROM

   // systemLoadProgram: // label probably not needed
  //FIXME make this suitable for Arduino and add all 4 options

  /* Commenting out for now as irrelevant for now
        if (fread(&memory,WORD_SIZE,ROM_SIZE_WDS,romHandle) == 0) {
          goto trapCantReadRom; // ROM read failed for some reason
        }
  */
  #endif
#endif // FVMP == FVMP_ARDUINO_IDE  -------------------------------------------

systemInitCore:
    dsp = DS_EMPTY;                 // Initialize data stack pointer
    rsp = RS_EMPTY;                 // Initialize return stack pointer
    ssp = SS_EMPTY;                 // Initialize software stack pointer
    pc = 0;                         // Set program counter to start of program

  nextInstruction: // Begin Freeputer program execution

    #ifdef FVMO_TRON
      optTrace
    #endif

    rA = wordAtPc
    incPc

    if ((0b1111111111111111111111100000000 & rA) != 0) {
      goto trapIllegalOpcode;
    }

    if (iOntrapCount == 2) {
      iOntrapCount = 1;
    } else if (iOntrapCount == 1) {
      iOntrapAddress = NONE;
      iOntrapCount = 0;
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
      case iMATH:
        iOntrapAddress = wordAtPc
        iOntrapCount = 2;
        incPc
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
      case iREV4:   //  ( n1 n2 n3 n4 -- n4 n3 n2 n1 )
        if (dsp > DS_HAS_FOUR) {
          goto trapDsUnderflow;
        }
        rA = dsp[0];    // n1
        rB = dsp[1];    // n2
        rC = dsp[2];    // n3
        rD = dsp[3];    // n4
        dsp[3] = rA;
        dsp[2] = rB;
        dsp[1] = rC;
        dsp[0] = rD;
        break;
      case iREAD:   //  ( -- TRUE/FALSE ) Is reador channel number supported?
        rA = wordAtPc
        if (rA == 1) {
          rA = TRUE;
        } else {
          rA = FALSE;
        }
        pushDs
        incPc
        break;
      case iWRITE:   //  ( -- TRUE/FALSE ) Is writor channel number supported?
        rA = wordAtPc
        if (rA ==-1) {
          rA = TRUE;
        } else {
          rA = FALSE;
        }
        pushDs
        incPc
        break;
      case iGET:   //  ( -- TRUE/FALSE ) Is getor channel number supported?
        rA = wordAtPc
#if STDBLK_SIZE > 0
        if (rA == 0) {
          rA = TRUE;
        } else {
          rA = FALSE;
        }
#else
        rA = FALSE;
#endif
        pushDs
        incPc
        break;
      case iPUT:   //  ( -- TRUE/FALSE ) Is putor channel number supported?
        rA = wordAtPc
#if STDBLK_SIZE > 0
        if (rA == 0) {
          rA = TRUE;
        } else {
          rA = FALSE;
        }
#else
        rA = FALSE;
#endif
        pushDs
        incPc
        break;
      case iTRACE:   //  ( -- TRUE/FALSE ) Is tracing supported?
        rA = TRUE;
        pushDs
        break;
      case iGETOR:
        switch(gchannel) {
#if STDBLK_SIZE > 0
          case STDBLK:
            getBuf = 0; // zero the buffer
            popDs
            if (rA >= STDBLK_SIZE || rA < 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fvmSeek(stdblkHandle,rA)) {
              branch // seek failed
              break;
            }
            if (fvmReadWord(&getBuf,stdblkHandle) < 1) {
              branch // get failed
              break;
            }
            rA = getBuf;
            pushDs
            dontBranch
            break;
#endif
          default:
            branch // Unsupported gchannel
            break;
        }
        break;
      case iGETORB:
        switch(gchannel) {
#if STDBLK_SIZE > 0
          case STDBLK:
            getBuf = 0; // zero the buffer
            popDs
            if (rA >= STDBLK_SIZE || rA < 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fvmSeek(stdblkHandle,rA)) {
              branch // seek failed
              break;
            }
            if (fvmReadByte(&getBuf,stdblkHandle) < 1) {
              branch // get failed
              break;
            }
            rA = getBuf;
            pushDs
            dontBranch
            break;
#endif
          default:
            branch // Unsupported gchannel
            break;
        }
        break;
      case iPUTOR:
        switch(pchannel) {
#if STDBLK_SIZE > 0
          case STDBLK:
            twoPopDs     // Address in now in rB
            putBuf = rA; // Value to be put is now in putBuf
            if (rB >= STDBLK_SIZE || rB < 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fvmSeek(stdblkHandle,rB)) {
              branch // seek failed
              break;
            }
            if (fvmWrite(&putBuf,WORD_SIZE,1,stdblkHandle) < 1) {
              branch // put failed
              break;
            }
            dontBranch
            break;
#endif
          default:
            branch // Unsupported pchannel
            break;
        }
        break;
      case iPUTORB:
        switch(pchannel) {
#if STDBLK_SIZE > 0
          case STDBLK:
            twoPopDs     // Address in now in rB
            putBuf = rA; // Value to be put is now in putBuf
            if (rB >= STDBLK_SIZE || rB < 0) {
              branch // outside block device bounds, cannot get
              break;
            }
            if (fvmSeek(stdblkHandle,rB)) {
              branch // seek failed
              break;
            }
            if (fvmWrite(&putBuf,1,1,stdblkHandle) < 1) {
              branch // put failed
              break;
            }
            dontBranch
            break;
#endif
          default:
            branch // Unsupported pchannel
            break;
        }
        break;
      case iREADOR:  // FIXME FVMO_MULTIPLEX //
        readBuf = 0; // zero the buffer
        switch(rchannel) {
          case STDIN:
            if (fvmReadWordStdin(&readBuf) < 1) {
              branch // read failed
              break;
            } else {
              rA = readBuf;
              pushDs
              dontBranch
              break;
            }
          break;
/* Not supported yet FIXME
          case STDIMP:
            if (fvmReadWord(&readBuf,stdimpHandle) < 1) {
              branch // read failed
              break;
            } else {
              rA = readBuf;
              pushDs
              dontBranch
              break;
            }
          break;
*/
          default:
            branch // Unsupported rchannel
            break;
        }
        break;
      case iREADORB:
        readBuf = 0; // zero the buffer
        switch(rchannel) {
          case STDIN:
            if (fvmReadByteStdin(&readBuf) < 1) {
              branch // read failed
              break;
            }
#ifdef FVMO_MULTIPLEX
// FIXME FVMO_MULTIPLEX // Actually this will not work without buffering if there
// is more than 1 input stream attached to the FVM instance and they are all
// sending data to the FVM simultaneoulsy by push rather than pull
            if (readBuf != STDIN_BYTE) {
              branch // read is from wrong input stream
              break;
            } else {
              // Now get data byte
              if (fvmReadByteStdin(&readBuf) < 1) {
                branch // read failed
                break;
              }
            }
#endif
            rA = readBuf;
            pushDs
            dontBranch
            break;
/* Not supported yet FIXME
          case STDIMP:
            if (fvmReadByte(&readBuf,stdimpHandle) < 1) {
              branch // read failed
              break;
            }
            rA = readBuf;
            pushDs
            dontBranch
            break;
*/
          default:
            branch // Unsupported rchannel
            break;
        }
        break;
      case iWRITOR:
        switch(wchannel) {
          case STDOUT:
            popDs
#ifdef FVMO_MULTIPLEX // FIXME refactor and roll out across all
              writeBuf = STDOUT_BYTE;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }
              writeBuf = rA;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }

              writeBuf = STDOUT_BYTE;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }
              rA = rA >> 8; writeBuf = rA;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }

              writeBuf = STDOUT_BYTE;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }
              rA = rA >> 8; writeBuf = rA;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }

              writeBuf = STDOUT_BYTE;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }
              rA = rA >> 8; writeBuf = rA;
              if (fvmWriteByteStdout(&writeBuf) < 1) { branch break; }
              dontBranch
#else
            writeBuf = rA;
            if (fvmWriteWordStdout(&writeBuf) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
            }
#endif
            break;
/* Not supported yet FIXME
          case STDEXP:
            popDs
            writeBuf = rA;
            if (fvmWrite(&writeBuf,WORD_SIZE,1,stdexpHandle) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
            }
            break;
*/
          default:
            branch // Unsupported wchannel
            break;
        }
        break;
      case iWRITORB:
        switch(wchannel) {
          case STDOUT:
            popDs
#ifdef FVMO_MULTIPLEX
            writeBuf = STDOUT_BYTE;
            if (fvmWriteByteStdout(&writeBuf) < 1) {
              branch  // write failed
              break;
            }
#endif
            writeBuf = rA;
            if (fvmWriteByteStdout(&writeBuf) < 1) {
              branch  // write failed
              break;
            } else {
              dontBranch
              break;
            }
/* Not supported yet FIXME
          case STDEXP:
            popDs
            writeBuf = rA;
            if (fvmWrite(&writeBuf,1,1,stdexpHandle) < 1) {
              branch  // write failed
            } else {
              dontBranch
              break;
            }
*/
          default:
            branch // Unsupported wchannel
            break;
        }
        break;
      case iTRACOR:  // FIXME MULTIPLEX (roll out across all I/O instrs)
        popDs
        writeBuf = rA;
        if (fvmWrite(&writeBuf,WORD_SIZE,1,stdtrcHandle) < 1) {
          branch // trace failed
          break;
        } else {
          dontBranch
          break;
        }
        break;
      case iTRACORB:
        popDs
#ifdef FVMO_MULTIPLEX
        writeBuf = STDTRC_BYTE;
        if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) {
          branch // trace failed
          break;
        }
#endif
        writeBuf = rA;
        if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) {
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
          for (; i>rA; i--) {
            addr = rC+(i*WORD_SIZE);
            if (rB == wordAtAddr(addr)) {
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
          for (; i<rA; i++) {
            addr = rC+(i*WORD_SIZE);
            if (rB == wordAtAddr(addr)) {
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
          for (; i>rA; i--) {
            addr = rC+i;
            if (byteAtAddr(addr) == rB) {
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
          for (; i<rA; i++) {
            addr = rC+i;
            if (byteAtAddr(addr) == rB) {
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
            for (; i<rA; i++) {
              addr = rC+(i*WORD_SIZE);
              setWordAtAddr(rB,addr);
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
            for (; i<rA; i++) {
              addr = rC+i;
              setByteAtAddr(rB,addr)
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
          for (; i<rA; i++) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            if (wordAtAddr(w1addr) != wordAtAddr(w2addr)) {
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
          for (; i<rA; i++) {
            b1addr = rB+i;
            b2addr = rC+i;
            if (byteAtAddr(b1addr) != byteAtAddr(b2addr)) {
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
          for (; i<rA; i++) {
            b1addr = rB+i;
            b2addr = rC+i;
            setByteAtAddr(byteAtAddr(b1addr), b2addr)
          }
          break;
        } else {
          // numBytes is negative, do descending move
          int i = 0;
          int b1addr;
          int b2addr;
          for (; i>rA; i--) {
            b1addr = rB+i;
            b2addr = rC+i;
            setByteAtAddr(byteAtAddr(b1addr), b2addr)
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
          for (; i<rA; i++) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            setWordAtAddr(wordAtAddr(w1addr), w2addr);
          }
          break;
        } else {
          // numWords is negative, do descending move
          int i = 0;
          int w1addr;
          int w2addr;
          for (; i>rA; i--) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            setWordAtAddr(wordAtAddr(w1addr), w2addr);
          }
          break;
        }
        break;
      case iLOADB:    //  ( a -- byte )
        popDs
        ensureByteAddressable(rA)
        rA = byteAtAddr(rA); // Retrieve byte starting at specified byte
        pushDs
        break;
      case iRLOADB:   //  ( a -- a byte )
        peekDs
        ensureByteAddressable(rA)
        rA = byteAtAddr(rA); // Retrieve byte starting at specified byte
        pushDs
        break;
      case iLOAD:     //  ( a -- word )
        popDs
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve word starting at specified byte
        pushDs
        break;
      case iRLOAD:    //  ( a -- a word )
        peekDs
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve word starting at specified byte
        pushDs
        break;
      case iPLOAD: ;  //  ( p -- word ) Like load but assumes address is a pointer
                      //                and therefore loads word from address stored at p
        popDs
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve addr starting at specified byte
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA);
        pushDs
        break;
      case iRPLOAD:  //  ( p -- p word ) Like rload but assumes address is a pointer
        peekDs
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve addr starting at specified byte
        ensureWordAddressable(rA)
        rA = wordAtAddr(rA);
        pushDs
        break;        break;
      case iPLOADB: ; //  ( p -- p word ) Like loadb but assumes address is a pointer
                      //                and therefore loads byte from address stored at p
        popDs
        ensureByteAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve addr starting at specified byte
        ensureByteAddressable(rA)
        rA = byteAtAddr(rA);
        pushDs
        break;
      case iRPLOADB: ; // ( p -- p word ) Like rloadb but assumes address is a pointer
                       //               and therefore loads byte from address stored at p
        peekDs
        ensureByteAddressable(rA)
        rA = wordAtAddr(rA); // Retrieve addr starting at specified byte
        ensureByteAddressable(rA)
        rA = byteAtAddr(rA);
        pushDs
        break;
      case iSTORE: ;  //  ( n a -- )
        popDs
        rB = rA;                  // Address is in rB
        popDs                     // Value to store is in rA
        ensureWordAddrWritable(rB)
        setWordAtAddr(rA,rB);
        break;
      case iSTOREB: ; //  ( n a -- )
        popDs
        rB = rA;                  // Address is in rB
        popDs                     // Value to store is in rA
        ensureByteAddrWritable(rB)
        setByteAtAddr(rA,rB)
        break;
      case iPSTORE: ; //  ( n p -- )      Like STORE but assumes p is a pointer
                      //                 and therefore stores word to address stored at p
        popDs
        rB = rA;                  // Pointer is in rB
        popDs                     // Value to store is in rA
        ensureWordAddressable(rB)
        rB = wordAtAddr(rB);
        ensureWordAddrWritable(rB)
        setWordAtAddr(rA,rB);
        break;
      case iPSTOREB: ; // ( n p -- )      Like STOREB but assumes p is a pointer
                       //                and therefore stores word to address stored at p
        popDs
        rB = rA;                  // Pointer is in rB
        popDs                     // Value to store is in rA
        ensureWordAddressable(rB)
        rB = wordAtAddr(rB);
        ensureByteAddrWritable(rB)
        setByteAtAddr(rA,rB)
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
        if (rB >= PROG_MEMORY || rB < 0) {
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
        if (rB >= PROG_MEMORY || rB < 0) {
          goto trapPcOverflow;
        }
        rA = pc;                  // rA now contains return address  
        pushRs                    // Return address is now on return stack
        pc = rB;                  // pc now contains call address
        break;
      case iEXIT:
        popRs
        // Ensure return address lies within program memory
        if (rA >= PROG_MEMORY || rA < 0) {
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
        if (rA >= PROG_MEMORY || rA < 0) {
          goto trapPcOverflow;
        }
        pc = rA;
        break;
      case iRDJMP:     //  ( a -- a )
        peekDs                    // rA now contains jump address             
        // Ensure dynamic jump address lies within program memory
        if (rA >= PROG_MEMORY || rA < 0) {
          goto trapPcOverflow;
        }
        pc = rA;
        break;
      case iZOOM:     //  ( n1 n2 n3 n3 -- n1 n2 n3 n4 n1 )
        peekFourthDs
        rA = rD;
        pushDs
        break;
      case iRCHANN:
        rA = rchannel;
        pushDs
        break;
      case iWCHANN:
        rA = wchannel;
        pushDs
        break;
      case iGCHANN:
        rA = gchannel;
        pushDs
        break;
      case iPCHANN:
        rA = pchannel;
        pushDs
        break; 
      case iPC:     //  ( -- a ) address of current instruction (namely iPC)
        rA = pc - WORD_SIZE;
        pushDs
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
      case iSWAP2:     //  ( n1 n2 n3 n4 -- n3 n4 n1 n2 )
        fourPeekDs
        ds[0] = rC;
        ds[1] = rD;
        ds[2] = rA;
        ds[3] = rB;
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
      case iROT4:      //  ( n1 n2 n3 n4 -- n2 n3 n4 n1 )
        fourPeekDs
        fourReplaceDs
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
      case iTOR4:      //  ( n1 n2 n3 n4 -- n4 n1 n2 n3 )
        fourPeekDs
        fourRReplaceDs
        break;
      case iHALT:     //  ( -- )
        goto exitSuccess;
        break;
      case iTRAP:     //  ( -- )
        rB = wordAtPc
        goto trap;
        break;
      case iDIE:      //  ( -- )
        rB = wordAtPc
        goto die;
        break;
      case iWALL:     //  ( -- )
        goto trapWall;
        break;
      case iDATA:     //  ( -- )
        goto trapData;
        break;
      case iLIT:      //  ( -- n )
        rA = wordAtPc
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
            /* FIXME the following uncommented overflow check is incorrect.
               It needs to be replaced by the commented code shown here.
               The equivalent fix also needs to be applied to FVM.java.
               fvmtest.fl also needs to be changed to detect this
               as it was not catching this malfunction.
            if ((rA != 0) && (rA - NEG_INT_MAX) < rB) {
              goto trapMathOverflow;
            }
            */
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

#ifdef FVMO_SD
#if STDBLK_SIZE > 0
  if (stdblkHandle) {
    closeStdblk                 // Close the standard block device
  }
#endif
if (stdexpHandle) {
  closeStdexp                 // Close stdexp
}
if (stdimpHandle) {
  closeStdimp                 // Close stdimp
}
if (stdtrcHandle) {
  closeStdtrc                 // Close stdtrc
}
#endif
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
  return rB;                  // Return from runfvm()

// ===========================================================================
//                              EXIT TRACING
// ===========================================================================
// Send an error message to stdtrc
// along with information regarding current program state.
#ifdef FVMO_TRON
  #define traceExit(msg) \
    fvmTrace(msg); \
    fvmTrace(" at "); fvmTraceWordHex(lastTrapAddress);  fvmTraceNewline(); \
    traceInfo \
    traceStacks
  #define traceExitMsg(msg) \
    fvmTrace(msg); \
    fvmTrace(" at "); fvmTraceWordHex(lastTrapAddress);  fvmTraceNewline();
#else
  #define traceExit(msg) \
    fvmTrace(msg); \
    fvmTrace(" at "); fvmTraceWordHex(lastTrapAddress);  fvmTraceNewline();
  #define traceExitMsg(msg) \
    fvmTrace(msg); \
    fvmTrace(" at "); fvmTraceWordHex(lastTrapAddress);  fvmTraceNewline();
#endif // .ifdef FVMO_TRON

// ===========================================================================
//                                TRAPS
// ===========================================================================
trap:
  lastTrapAddress = safePreviousAddress(pc);
  traceExit(msgTrap)
  goto systemReset;
die:
  lastTrapAddress = safePreviousAddress(pc);
  traceExit(msgDied)
  goto systemExit;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL PROGRAM FLOW
//----------------------------------------------------------------------------
trapWall:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 2;
  traceExit(msgTrapWall)
  goto systemReset;
trapData:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 3;
  traceExit(msgTrapData)
  goto systemReset;
trapPcOverflow:
  lastTrapAddress = pc;
  rB = 4;
  traceExitMsg(msgTrapPcOverflow)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL OPCODES
//----------------------------------------------------------------------------
iNONE:
trapIllegalOpcode:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 11;
  traceExit(msgTrapIllegalOpcode)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL MATHEMATICAL OPERATIONS
//----------------------------------------------------------------------------
trapMathOverflow:
  if (iOntrapAddress != NONE) {
    branchFromMath
    goto nextInstruction;
  }
  lastTrapAddress = safePreviousAddress(pc);
  rB = 21;
  traceExit(msgTrapMathOverflow)
  goto systemReset;
trapDivideByZero:
  if (iOntrapAddress != NONE) {
    branchFromMath
    goto nextInstruction;
  }
  lastTrapAddress = safePreviousAddress(pc);
  rB = 22;
  traceExit(msgTrapDivideByZero)
  goto systemReset;
trapXsBitshift:
  if (iOntrapAddress != NONE) {
    branchFromMath
    goto nextInstruction;
  }
  lastTrapAddress = safePreviousAddress(pc);
  rB = 23;
traceExit(msgTrapXsBitshift)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL STACK OPERATIONS
//----------------------------------------------------------------------------
trapDsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 31;
  traceExit(msgTrapDsUnderflow)
  goto systemReset;
trapDsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 32;
  traceExit(msgTrapDsOverflow)
  goto systemReset;
trapRsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 33;
  traceExit(msgTrapRsUnderflow)
  goto systemReset;
trapRsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 34;
  traceExit(msgTrapRsOverflow)
  goto systemReset;
trapSsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 35;
  traceExit(msgTrapSsUnderflow)
  goto systemReset;
trapSsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 36;
  traceExit(msgTrapSsOverflow)
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL MEMORY ACCESS
//----------------------------------------------------------------------------
trapMemBounds:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 41;
  traceExitMsg(msgTrapMemBounds)
  goto systemReset;
trapRAMBounds:
  lastTrapAddress = safePreviousAddress(pc);
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
  goto exitFail;
trapCantCloseRom:
  rB = 52;
  traceExitMsg(msgTrapCantCloseRom)
  goto exitFail;
trapCantReadRom:
  rB = 53;
  traceExitMsg(msgTrapCantReadRom)
  goto exitFail;
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
  goto exitFail;
trapCantCloseStdblk:
  rB = 62;
  traceExitMsg(msgTrapCantCloseStdblk)
  goto exitFail;
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
  goto exitFail;
trapCantCloseStdexp:
  rB = 75;
  traceExitMsg(msgTrapCantCloseStdexp)
  goto exitFail;

//Note: to create a 'std.imp' file of 0 size on Linux simply use:
//           touch std.imp
trapCantOpenStdimp:
  rB = 77;
  traceExitMsg(msgTrapCantOpenStdimp)
  goto exitFail;
trapCantCloseStdimp:
  rB = 78;
  traceExitMsg(msgTrapCantCloseStdimp)
  goto exitFail;
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

  clearDevices();

  pcTmp = 0;

  rsp = NULL;
  memset(rs, 0, WORD_SIZE + (MAX_DEPTH_RS * WORD_SIZE));

  ssp = NULL;
  memset(ss, 0, WORD_SIZE + (MAX_DEPTH_SS * WORD_SIZE));  
  //ss[MAX_DEPTH_SS * WORD_SIZE];

  dsp = NULL;
  memset(ds, 0, WORD_SIZE + (MAX_DEPTH_DS * WORD_SIZE));
}

#if FVMP == FVMP_STDIO  // ---------------------------------------------------
  int main() {
    return runfvm();
  }
#endif   // ------------------------------------------------------------------

#if FVMP == FVMP_ARDUINO_IDE // ----------------------------------------------
  void setup() {

  /*
  // FIXME Fubarino only add this as a callback
      // change this to match your SD shield or module;
      // Arduino Ethernet shield: pin 4
      // Adafruit SD shields and modules: pin 10
      // Sparkfun SD shield: pin 8
      // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
      // Note that even if it's not used as the CS pin, the hardware SS pin 
      // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
      // or the SD library functions will not work. 

      // Default SD chip select for Uno and Mega type devices
      const int chipSelect_SD_default = 10; // Change 10 to 53 for a Mega

      // chipSelect_SD can be changed if you do not use default CS pin
      const int chipSelect_SD = 27;
        // Make sure the default chip select pin is set to so that
        // shields that have a device that use the default CS pin
        // that are connected to the SPI bus do not hold drive bus
        pinMode(chipSelect_SD_default, OUTPUT);
        digitalWrite(chipSelect_SD_default, HIGH);

        pinMode(chipSelect_SD, OUTPUT);
        digitalWrite(chipSelect_SD, HIGH);
  // End of Fubarino only section
  */

    Serial.begin(115200);
    while (!Serial) {;}

    fvmTraceNewline();

  /*
    // FIXME the below is unnecessary if no SD-card-based stdblk
    if (!SD.begin(chipSelect_SD)) {
      Serial.println("initialization failed!"); // FIXME use fvmTrace...
      return;
    }
  */

    fvmTrace("About to run FVM...");
    fvmTraceNewline();

  }
  void loop() {
    int theExitCode = runfvm();
    fvmTrace("FVM exit code was: ");
    fvmTraceWordHex(theExitCode);
    fvmTraceNewline();
    while(true);
  }
#endif // #if FVMP == FVMP_ARDUINO_IDE // -------------------------------------
// ============================================================================

