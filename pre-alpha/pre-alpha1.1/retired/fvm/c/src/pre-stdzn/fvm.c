/* 

                        FREEPUTER VIRTUAL MACHINE

Program:    fvm.c
Copyright © Robert Gollagher 2015, 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    20150822
Updated:    20160506:0901 [with explanatory note added 20160506:0937]
Version:    pre-alpha-0.0.0.48 for FVM 1.1

    [EXPLANATORY NOTE UPON RETIRING THIS CODE:
       This old version of 'fvm.c' has been moved to the 'retired'
       folder because it was decided to standardize FVM sizings
       and this 'fvm.c' contains configurations for sizings
       which do not conform to the new standard sizings.

       For example, the FVMC_ARDUINO_MEGA_SD53_FVMTEST configuration
       in this 'fvm.c' uses the FVMOS_SIZE_FVMTEST_ARDUINO sizing
       which provides 4 kB of RAM and 128 kB of ROM. This was
       very convenient for running Freeputer on Arduino Mega 2560
       and worked very nicely, including passing fvmtest (although in
       reality this 'fvm.c' implementation as used on the Mega could
       really only support programs up to 32 kB in size due to
       the limitations of C arrays using signed int indexes
       and therefore 'fvmtest.fl' had to be broken up
       and run in three parts). The new standard sizing will
       require more RAM than the Arduino Mega possesses and will instead
       support Arduino Due, chipKIT Max32 and the like.]

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

All instructions assume your workstation is Debian Linux.

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
for a Linux target with the provided Makefile is simply:

  make

Which is equivalent to:

  gcc -o fvm fvm.c

If your target is Arduino or chipKIT then of course you will need to use
the Arduino IDE to build the FVM executable rather than running make. To do
so, rename this 'fvm.c' file to 'fvm.ino' and open it with the Arduino IDE.
Assuming that you are using the FVMO_INCORPORATE_ROM option you must of
course also ensure 'rom.h' is in the same directory as your 'fvm.ino'.
Alternatively, use appropriate symbolic links for convenience.
Use Arduino IDE 1.6.7 or higher.

==============================================================================

  The following targets have successfully run the 'ts.fl' tape server
  on an 'fvm.c' FVM instance connected by serial connection over USB to a
  'tape.c' tape terminal running on Linux.

  Of course, doing so requires appropriate 'fvm.c' configuration
  for the target in question and for the desired mode of tape operation.
  It also requires matching 'tape.c' configuration for that same mode.
  Lastly, slotFloor in 'ts.fl' must equal ROM_SIZE in 'fvm.c'.

  TARGETS CURRENTLY WORKING WELL:

==============================================================================

  LINUX
  * Debian Linux

  ARDUINO IDE
  * Arduino Mega 2560 (clone)
  * Arduino Due (clone)
  * Freetronics EtherDue
  * chipKIT Max32
  
==============================================================================
  TARGETS CURRENTLY NOT WORKING
==============================================================================

  ARDUINO IDE
  * Fubarino SD 1.5 (incompatible serial communication)
  * DuinoMite-Mega  (incompatible serial communication)

==============================================================================
  TARGETS FOR WHICH SUPPORT HAS BEEN WITHDRAWN
==============================================================================
  
  Earlier versions of this 'fvm.c' supported some MSP430 Launchpads and
  at least one Tiva C Series Launchpad via the Energia IDE. Unfortunately
  the Energia IDE 0101E0017 has since been unreliable in this context 
  and extremely difficult to debug when a compile error occurs.
  Accordingly as on 20160331 a decision has been made to abandon
  using the Energia IDE and hence all TI Launchpads. This is not to say
  that this 'fvm.c' could not be successfully ported to TI Launchpads
  using a tool other than the Energia IDE; it probably could be
  but doing so is beyond the current scope of this project.

  Earlier versions of this 'fvm.c' supported running small Freeputer
  programs on the Arduino Uno. However, after much thought, a decision has
  been made on 20160331 to withdraw support for the Arduino Uno as a target.
  This is because it has insufficient flash memory (only 32 kB) to
  conveniently accommodate 'fvm.c' except when compiled without
  SD card support, and because meaningfully running the 'fvmtest.fl'
  test suite on an Uno is not easily possible, thus making it
  difficult to verify the full and correct function of the FVM.
  An Arduino Mega 2560 has none of these problems.

  As a rule of thumb for the near future, this project will only
  bother using microcontrollers with at least 128 kB of flash (or similar
  persistent memory technology) and at least 8 kB of RAM. This is because
  the project also targets Linux computers, and other platforms such as Java,
  and a balance needs to be struck between supporting the very small and
  the very large. After all, Freeputer is primarily intended to be
  an extremely portable platform for software development and therefore
  it makes sense to support small but not miniscule devices.

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

NOTE:
=====

  The use of this multiplexing scheme, as opposed to some other scheme
  or no multiplexing at all, is optional. It is merely provided as a
  convenient option for use with this FVM implementation. This is
  not meant to suggest that this has to be a Freeputer standard.
  Indeed, an application running within an FVM instance does
  not know or care whether that FVM uses multiplexing.


IMPORTANT WARNINGS REGARDING THIS 'fvm.c' MULTIPLEXING IMPLEMENTATION:
======================================================================

  (1) The limited multiplexing implemented in this 'fvm.c' works
      reasonably well when the FVM instance, and any process it communicates
      with, are hosted on a sophisticated operating system such as Linux.
      This is because Linux provides comprehensive buffering.

  (2) The limited multiplexing implemented in this 'fvm.c' works
      reasonably well when the FVM instance is on an Arduino (or similar)
      and the only communication is to a process (such as the 'tape.c'
      tape terminal) running on Linux (or similar operating system).
      This is because Linux provides comprehensive buffering.

  (3) The limited multiplexing implemented in this 'fvm.c' works almost
      tolerably well, but only if slow baud rates are used, when the
      FVM instance is on an Arduino (or similar) communicating with a
      process (such as the 'tape-clcd' tape terminal) running on
      another Arduino. Problems arise from limited buffering.

  (4) This implementation never multiplexes anything other than stdout,
      stdtrc and stdin. That is, even when in FVMO_MULTIPLEX mode there
      is no multiplexing of stdimp, stdexp and stdblk.

  (5) In summary: whenever reasonably possible you should avoid
      using multiplexing. However, it is undeniably convenient and
      very useful for (2) and of some use for (3). In future it is likely
      that multiplexing support will be dropped from this implementation
      and instead the use of FVMO_STDTRC_SEP will be promoted;
      but first 'tape.c' will need to be enhanced.

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
  #define FVMO_STDTRC_FILE_ALSO // Send stdtrc output to 'std.trc' file also
  #define FVMO_STDTRC_STDOUT // Send stdtrc serial output to stdout instead
  #define FVMO_STDTRC_SEP // Use second serial connection for stdtrc (Arduino)
  #define FVMO_SLOW_BAUD // Use low baud rate when multiplexing
  #define FVMO_VERY_SLOW_BAUD // Use very low baud rate when multiplexing
  #define FVMO_SEPARATE_ROM // Use real (or at least separate) ROM memory
  #define FVMO_INCORPORATE_ROM // Incorporate 'rom.h' program in FVM executable
  #define FVMO_INCORPORATE_BIN // Incorporate 'rom.fp' ditto // FIXME give more instructions
  #define FVMO_NO_UPCASTS // Avoid doing *(WORD *)&memory[addr] // FIXME needs comprehensive testing in all variants
  #define FVMO_NO_PROGMEM // Target does not understand the PROGMEM keyword
  #define FVMO_SMALL_ROM // Target only supports _near not _far pgm_read
  #define FVMO_SERIALUSB // Target uses SerialUSB (Arduino Due native port)
  #define FVMO_SD // Target has SD card and uses Arduino SD library
  #define FVMO_SD_CS_PIN 4 // Specify CS pin for SD card for your board
  #define FVMO_FVMTEST // A special mode for running the fvmtest suite
  #define FVMO_STDIMP // Support stdimp // FIXME add one for stdblk (eg EEPROM not SD Card etc rather than size)
  #define FVMO_STDEXP // Support stdexp
  #define FVMO_SDCROM // Use in-situ ROM on SD card (exceedingly slow)
  #define FVMO_STDIN_FROM_FILE // Use 'std.in' file as stdin (eg on SD card) // FIXME and for Linux

  WARNING regarding FVMO_SD_CS_PIN: be sure to consult the documentation for
  your board and/or SD card shield to determine the correct pin. Examples:
  Ethernet Shield = 4; Freetronics EtherDue = 4; Fubarino SD (1.5) = 27.
  Note that even if it is not used as the CS pin, the hardware SS pin
  (10 on most Arduino boards, 53 on the Mega) must be left as an
  output or the SD library functions will not work.

  New feature: Incorporate a tape device into the FVM executable
  and use it as stdin and stdout (monolithic but fast and sometimes convenient)
  rather than connecting to a tape terminal over a serial connection.
  Currently this can only be used on the platform FVMP_ARDUINO_IDE
  and is only supported by the 'tape-clcd.ino' implementation. Of course
  the Arduino board running your FVM must then have a keyboard and CLCD display.
  Note that you can still lose characters if your type faster than the system
  can keep up (presumably due to the behaviour of the keyboard buffering).
  For reasons of quirks of the Arduino IDE, the source code of the
  tape (such as 'tape-clcd.ino') must be soft-linked to, or renamed as,
  a local header file of the same name (such as 'tape-clcd.h') to accompany
  this 'fvm.c' (used as 'fvm.ino') and the #define is done thus:
  #define FVMO_LOCAL_TAPE "tape-clcd.h" // Tape definition to include

  If in addition to using FVMO_LOCAL_TAPE you may wish also to, as an aid to
  debugging, send stdtrc output (which you can already see in split mode
  of your local tape display device anyway) to the ordinary Serial port
  to view or log on a device or computer over a serial connection.
  To do so, in addition to defining FVMO_LOCAL_TAPE also do:
  #define FVMO_STDTRC_ALSO_SERIAL // Send stdtrc both to local tape and serial

  Note that FVMO_STDTRC_SEP is still a valid alternative to multiplexing
  even when using FVMO_LOCAL_TAPE mode, since FVMO_LOCAL_TAPE mode will
  in that case use a separate memcom for stdtrc.

  Notes on FVMO_NO_UPCASTS based on fvmtest results:
  ==================================================

    FVMO_NO_UPCASTS is not needed by:

      Arduino Due, Arduino Mega 2560

    FVMO_NO_UPCASTS is needed by:

      chipKIT Max32

    FVMO_NO_UPCASTS cannot be used on (breaks):



    FVMO_NO_UPCASTS can be used successfully on:

      Arduino Due, Arduino Mega 2560, chipKIT Max32


  Boards known to have fully passed the fvmtest suite:
  ====================================================

  Listed are boards that have passed the complete 'fvmtest.fl' test suite
  in at least one possible configuration for the board (not necessarily
  in all possible configurations that this 'fvm.c' allows).

            BOARD                 DATE FIRST PASSED

    Freetronics EtherDue              20160329
    Arduino Mega 2560 (clone)         20160330
    Arduino Due (clone)               20160331
    chipKIT Max32                     20160331


  GENERAL NOTE
  ============

  Don't forget to set the corresponding mode #defines in your tape source code
  to the ones found in this 'fvm.c' FVM source code, or things won't work.
  Also, slotFloor in 'ts.fl' must equal ROM_SIZE in 'fvm.c'.

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
#define FVMC_ARDUINO_DUE_SD4_FLC_SMALL

// ===========================================================================
//                SOME EXAMPLE CONFIGURATIONS TO CHOOSE FROM:
// ===========================================================================

/* A mini Linux FVM without multiplexing */
#ifdef FVMC_LINUX_MINI
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
#endif

/* A mini Arduino FVM with a built-in CLCD tape user interface.
   Requires a CLCD display and PS/2 keyboard directly
   driven by the Arduino running the FVM. */
#ifdef FVMC_ARDUINO_MINI_CLCD
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_LOCAL_TAPE "tape-clcd.h"
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_ALSO_SERIAL
#endif

/* A mini Linux FVM with multiplexing */
#ifdef FVMC_LINUX_MINI_MUX
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

/* A mini Linux FVM without multiplexing */
#ifdef FVMC_LINUX_MINI
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
#endif

/* A mini Linux FVM with multiplexing and a very slow baud rate.
   Suitable when connected to a very slow Arduino CLCD tape terminal
   such as a 40x4 CLCD running on an Arduino Mega 2560. */
#ifdef FVMC_LINUX_MINI_MUX_VERY_SLOW
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_VERY_SLOW_BAUD
#endif

/* A mini Linux FVM with multiplexing and a slow baud rate. */
#ifdef FVMC_LINUX_MINI_MUX_SLOW
  #define FVMOS_LINUX
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_SLOW_BAUD
#endif

/* A mini Arduino FVM with multiplexing.
   Suitable for an Arduino Mega 2560 when connected
   to a very slow Arduino CLCD tape terminal such as
   a 40x4 CLCD running on an Arduino Mega 2560. */
#ifdef FVMC_ARDUINO_MINI_MUX_VERY_SLOW
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_VERY_SLOW_BAUD
#endif

/* A mini Arduino FVM with multiplexing.
   Suitable for Arduino Mega 2560 */
#ifdef FVMC_ARDUINO_MINI_MUX
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

/* A mini Arduino FVM with multiplexing and an SD card whose CS pin is 4.
   Suitable for Freetronics EtherDue board. 
*/
#ifdef FVMC_ARDUINO_SD4_MINI_MUX
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 4
  #define FVMO_STDTRC_FILE_ALSO
#endif

/* A mini Arduino FVM with multiplexing and an SD card whose CS pin is 4.
   Also 16 MB stdblk (as a file on that SD card).
   Suitable for Freetronics EtherDue board. */
#ifdef FVMC_ARDUINO_SD4_MINI_STDBLK16MB_MUX
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI_STDBLK16MB
  #define FVMO_MULTIPLEX
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 4
  #define FVMO_STDTRC_FILE_ALSO
#endif

/* An Arduino FVM suitable for running 'fvmtest.fl' on Arduino Mega.
   Pins are connected as follows (Mega to SD Card):
      53 (SS)   to CS
      5V        to VCC
      GND       to GND
      ICSP MOSI to MOSI
      ICSP MISO to MISO
      ICSP SCK  to SCK

   This configuration uses FVMO_INCORPORATE_ROM.

   Arduino Mega passed fvmtest suite on: 20160330
 */
#ifdef FVMC_ARDUINO_MEGA_SD53_FVMTEST
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 53
  // #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_STDOUT
  // #define FVMO_NO_UPCASTS // Arduino Mega 2560 passes fvmtest either way
#endif

/* An Arduino FVM suitable for running 'fvmtest.fl' on Arduino Mega.
   Pins are connected as follows (Mega to SD Card):
      53 (SS)   to CS
      5V        to VCC
      GND       to GND
      ICSP MOSI to MOSI
      ICSP MISO to MISO
      ICSP SCK  to SCK

   This configuration uses FVMO_INCORPORATE_BIN.

   Arduino Mega passed fvmtest suite on: 20160330
 */
#ifdef FVMC_ARDUINO_MEGA_SD53_FVMTEST_BIN
  #define FVMOS_ARDUINO_BIN
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 53
  // #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_STDOUT
  // #define FVMO_NO_UPCASTS // Arduino Mega 2560 passes fvmtest either way
#endif

/* 
   Like FVMC_ARDUINO_MEGA_SD53_FVMTEST but uses FVMO_SDCROM instead of
   FVMO_INCORPORATE_ROM and therefore is exceedingly slow.
   See notes for FVMOS_ARDUINO_SDCROM below.

   Your SD card must of course have a 'rom.fp' file, containing the compiled
   Freeputer program (as Freeputer bytecode) that you wish to run.

   WARNING: running fvmtest in this way does not test if using
   the physical ROM memory of a microcontroller as Freeputer ROM works
   or not (and therefore does not test FVMO_NO_UPCASTS at all).

   Arduino Mega passed fvmtest suite on: 20160331
 */
#ifdef FVMC_ARDUINO_MEGA_SD53_FVMTEST_SDCROM
  #define FVMOS_ARDUINO_SDCROM
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 53
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_STDOUT
#endif

/* An Arduino FVM suitable for running 'fvmtest.fl'
   on Freetronics EtherDue using its native port and inbuilt SD card.
   This configuration uses FVMO_INCORPORATE_ROM.

   Freetronics Due passed fvmtest suite on: 20160329
 */
#ifdef FVMC_ARDUINO_ETHERDUE_SD4_FVMTEST
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 4
  // #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDTRC_SEP
  #define FVMO_SERIALUSB // FIXME TODO also test if tests pass without this
  // #define FVMO_NO_UPCASTS // FIXME TODO also test if tests pass with this
  #define FVMO_STDTRC_STDOUT
#endif

/* An Arduino FVM suitable for running 'fvmtest.fl' on Arduino Due.

   Pins are connected as follows (Due to SD Card):
      4         to CS
      3.3 V     to VCC
      GND       to GND
      ICSP MOSI to MOSI
      ICSP MISO to MISO
      ICSP SCK  to SCK

   This configuration uses FVMO_INCORPORATE_ROM.

   Arduino Due passed fvmtest suite on: 20160331
 */
#ifdef FVMC_ARDUINO_DUE_SD4_FVMTEST
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 4
  // #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_STDOUT
  #define FVMO_SERIALUSB // Arduino Due passes fvmtest either way
  // #define FVMO_NO_UPCASTS // Arduino Due passes fvmtest either way
#endif

/* A small Arduino Due FVM for running flc with minimal RAM.

   Pins are connected as follows (Due to SD Card):
      4         to CS
      3.3 V     to VCC
      GND       to GND
      ICSP MOSI to MOSI
      ICSP MISO to MISO
      ICSP SCK  to SCK

   This configuration uses FVMO_INCORPORATE_ROM.

*/
#ifdef FVMC_ARDUINO_DUE_SD4_FLC_SMALL
  #define FVMOS_ARDUINO
  #define FVMO_STDIN_FROM_FILE // FIXME
  #define FVMO_STDTRC_SEP // FIXME DELETEME
  #define FVMO_STDTRC_STDOUT // FIXME DELETEME
  #define FVMOS_SIZE_SD4_FLC_SMALL
#endif

/* An Arduino FVM suitable for running 'fvmtest.fl' on chipKIT Max32.

   Pins are connected as follows (Max32 to SD Card):
      53 (SS)   to CS
      3.3 V     to VCC
      GND       to GND
      51 (MOSI) to MOSI
      50 (MISO) to MISO
      52 (SCK)  to SCK

  chipKIT Max32 passed fvmtest suite on: 20160331

 */
#ifdef FVMC_CHIPKIT_MAX32_SD53_FVMTEST
  #define FVMOS_CHIPKIT
  #define FVMOS_SIZE_FVMTEST_ARDUINO
  #define FVMO_FVMTEST
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 53
  // #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDTRC_SEP
  #define FVMO_STDTRC_STDOUT
#endif

/* A mini Arduino FVM, not multiplexed, and using a second serial connection
   for stdtrc. Suitable for Arduino Due or Mega 2560 connected to an Arduino
   tape terminal (and producing good terminal performance). */
#ifdef FVMC_ARDUINO_MINI_TRC
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_STDTRC_SEP
#endif

/* A mini Arduino FVM with multiplexing and a slow baud rate.
   Suitable for Arduino Due or Mega 2560 connected to an Arduino tape
   terminal (and producing poor terminal performance).  */
#ifdef FVMC_ARDUINO_MINI_MUX_SLOW
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_SLOW_BAUD
#endif

/* A mini chipKIT FVM with multiplexing.
   Suitable for chipKIT Max32 */
#ifdef FVMC_CHIPKIT_MINI_MUX
  #define FVMOS_CHIPKIT
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
#endif

/* A mini chipKIT FVM without multiplexing. */
#ifdef FVMC_CHIPKIT_MINI
  #define FVMOS_CHIPKIT
  #define FVMOS_SIZE_MINI
#endif

/* A small chipKIT FVM for running flc with minimal RAM */
#ifdef FVMC_CHIPKIT_SD53_FLC_SMALL
  #define FVMOS_CHIPKIT
  #define FVMO_STDIN_FROM_FILE // FIXME
  #define FVMO_STDTRC_SEP // FIXME DELETEME
  #define FVMO_STDTRC_STDOUT // FIXME DELETEME
  #define FVMOS_SIZE_SD53_FLC_SMALL
#endif

/* A mini chipKIT FVM without multiplexing and a slow baud rate. */
#ifdef FVMC_CHIPKIT_MINI_MUX_SLOW
  #define FVMOS_CHIPKIT
  #define FVMOS_SIZE_MINI
  #define FVMO_MULTIPLEX
  #define FVMO_SLOW_BAUD
#endif

/* A tiny Arduino FVM with multiplexing. */
#ifdef FVMC_ARDUINO_TINY_MUX
  #define FVMOS_ARDUINO
  #define FVMOS_SIZE_TINY
  #define FVMO_SMALL_ROM
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
  #define FVMO_STDTRC_FILE_ALSO
  #define FVMO_STDIMP
  #define FVMO_STDEXP
#endif

/* Generic option set: typical options for Arduinos larger than Uno
   and using the file 'rom.fp' on an SD card directly (in situ) as ROM.
   This is exceedingly slow! However, it allows huge Freeputer programs to
   run (at a snail's pace) on microcontrollers whose inbuilt
   physical ROM is too small to accommodate such programs.
   The main purpose of this configuration is to run the 'fvmtest.fl'
   test suite conveniently on small microcontrollers. Don't bother using
   this configuration if the physical ROM of your microcontroller is large
   enough to accommodate fvmtest (in that case use FVMOS_ARDUINO
   or FVMOS_ARDUINO_BIN instead; they use FVMO_INCORPORATE_ROM or
   FVMO_INCORPORATE_BIN respectively rather than FVMO_SDCROM and
   will run one or two orders of magnitude faster).

   Your SD card must of course have a 'rom.fp' file, containing the compiled
   Freeputer program (as Freeputer bytecode) that you wish to run.

   IMPORTANT: you MUST also #define FVMO_SD and must also ensure that
   you define FVMO_SD_CS_PIN correctly for your board.

 */
#ifdef FVMOS_ARDUINO_SDCROM
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_SDCROM
//  #define FVMO_NO_UPCASTS
#endif

/* Generic option set: typical options for Arduinos larger than Uno
   and using binary incorporation of 'rom.fp' rather than using 'rom.h' */
#ifdef FVMOS_ARDUINO_BIN
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_BIN
//  #define FVMO_NO_UPCASTS
#endif

/* Generic option set: typical options for Arduinos larger than Uno. */
#ifdef FVMOS_ARDUINO
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_ROM
//  #define FVMO_NO_UPCASTS
#endif

/* Generic option set: typical chipKIT options */
#ifdef FVMOS_CHIPKIT
  #define FVMP FVMP_ARDUINO_IDE
  #define FVMO_TRON
  #define FVMO_SEPARATE_ROM
  #define FVMO_INCORPORATE_ROM
  #define FVMO_NO_UPCASTS // chipKIT always requires FVMO_NO_UPCASTS
  #define FVMO_NO_PROGMEM // chipKIT always requires FVMO_NO_PROGMEM
#endif

/* Sizing: tiny */
#ifdef FVMOS_SIZE_TINY
  #define ROM_SIZE 4096 
  #define RAM_SIZE 512
  #define STDBLK_SIZE 0
#endif

/* Sizing: mini */
#ifdef FVMOS_SIZE_MINI
  #define ROM_SIZE 32768
  #define RAM_SIZE 2048
  #define STDBLK_SIZE 0
#endif

/* Sizing: small for running flc in stdblk mode (minimal RAM usage)
   using FVMO_SD_CS_PIN 53
*/
#ifdef FVMOS_SIZE_SD53_FLC_SMALL
  #define ROM_SIZE 126976
  #define RAM_SIZE 4096
  #define STDBLK_SIZE 16777216
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 53
#endif

/* Sizing: small for running flc in stdblk mode (minimal RAM usage) 
   using FVMO_SD_CS_PIN 4 
*/
#ifdef FVMOS_SIZE_SD4_FLC_SMALL
  #define ROM_SIZE 126976
  #define RAM_SIZE 4096
  #define STDBLK_SIZE 16777216
  #define FVMO_SD
  #define FVMO_SD_CS_PIN 4
#endif

/* Sizing: mini with 16 MB stdblk */
#ifdef FVMOS_SIZE_MINI_STDBLK16MB
  #define ROM_SIZE 32768
  #define RAM_SIZE 2048
  #define STDBLK_SIZE 16777216
#endif

/* Sizing: small for running 'fvmtest.fl' suite */
#ifdef FVMOS_SIZE_FVMTEST_ARDUINO
  #define ROM_SIZE 126976
  #define RAM_SIZE 4096
  #define STDBLK_SIZE 16777216
#endif

// ===========================================================================
#ifdef FVMO_SMALL_ROM // FIXME refactor these to generic name
  #define pgm_read_byte_far pgm_read_byte_near
  #define pgm_read_dword_far pgm_read_dword_near
#endif

// FIXME rationalize this baud rate stuff, it is too complex and doesn't play nicely with some config combos 
#ifdef FVMO_STDTRC_SEP
  #define BAUD_RATE_STDTRC 9600 // 115200 // FIXME 38400
#endif

#ifdef FVMO_MULTIPLEX
  #define STDIN_BYTE 0b00000001
  #define STDOUT_BYTE 0b01000001
  #define STDTRC_BYTE 0b01000000
  #ifdef FVMO_SLOW_BAUD
    #define BAUD_RATE 4800
  #endif
  #ifndef BAUD_RATE
    #ifdef FVMO_VERY_SLOW_BAUD
      #define BAUD_RATE 2400
    #else
      #define BAUD_RATE 115200
    #endif
  #endif
#else
  #define BAUD_RATE 9600 // FIXME 115200
#endif

#ifdef FVMO_NO_PROGMEM
   #define PROGMEM
#endif

#ifdef FVMO_STDTRC_STDOUT
  // FIXME need a more robust, comprehensive solution here
  #define Serial1 Serial
#endif

#ifdef FVMO_INCORPORATE_BIN
  #define FVMO_INCORPORATE_ROM
#endif
// ============================================================================
//                             EXPERIMENTAL!
// ============================================================================
#ifdef FVMO_LOCAL_TAPE
   #include FVMO_LOCAL_TAPE
#endif
// ============================================================================
// ============================================================================

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
const static char msgTrapIllegalOpcode[] PROGMEM =    "ILLEGAL OPCODE    ";
const static char msgTrapMathOverflow[] PROGMEM =     "MATH OVERFLOW     ";
const static char msgTrapDsOverflow[] PROGMEM =       "DS OVERFLOW       ";
const static char msgTrapDsUnderflow[] PROGMEM =      "DS UNDERFLOW      ";
const static char msgTrapRsOverflow[] PROGMEM =       "RS OVERFLOW       ";
const static char msgTrapRsUnderflow[] PROGMEM =      "RS UNDERFLOW      ";
const static char msgTrapSsOverflow[] PROGMEM =       "SS OVERFLOW       ";
const static char msgTrapSsUnderflow[] PROGMEM =      "SS UNDERFLOW      ";
const static char msgTrapXsBitshift[] PROGMEM =       "XS BITSHIFT       ";
const static char msgTrapMemBounds[] PROGMEM =        "BEYOND MEM BOUNDS ";
const static char msgTrapRAMBounds[] PROGMEM =        "BEYOND RAM BOUNDS ";
const static char msgTrapDivideByZero[] PROGMEM =     "DIVIDE BY ZERO    ";
const static char msgTrapWall[] PROGMEM =             "HIT WALL          ";
const static char msgTrapData[] PROGMEM =             "HIT DATA          ";
const static char msgTrapPcOverflow[] PROGMEM =       "PC OVERFLOW       ";
const static char msgTrap[] PROGMEM =                 "APPLICATION TRAP  ";
const static char msgDied[] PROGMEM =                 "APPLICATION DIED  ";
const static char msgBefore[] PROGMEM =               " just before:     ";

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
      #ifdef FVMO_NO_UPCASTS
        // FIXME untested
        // FIXME this is wrong
        inline WORD wordAtAddr(WORD addr) {
          return (WORD)(memory[addr] |
                  memory[addr+1] << 8 |
                  memory[addr+2] << 16 | 
                  memory[addr+3] << 24);
        }
      #else
        // Evaluates to word at specified address in system memory
        #define wordAtAddr(addr) *(WORD *)&memory[addr]
      #endif
      #define setByteAtAddr(val,addr) memory[addr] = val;
      #ifdef FVMO_NO_UPCASTS
        // FIXME untested
        inline void setWordAtAddr(WORD val, WORD addr) {
          memory[addr] = val;
          memory[addr+1] = val >> 8;
          memory[addr+2] = val >> 16;
          memory[addr+3] = val >> 24;
        }
      #else
        #define setWordAtAddr(val,addr) *(WORD *)&memory[addr] = val;
      #endif
      // System memory (even multiple of WORD size)
      BYTE memory[ROM_SIZE + RAM_SIZE + MAP_SIZE];
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
        #ifdef FVMO_NO_UPCASTS
          // FIXME untested
          // FIXME this is wrong
          WORD result;
          result = rom[addr];
          result = result | rom[addr+1];
          result = result | rom[addr+2];
          result = result | rom[addr+3];
          return result;

/*
          return (WORD)(rom[addr] |
                  rom[addr+1] << 8 |
                  rom[addr+2] << 16 | 
                  rom[addr+3] << 24);
*/
        #else
            result = *(WORD *)&rom[addr];
            return result;
        #endif
        } else {
        #ifdef FVMO_NO_UPCASTS
          // FIXME untested
          // FIXME this is wrong
          WORD result;
          result = ram[addr-ROM_SIZE];
          result = result | ram[addr+1-ROM_SIZE] << 8;
          result = result | ram[addr+2-ROM_SIZE] << 16;
          result = result | ram[addr+3-ROM_SIZE] << 24;
          return result;
/*
          return (WORD)(ram[addr-ROM_SIZE] |
                  ram[addr+1-ROM_SIZE] << 8 |
                  ram[addr+2-ROM_SIZE] << 16 | 
                  ram[addr+3-ROM_SIZE] << 24);
*/
        #else
          result = *(WORD *)&ram[addr-ROM_SIZE];
          return result;
        #endif
        }
      }
      #define setByteAtAddr(val,addr) ram[addr-ROM_SIZE] = val;
      #ifdef FVMO_NO_UPCASTS
        // FIXME untested
        // FIXME this is wrong
        inline void setWordAtAddr(WORD val, WORD addr) {
          ram[addr-ROM_SIZE] = val;
          ram[addr+1-ROM_SIZE] = val >> 8;
          ram[addr+2-ROM_SIZE] = val >> 16;
          ram[addr+3-ROM_SIZE] = val >> 24;
        }
      #else
        #define setWordAtAddr(val,addr) *(WORD *)&ram[addr-ROM_SIZE] = val;
      #endif
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
      #ifdef FVMO_INCORPORATE_BIN
/* FIXME
      // FIXME untested in this context (but known to work elsewhere)
  __asm__ __volatile__(
      "  .section  .rodata             \n\t"
      "  .global   prog                \n\t"
      "  .type     prog , @object      \n\t"
      "  .align    4                   \n\t"
      "prog:                           \n\t"
      "  .incbin   \"rom.fp\"          \n\t"
      "  .global   prog_size           \n\t"
      "  .type     prog_size, @object  \n\t"
      "  .align    4                   \n\t"
      "prog_size:                      \n\t"
      "  .int      prog_size - prog    \n\t"
  );
  // FIXME these probably need changing
  extern char prog[]; // Was working perfectly on Linux elsewhere
  extern unsigned int prog_size;
*/
      #else
        #include "rom.h"
      #endif
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
 
  #ifdef FVMO_FVMTEST
    /* Open stdtrc for appending (suitable for fvmtest suite) */
    #define openStdtrc \
    stdtrcHandle = fopen(stdtrcFilename, "a"); \
    if (!stdtrcHandle) { \
      goto trapCantOpenStdtrc; \
    }
  #else
    /* Open stdtrc for writing (not appending) */ 
    #define openStdtrc \
    stdtrcHandle = fopen(stdtrcFilename, "w"); \
    if (!stdtrcHandle) { \
      goto trapCantOpenStdtrc; \
    }
  #endif

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
        #ifdef FVMO_STDTRC_FILE_ALSO
        if (stdtrcHandle) {
          putc(c,stdtrcHandle);
          fflush(stdtrcHandle);
        }        
        #endif
      #else
        #ifdef FVMO_STDTRC_FILE_ALSO
        if (stdtrcHandle) {
          putc(c,stdtrcHandle);
          fflush(stdtrcHandle);
        } 
        #endif
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
      #include <SD.h>
      #define SD_FILE_TYPE File  // FIXME Arduino IDE needs this comment!!
      #define FVMO_STDIMP
      #define FVMO_STDEXP
      int ardReadByte(WORD *buf, SD_FILE_TYPE file);
      int ardReadWord(WORD *buf, SD_FILE_TYPE file);
  #else
      #define SD_FILE_TYPE int // FIXME a hack for stdtrcHandle
  #endif

  #ifdef FVMO_SERIALUSB
    #define Serial SerialUSB
  #endif

  #ifdef FVMO_INCORPORATE_ROM
      #include <avr/pgmspace.h>
      #ifdef FVMO_INCORPORATE_BIN
  // FIXME document that this will NOT compile on ARM boards (eg Arduino Due)
  // using the Arduino IDE (at least not without hacking the IDE gcc config)
  // FIXME need also to consider FVMO_NO_PROGMEM
  // FIXME fully qualified path is inconvenient here
  // IMPORTANT: When compiling for Arduino, you must use a fully qualified
  // path to your rom.fp file in the .incbin line below. For example:
  //        "  .incbin   \"/home/sally/foo/bar/rom.fp\"          \n\t"
  __asm__ __volatile__(
      "  .section  .progmem.data       \n\t"
      "  .global   prog                \n\t"
      "  .type     prog , @object      \n\t"
      "  .align    4                   \n\t"
      "prog:                           \n\t"
      "  .incbin   \"/home/rob/Dev/Freeputer2/16th-spike/rom.fp\"          \n\t"
      "  .global   prog_size           \n\t"
      "  .type     prog_size, @object  \n\t"
      "  .align    4                   \n\t"
      "prog_size:                      \n\t"
      "  .long     prog_size - prog    \n\t"
  );
  extern const unsigned char PROGMEM prog[];
  extern unsigned int prog_size; // Maximum 32 kB on 8-bit Arduinos, sadly
      #else
        #include "rom.h"
      #endif
  #else
      #ifdef FVMO_SD
        SD_FILE_TYPE romHandle; // File handle for ROM file
        /* Open ROM */
        #define openRom \
        romHandle = SD.open(romFilename, FILE_READ); \
        if (!romHandle) { \
          goto trapCantOpenRom; \
        }
        /* Close ROM */
        #define closeRom romHandle.close();
      #endif // #ifdef FVMO_SD
  #endif

  // ===========================================================================
  //        SYSTEM MEMORY
  // ===========================================================================
  // WARNING: The below must only be used AFTER you have first ensured
  // that the address to be accessed is neither inappropriate nor out of bounds!
  // Such caution is essential since no overflow checking is performed here.
  #ifndef FVMO_SEPARATE_ROM
      #define byteAtAddr(addr) memory[addr]
      #define setByteAtAddr(val,addr) memory[addr] = val;
      #ifdef FVMO_NO_UPCASTS
        // FIXME untested
        inline void setWordAtAddr(WORD val, WORD addr) {
          memory[addr] = val;
          memory[addr+1] = val >> 8;
          memory[addr+2] = val >> 16;
          memory[addr+3] = val >> 24;
        }
      #else
        #define setWordAtAddr(val,addr) *(WORD *)&memory[addr] = val;
      #endif
      // System memory (even multiple of WORD size)
      BYTE memory[ROM_SIZE + RAM_SIZE + MAP_SIZE];
      #ifdef FVMO_NO_UPCASTS
        // FIXME untested
        // FIXME this is wrong
        inline WORD wordAtAddr(WORD addr) {
          return (WORD)(memory[addr] |
                  memory[addr+1] << 8 |
                  memory[addr+2] << 16 | 
                  memory[addr+3] << 24);
        }
      #else
        // Evaluates to word at specified address in system memory
        #define wordAtAddr(addr) *(WORD *)&memory[addr]
      #endif
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
          #ifdef FVMO_SDCROM
            // WARNING: returns 0 upon any kind of failure whatsoever;
            // this is a reasonable compromise for now as failure here is
            // essentially equivalent to hardware failue.
            // TODO reconsider failure handling.
            WORD buf = 0;
            if (romHandle.seek(addr) == 0) {
              // seek failed
              return 0;
            }
            if (ardReadByte(&buf,romHandle) < 1) {
              // get failed
              return 0;
            }
            return buf;
          #else
            #ifdef FVMO_NO_PROGMEM
              return prog[addr];
            #else
              return (BYTE)pgm_read_byte_far(prog+addr);
            #endif
          #endif
        } else {
          return ram[addr-ROM_SIZE];
        }
      }
      inline WORD wordAtAddr(WORD addr) {
        if (addr < ROM_SIZE) {
          #ifdef FVMO_SDCROM
            // WARNING: returns 0 upon any kind of failure whatsoever;
            // this is a reasonable compromise for now as failure here is
            // essentially equivalent to hardware failue.
            // TODO reconsider failure handling.
            WORD buf = 0;
            if (romHandle.seek(addr) == 0) {
              // seek failed
              return 0;
            }
            if (ardReadWord(&buf,romHandle) < 1) {
              // get failed
              return 0;
            }
            return buf;
          #else // #ifdef FVMO_SDCROM
            #ifdef FVMO_NO_UPCASTS
              #ifdef FVMO_NO_PROGMEM
                // FIXME untested
                WORD result, temp;
                temp = prog[addr+3];
                result = result | temp << 24;
                temp = prog[addr+2];
                result = result | temp << 16; 
                temp = prog[addr+1];
                result = result | temp << 8;
                temp = prog[addr];
                result = result | temp;
                return result;
/*
                return (WORD)(prog[addr] |
                        prog[addr+1] << 8 |
                        prog[addr+2] << 16 | 
                        prog[addr+3] << 24);
*/
              #else // #ifdef FVMO_NO_PROGMEM
                WORD result, temp;
                temp = (WORD)(pgm_read_byte_far(prog+addr+3));
                result = temp << 24;
                temp = (WORD)(pgm_read_byte_far(prog+addr+2));
                result = result | temp << 16;
                temp = (WORD)(pgm_read_byte_far(prog+addr+1));
                result = result | temp << 8;
                temp = (WORD)(pgm_read_byte_far(prog+addr));
                result = result | temp;
                return result;
/*
                return (WORD)(pgm_read_byte_far(prog+addr) |
                        pgm_read_byte_far(prog+addr+1) << 8 |
                        pgm_read_byte_far(prog+addr+2) << 16 | 
                        pgm_read_byte_far(prog+addr+3) << 24);
*/
              #endif // #ifdef FVMO_NO_PROGMEM
            #else // #ifdef FVMO_NO_UPCASTS
              #ifdef FVMO_NO_PROGMEM
                return *(WORD *)&prog[addr];
              #else
                return (WORD)pgm_read_dword_far(prog+addr);
              #endif
            #endif // #ifdef FVMO_NO_UPCASTS
          #endif // #ifdef FVMO_SDCROM
        } else { // if (addr < ROM_SIZE) {
            #ifdef FVMO_NO_UPCASTS
                WORD result, temp;
                temp = ram[addr+3-ROM_SIZE];
                result = temp << 24;
                temp = ram[addr+2-ROM_SIZE];
                result = result | temp << 16; 
                temp = ram[addr+1-ROM_SIZE];
                result = result | temp << 8;
                temp = ram[addr-ROM_SIZE];
                result = result | temp;
                return result;
/*
              return (WORD)(ram[addr-ROM_SIZE] |
                      ram[addr+1-ROM_SIZE] << 8 |
                      ram[addr+2-ROM_SIZE] << 16 | 
                      ram[addr+3-ROM_SIZE] << 24);
*/
            #else
              return *(WORD *)&ram[addr-ROM_SIZE]; 
            #endif
        }
      }
      #define setByteAtAddr(val,addr) ram[addr-ROM_SIZE] = val;
      #ifdef FVMO_NO_UPCASTS
        inline void setWordAtAddr(WORD val, WORD addr) {
          ram[addr-ROM_SIZE] = val;
          ram[addr+1-ROM_SIZE] = val >> 8;
          ram[addr+2-ROM_SIZE] = val >> 16;
          ram[addr+3-ROM_SIZE] = val >> 24;
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
  //        DEVICES  // FIXME add logic for Linux too: #if STDBLK_SIZE > 0
  // ===========================================================================
  #define stdblkFilename         "std.blk"   // Name of file for standard block
  #define romFilename            "rom.fp"    // Name of file for system ROM
  #define stdtrcFilename         "std.trc"   // Name of file for standard trace
  #define stdexpFilename         "std.exp"   // Name of file for standard export
  #define stdimpFilename         "std.imp"   // Name of file for standard import
  #ifdef FVMO_STDIN_FROM_FILE
    #define stdinFilename          "std.in" // Name of file for stdin
    #define msgTrapCantOpenStdin            "CAN'T OPEN STDIN  "
    #define msgTrapCantCloseStdin           "CAN'T CLOSE STDIN "
  #endif

  #define msgTrapCantOpenStdblk             "CAN'T OPEN STDBLK "
  #define msgTrapCantCloseStdblk            "CAN'T CLOSE STDBLK"
  #define msgTrapCantOpenRom                "CAN'T OPEN ROM    "
  #define msgTrapCantCloseRom               "CAN'T CLOSE ROM   "
  #define msgTrapCantReadRom                "CAN'T READ ROM    "
  #define msgTrapCantOpenStdexp             "CAN'T OPEN STDEXP "
  #define msgTrapCantCloseStdexp            "CAN'T CLOSE STDEXP"
  #define msgTrapCantOpenStdimp             "CAN'T OPEN STDIMP "
  #define msgTrapCantCloseStdimp            "CAN'T CLOSE STDIMP"
  // Note: declaration of romHandle, openRom, closeRom have been moved up

  // =========================================================================
  //   PRIVATE SERVICES
  // =========================================================================
  #ifdef FVMO_SD
    #if STDBLK_SIZE > 0
      SD_FILE_TYPE stdblkHandle; // File handle for stdblk file
      /* Open stdblk */
      #define openStdblk \
        stdblkHandle = SD.open(stdblkFilename, FILE_WRITE); \
        if (!stdblkHandle) { \
          goto trapCantOpenStdblk; \
        }
      /* Close stdblk */
      #define closeStdblk stdblkHandle.close();
    #endif
    SD_FILE_TYPE stdtrcHandle; // File handle for stdtrc file
    SD_FILE_TYPE stdexpHandle; // File handle for stdexp file
    SD_FILE_TYPE stdimpHandle; // File handle for stdimp file

    #ifdef FVMO_STDIN_FROM_FILE
      SD_FILE_TYPE stdinHandle; // File handle for stdin file
      /* Open stdin */
      #define openStdin \
        stdinHandle = SD.open(stdinFilename, FILE_READ); \
        if (!stdinHandle) { \
          goto trapCantOpenStdin; \
        }
      /* Close stdin */
      #define closeStdin stdinHandle.close();
    #endif

    // FIXME may need another config option here for stdtrc
    #ifdef FVMO_FVMTEST
      /* Open stdtrc for appending */ 
      #define openStdtrc \
      stdtrcHandle = SD.open(stdtrcFilename, FILE_WRITE); \
      if (!stdtrcHandle) { \
        goto trapCantOpenStdtrc; \
      }
    #else
      /* Open stdtrc for writing (not appending) */ 
      #define openStdtrc \
      if (SD.exists(stdtrcFilename)) { \
        SD.remove(stdtrcFilename); \
      } \
      stdtrcHandle = SD.open(stdtrcFilename, FILE_WRITE); \
      if (!stdtrcHandle) { \
        goto trapCantOpenStdtrc; \
      }
    #endif

    /* Close stdtrc */
    #define closeStdtrc stdtrcHandle.close();

    /* Open stdexp */
    #define openStdexp \
      stdexpHandle = SD.open(stdexpFilename, FILE_WRITE); \
      if (!stdexpHandle) { \
        goto trapCantOpenStdexp; \
      }
    /* Close stdexp */
    #define closeStdexp stdexpHandle.close();

    /* Open stdimp */
    #define openStdimp \
      stdimpHandle = SD.open(stdimpFilename, FILE_READ); \
      if (!stdimpHandle) { \
        goto trapCantOpenStdimp; \
      }
    /* Close stdimp */
    #define closeStdimp stdimpHandle.close();

  #else // #ifdef FVMO_SD
    // We have no SD card, so define these so as to make them do nothing.
    //   FIXME consider adding an EEPROM option for stdblok
    //   FIXME consider what to do about stdimp and stdexp
    #define openStdtrc
    #define closeStdtrc 
    #define openStdexp
    #define closeStdexp
    #define openStdimp
    #define closeStdimp
    // FIXME need to make all this more robust and roll out across all
    SD_FILE_TYPE stdtrcHandle = 0; // FIXME a hack for when no SD card
    int ardWrite(SD_FILE_TYPE file, char *buf, int numBytes) { // FIXME a hack for when no SD card
        #ifdef FVMO_STDTRC_SEP // FIXME could route all this through fvmTraceChar instead
          if (file == stdtrcHandle ) {
            Serial1.write(buf,numBytes);
            Serial1.flush();
          }
        #endif
       return numBytes;
    }
  #endif // #ifdef FVMO_SD 

  void clearDevices() {
    // On Linux we set all file pointers to NULL here but on Arduino here we
    // do nothing (since Arduino SD files apparently are not pointers).
  }

  // =========================================================================
  //        UTILITIES
  // =========================================================================
  // FIXME Consider here smart Arduinos with SD for stdtrc or similar
  void fvmTraceChar(const char c) {
    #ifdef FVMO_LOCAL_TAPE
        #ifdef FVMO_STDTRC_ALSO_SERIAL
        Serial.write(c);
        Serial.flush();
        #endif
        #ifdef FVMO_STDTRC_SEP
            memcom_write(&memcom1, c);
        #else
          #ifdef FVMO_MULTIPLEX
            memcom_write(&memcom, (char)STDTRC_BYTE);
            memcom_write(&memcom, c);
          #endif
        #endif
    #else
        #ifdef FVMO_STDTRC_ALSO_SERIAL // FIXME originally was for when FVMO_LOCAL_TAPE only
          Serial.write(c);
          Serial.flush();
        #endif
        #ifdef FVMO_STDTRC_SEP
            Serial1.write(c);
        #else
          #ifdef FVMO_MULTIPLEX
            Serial.write((char)STDTRC_BYTE);
            Serial.write(c);
          #endif
        #endif
        #ifdef FVMO_STDTRC_SEP
          Serial1.flush();
        #else
          Serial.flush();
        #endif
    #endif // #ifdef FVMO_LOCAL_TAPE
    #ifdef FVMO_SD
      #ifdef FVMO_STDTRC_FILE_ALSO
      if (stdtrcHandle) {
        stdtrcHandle.write(c);
        stdtrcHandle.flush();
      }
      #endif
    #endif
  }

  #ifdef FVMO_SD
    #define fvmSeek(file,pos) file.seek(pos) == 0
    // FIXME NEXT These 3 SD_FILE_TYPEs are the Energia problem
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
      #ifndef FVMO_TRON
        int numBytesWritten = file.write(buf,numBytes);
        //file.flush(); // FIXME need to try without this to speed up stdblk on SD card
        return numBytesWritten;
      #else
        // TODO refactor for better performance
        #ifdef FVMO_STDTRC_SEP
          if (strcmp(file.name(),stdtrcHandle.name()) == 0) {
            Serial1.write(buf,numBytes);
            Serial1.flush(); // Necessary for timely debugging
          }
        #endif
        #ifdef FVMO_STDTRC_FILE_ALSO
          // Write to any file here
          int numBytesWritten = file.write(buf,numBytes);
          //file.flush(); // FIXME maybe try without this?
          return numBytesWritten;
        #else
          if ( strcmp(file.name(),stdtrcHandle.name()) != 0) {
            // Write files other than stdtrc here...
            int numBytesWritten = file.write(buf,numBytes);
            // file.flush(); // FIXME need to try without this to speed up stdblk on SD card
            return numBytesWritten;
          } else {
            // ...and silently write nothing to stdfile
            return numBytes;
          }
        #endif
      #endif
    }
  #else
    #define fvmReadByte(buf,file) 0
    #define fvmReadWord(buf,file) 0
  #endif

  int ardReadByteStdin(WORD *buf) {
    #ifdef FVMO_LOCAL_TAPE
      while (memcom_available(&memcom, 1) < 1) {};
      *buf = memcom_read(&memcom);
    #else
      #ifdef FVMO_STDIN_FROM_FILE
        ardReadByte(buf,stdinHandle);
      #else
        while (Serial.available() < 1) {};
        *buf = Serial.read();
      #endif
    #endif
    return 1;
  }
  int ardReadWordStdin(WORD *buf) {
    #ifdef FVMO_LOCAL_TAPE
      while (memcom_available(&memcom, 4) < 4) {};
      WORD b1 = memcom_read(&memcom);
      WORD b2 = memcom_read(&memcom);
      WORD b3 = memcom_read(&memcom);
      WORD b4 = memcom_read(&memcom);
    #else
      #ifdef FVMO_STDIN_FROM_FILE
        ardReadByte(buf,stdinHandle); WORD b1 = *buf; *buf = 0;
        ardReadByte(buf,stdinHandle); WORD b2 = *buf; *buf = 0;
        ardReadByte(buf,stdinHandle); WORD b3 = *buf; *buf = 0;
        ardReadByte(buf,stdinHandle); WORD b4 = *buf; *buf = 0;
      #else
        while (Serial.available() < 4) {};
        WORD b1 = Serial.read();
        WORD b2 = Serial.read();
        WORD b3 = Serial.read();
        WORD b4 = Serial.read();
      #endif
    #endif
    *buf = (b4 << 24) + (b3 <<16) + (b2<<8) + b1;
    return 4;
  }

  int ardWriteStdout(char *buf, int numBytes) {
    const uint8_t *bbuf = (const uint8_t *)buf; // FIXME this line is for Energia
    #ifdef FVMO_LOCAL_TAPE
      int numBytesWritten = memcom_write(&memcom, bbuf,numBytes);
    #else
      int numBytesWritten = Serial.write(bbuf,numBytes);
    #endif
    return numBytesWritten;
  }

  #define fvmWrite(buf,unitSize,numUnits,file) ardWrite(file,(char *)buf,(unitSize*numUnits))
  #define fvmReadByteStdin(buf) ardReadByteStdin(buf)
  #define fvmReadWordStdin(buf) ardReadWordStdin(buf)
  #define fvmWriteByteStdout(buf) ardWriteStdout((char *)buf,1)
  #define fvmWriteWordStdout(buf) ardWriteStdout((char *)buf,4)

#endif // #if FVMP == FVMP_ARDUINO_IDE  // ------------------------------------

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

/*=============================================================================
  TRACING
=============================================================================*/
#ifdef FVMO_TRON

const static char mn0[] PROGMEM = "===     ";
const static char mn1[] PROGMEM = "lit     ";
const static char mn2[] PROGMEM = "call    ";
const static char mn3[] PROGMEM = "go      ";
const static char mn4[] PROGMEM = "go[>0]  ";
const static char mn5[] PROGMEM = "go[>=0] ";
const static char mn6[] PROGMEM = "go[==0] ";
const static char mn7[] PROGMEM = "go[!=0] ";
const static char mn8[] PROGMEM = "go[<=0] ";
const static char mn9[] PROGMEM = "go[<0]  ";
const static char mn10[] PROGMEM = "go[>]   ";
const static char mn11[] PROGMEM = "go[>=]  ";
const static char mn12[] PROGMEM = "go[==]  ";
const static char mn13[] PROGMEM = "go[!=]  ";
const static char mn14[] PROGMEM = "go[<=]  ";
const static char mn15[] PROGMEM = "go[<]   ";
const static char mn16[] PROGMEM = "go>0    ";
const static char mn17[] PROGMEM = "go>=0   ";
const static char mn18[] PROGMEM = "go==0   ";
const static char mn19[] PROGMEM = "go!=0   ";
const static char mn20[] PROGMEM = "go<=0   ";
const static char mn21[] PROGMEM = "go<0    ";
const static char mn22[] PROGMEM = "go>     ";
const static char mn23[] PROGMEM = "go>=    ";
const static char mn24[] PROGMEM = "go==    ";
const static char mn25[] PROGMEM = "go!=    ";
const static char mn26[] PROGMEM = "go<=    ";
const static char mn27[] PROGMEM = "go<     ";
const static char mn28[] PROGMEM = "reador  ";
const static char mn29[] PROGMEM = "writor  ";
const static char mn30[] PROGMEM = "tracor  ";
const static char mn31[] PROGMEM = "getor   ";
const static char mn32[] PROGMEM = "putor   ";
const static char mn33[] PROGMEM = "readorb ";
const static char mn34[] PROGMEM = "writorb ";
const static char mn35[] PROGMEM = "tracorb ";
const static char mn36[] PROGMEM = "getorb  ";
const static char mn37[] PROGMEM = "putorb  ";
const static char mn38[] PROGMEM = "math    ";
const static char mn39[] PROGMEM = "trap    ";
const static char mn40[] PROGMEM = "die     ";
const static char mn41[] PROGMEM = "read?   ";
const static char mn42[] PROGMEM = "write?  ";
const static char mn43[] PROGMEM = "get?    ";
const static char mn44[] PROGMEM = "put?    "; // FIXME reconsider order of new instructions
const static char mnBk[] PROGMEM = "        ";
const static char mn133[] PROGMEM = "trace?  "; // FIXME reconsider order of new instructions
const static char mn134[] PROGMEM = "zoom    ";
const static char mn135[] PROGMEM = "rchan?  ";
const static char mn136[] PROGMEM = "wchan?  ";
const static char mn137[] PROGMEM = "gchan?  ";
const static char mn138[] PROGMEM = "pchan?  ";
const static char mn139[] PROGMEM = "pc?     ";
const static char mn140[] PROGMEM = "[fly]   ";
const static char mn141[] PROGMEM = "swap2   ";
const static char mn142[] PROGMEM = "rev4    ";
const static char mn143[] PROGMEM = "tor4    ";
const static char mn144[] PROGMEM = "rot4    ";
const static char mn145[] PROGMEM = "ret     ";
const static char mn146[] PROGMEM = "invoke  ";
const static char mn147[] PROGMEM = "[invoke]";
const static char mn148[] PROGMEM = "fly     ";
const static char mn149[] PROGMEM = "swap    ";
const static char mn150[] PROGMEM = "over    ";
const static char mn151[] PROGMEM = "rot     ";
const static char mn152[] PROGMEM = "tor     ";
const static char mn153[] PROGMEM = "leap    ";
const static char mn154[] PROGMEM = "nip     ";
const static char mn155[] PROGMEM = "tuck    ";
const static char mn156[] PROGMEM = "rev     ";
const static char mn157[] PROGMEM = "rpush   ";
const static char mn158[] PROGMEM = "rpop    ";
const static char mn159[] PROGMEM = "drop    ";
const static char mn160[] PROGMEM = "drop2   ";
const static char mn161[] PROGMEM = "drop3   ";
const static char mn162[] PROGMEM = "drop4   ";
const static char mn163[] PROGMEM = "dup     ";
const static char mn164[] PROGMEM = "dup2    ";
const static char mn165[] PROGMEM = "dup3    ";
const static char mn166[] PROGMEM = "dup4    ";
const static char mn167[] PROGMEM = "hold    ";
const static char mn168[] PROGMEM = "hold2   ";
const static char mn169[] PROGMEM = "hold3   ";
const static char mn170[] PROGMEM = "hold4   ";
const static char mn171[] PROGMEM = "speek   ";
const static char mn172[] PROGMEM = "speek2  ";
const static char mn173[] PROGMEM = "speek3  ";
const static char mn174[] PROGMEM = "speek4  ";
const static char mn175[] PROGMEM = "spush   ";
const static char mn176[] PROGMEM = "spush2  ";
const static char mn177[] PROGMEM = "spush3  ";
const static char mn178[] PROGMEM = "spush4  ";
const static char mn179[] PROGMEM = "spop    ";
const static char mn180[] PROGMEM = "spop2   ";
const static char mn181[] PROGMEM = "spop3   ";
const static char mn182[] PROGMEM = "spop4   ";
const static char mn183[] PROGMEM = "dec     ";
const static char mn184[] PROGMEM = "decw    ";
const static char mn185[] PROGMEM = "dec2w   ";
const static char mn186[] PROGMEM = "inc     ";
const static char mn187[] PROGMEM = "incw    ";
const static char mn188[] PROGMEM = "inc2w   ";
const static char mn189[] PROGMEM = "@       ";
const static char mn190[] PROGMEM = "!       ";
const static char mn191[] PROGMEM = "[@]     ";
const static char mn192[] PROGMEM = "@b      ";
const static char mn193[] PROGMEM = "!b      ";
const static char mn194[] PROGMEM = "[@b]    ";
const static char mn195[] PROGMEM = "@@      ";
const static char mn196[] PROGMEM = "@!      ";
const static char mn197[] PROGMEM = "[@@]    ";
const static char mn198[] PROGMEM = "@@b     ";
const static char mn199[] PROGMEM = "@!b     ";
const static char mn200[] PROGMEM = "[@@b]   ";
const static char mn201[] PROGMEM = "+       ";
const static char mn202[] PROGMEM = "-       ";
const static char mn203[] PROGMEM = "*       ";
const static char mn204[] PROGMEM = "/       ";
const static char mn205[] PROGMEM = "%       ";
const static char mn206[] PROGMEM = "/%      ";
const static char mn207[] PROGMEM = "[+]     ";
const static char mn208[] PROGMEM = "[-]     ";
const static char mn209[] PROGMEM = "[*]     ";
const static char mn210[] PROGMEM = "[/]     ";
const static char mn211[] PROGMEM = "[%]     ";
const static char mn212[] PROGMEM = "[/%]    ";
const static char mn213[] PROGMEM = "neg     ";
const static char mn214[] PROGMEM = "abs     ";
const static char mn215[] PROGMEM = "&       ";
const static char mn216[] PROGMEM = "|       ";
const static char mn217[] PROGMEM = "^       ";
const static char mn218[] PROGMEM = "[&]     ";
const static char mn219[] PROGMEM = "[|]     ";
const static char mn220[] PROGMEM = "[^]     ";
const static char mn221[] PROGMEM = "<<      ";
const static char mn222[] PROGMEM = ">>      ";
const static char mn223[] PROGMEM = "[<<]    ";
const static char mn224[] PROGMEM = "[>>]    ";
const static char mn225[] PROGMEM = "move    ";
const static char mn226[] PROGMEM = "fill    ";
const static char mn227[] PROGMEM = "find    ";
const static char mn228[] PROGMEM = "match   ";
const static char mn229[] PROGMEM = "moveb   ";
const static char mn230[] PROGMEM = "fillb   ";
const static char mn231[] PROGMEM = "findb   ";
const static char mn232[] PROGMEM = "matchb  ";
const static char mn233[] PROGMEM = "homio   ";
const static char mn234[] PROGMEM = "rchan   ";
const static char mn235[] PROGMEM = "wchan   ";
const static char mn236[] PROGMEM = "gchan   ";
const static char mn237[] PROGMEM = "pchan   ";
const static char mn238[] PROGMEM = "ecode?  ";
const static char mn239[] PROGMEM = "rcode?  ";
const static char mn240[] PROGMEM = "rom?    ";
const static char mn241[] PROGMEM = "ram?    ";
const static char mn242[] PROGMEM = "map?    ";
const static char mn243[] PROGMEM = "stdblk? ";
const static char mn244[] PROGMEM = "ds?     ";
const static char mn245[] PROGMEM = "ss?     ";
const static char mn246[] PROGMEM = "rs?     ";
const static char mn247[] PROGMEM = "dsn?    ";
const static char mn248[] PROGMEM = "ssn?    ";
const static char mn249[] PROGMEM = "rsn?    ";
const static char mn250[] PROGMEM = "tron    ";
const static char mn251[] PROGMEM = "troff   ";
const static char mn252[] PROGMEM = "reset   ";
const static char mn253[] PROGMEM = "reboot  ";
const static char mn254[] PROGMEM = "halt    ";
const static char mn255[] PROGMEM = "data    " ;
const static char* const traceTable[] PROGMEM = {
  mn0, // Must be in same order as opcodeTable
  mn1,
  mn2,
  mn3,
  mn4,
  mn5,
  mn6,
  mn7,
  mn8,
  mn9,
  mn10,
  mn11,
  mn12,
  mn13,
  mn14,
  mn15,
  mn16,
  mn17,
  mn18,
  mn19,
  mn20,
  mn21,
  mn22,
  mn23,
  mn24,
  mn25,
  mn26,
  mn27,
  mn28,
  mn29,
  mn30,
  mn31,
  mn32,
  mn33,
  mn34,
  mn35,
  mn36,
  mn37,
  mn38,
  mn39,
  mn40,
  mn41,
  mn42,
  mn43,
  mn44,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mnBk,
  mn133,
  mn134,
  mn135,
  mn136,
  mn137,
  mn138,
  mn139,
  mn140,
  mn141,
  mn142,
  mn143,
  mn144,
  mn145,
  mn146,
  mn147,
  mn148,
  mn149,
  mn150,
  mn151,
  mn152,
  mn153,
  mn154,
  mn155,
  mn156,
  mn157,
  mn158,
  mn159,
  mn160,
  mn161,
  mn162,
  mn163,
  mn164,
  mn165,
  mn166,
  mn167,
  mn168,
  mn169,
  mn170,
  mn171,
  mn172,
  mn173,
  mn174,
  mn175,
  mn176,
  mn177,
  mn178,
  mn179,
  mn180,
  mn181,
  mn182,
  mn183,
  mn184,
  mn185,
  mn186,
  mn187,
  mn188,
  mn189,
  mn190,
  mn191,
  mn192,
  mn193,
  mn194,
  mn195,
  mn196,
  mn197,
  mn198,
  mn199,
  mn200,
  mn201,
  mn202,
  mn203,
  mn204,
  mn205,
  mn206,
  mn207,
  mn208,
  mn209,
  mn210,
  mn211,
  mn212,
  mn213,
  mn214,
  mn215,
  mn216,
  mn217,
  mn218,
  mn219,
  mn220,
  mn221,
  mn222,
  mn223,
  mn224,
  mn225,
  mn226,
  mn227,
  mn228,
  mn229,
  mn230,
  mn231,
  mn232,
  mn233,
  mn234,
  mn235,
  mn236,
  mn237,
  mn238,
  mn239,
  mn240,
  mn241,
  mn242,
  mn243,
  mn244,
  mn245,
  mn246,
  mn247,
  mn248,
  mn249,
  mn250,
  mn251,
  mn252,
  mn253,
  mn254,
  mn255
};


  const char fvmhex[0x10] PROGMEM = {'0','1','2','3','4','5','6','7','8','9',
                      'a','b','c','d','e','f'};

  void fvmTraceNewline() {
    fvmTraceChar('\r');
    fvmTraceChar('\n');
  }

  #ifdef FVMO_NO_PROGMEM
    /* For tracing: print a message up to 256 characters long */
    void fvmTrace(const char *msg) {
      BYTE i = 0;
      while ((i <= BYTE_MAX) && (msg[i] != 0)) {
        fvmTraceChar(msg[i]);
        i++;
      }
    }
  #else
    /* 
       For tracing: print a message from PROGMEM up to 256 characters long.
       IMPORTANT NOTE: when not in FVMO_NO_PROGMEM mode, absolutely all
       strings passed into fvmTrace must reside in PROGMEM!
       This will NOT work:

          fvmTrace("some string"); 

       But this will work:

          const static char someStr[] PROGMEM = "some string";
          fvmTrace(someStr); 

    */
    void fvmTrace(const char *msg) {
      BYTE i = 0;
      char c;
      while (i <= BYTE_MAX) {
        c = pgm_read_byte_far(msg+i);
        if (c == 0) {
          break;
        }
        fvmTraceChar(c);
        i++;
      }
    }
  #endif

  /* For tracing: print a byte in hexadecimal */
  void fvmTraceByteHex(BYTE b) {
    BYTE h = (b >> 4) & 0x0f;
    char c;
    #ifdef FVMO_NO_PROGMEM
      c = fvmhex[h];
    #else
      c = pgm_read_byte_far(fvmhex+h);
    #endif
    fvmTraceChar(c);
    h = b & 0x0f;
    #ifdef FVMO_NO_PROGMEM
      c = fvmhex[h];
    #else
      c = pgm_read_byte_far(fvmhex+h);
    #endif
    fvmTraceChar(c);
  }

  /* For tracing: print a byte in hexadecimal */
  void fvmTraceWordHex(WORD n) { /* TODO make generic */
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
    char c;
    while (i < 8) {
    #ifdef FVMO_NO_PROGMEM
      c = msg[i];
    #else
      c = pgm_read_byte_far(msg+i);
    #endif
      if (c == 0) {
        break;
      }
      fvmTraceChar(c);
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
  void traceInfo(WORD pc)
  {
    WORD opcode = wordAtPc
    if (opcode >= 0 && opcode <=256) {
      fvmTraceWordHex(pc);
      fvmTraceChar(' ');
      const char* pStr;
      #ifdef FVMO_NO_PROGMEM
        pStr = traceTable[opcode];
      #else
        pStr = (char *)pgm_read_dword_far(&(traceTable[opcode]));
      #endif
      fvmTraceMnemonic(pStr);
    } else {
      fvmTraceWordHex(pc);
      const static char str6[] PROGMEM = "  ";
      fvmTrace(str6);
    }
    if (opcode < LOWEST_SIMPLE_OPCODE && pc
        < (HIGHEST_WRITABLE_WORD - WORD_SIZE)) {
      WORD cellValue = wordAtAddr(pc+WORD_SIZE);
      fvmTraceWordHex(cellValue);
      fvmTraceChar(' ');
    } else {
      const static char str7[] PROGMEM = "         ";
      fvmTrace(str7);
    }
  }

  void traceStacks() {
    const static char str8[] PROGMEM = "( ";
    fvmTrace(str8);
    int i = 1;
    int numElems = (DS_EMPTY-dsp);
    for (; i<=numElems; i++) {
      fvmTraceWordHex(ds[dsStop-i]);
      fvmTraceChar(' '); 
    }
    const static char str9[] PROGMEM = ") ";
    fvmTrace(str9);
    const static char str10[] PROGMEM = "[ ";
    fvmTrace(str10);
    i = 1;
    numElems = (SS_EMPTY-ssp);
    for (; i<=numElems; i++) {
      fvmTraceWordHex(ss[ssStop-i]);
      fvmTraceChar(' '); 
    }
    const static char str11[] PROGMEM = "] ";
    fvmTrace(str11);
    const static char str12[] PROGMEM = "{ ";
    fvmTrace(str12);
    i = 1;
    numElems = (RS_EMPTY-rsp);
    for (; i<=numElems; i++) {
      fvmTraceWordHex(rs[rsStop-i]);
      fvmTraceChar(' '); 
    } 
    fvmTraceChar('}');
    fvmTraceNewline();
  }

#endif

// ===========================================================================
//                              EXIT TRACING
// ===========================================================================
// Send an error message to stdtrc
// along with information regarding current program state.
#ifdef FVMO_TRON
  const static char STR_AT[] PROGMEM = " at ";

  void traceExit(WORD pc, const char *msg) {
    fvmTrace(msg);
    fvmTrace(STR_AT);
    fvmTraceWordHex(lastTrapAddress);
    fvmTraceNewline();
    traceInfo(pc);
    traceStacks();
  }
  void traceExitMsg(const char *msg) {
    fvmTrace(msg);
    fvmTrace(STR_AT);
    fvmTraceWordHex(lastTrapAddress); 
    fvmTraceNewline();
  }
#else
  void traceExit(WORD pc, const char *msg) {}
  void traceExitMsg(const char *msg) {}
#endif // .ifdef FVMO_TRON

// ===========================================================================
//                          FVM RUNTIME LOGIC
//       In C this has to be packaged into a runfvm() function
//              to allow use of goto and labels for speed  
// ===========================================================================
int runfvm() {

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
    traceInfo(pc); \
    traceStacks(); \
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
    openStdtrc
    #if STDBLK_SIZE > 0
      openStdblk
    #endif
    openStdexp
    openStdimp

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
  #ifdef FVMO_SD
    openStdtrc
    #if STDBLK_SIZE > 0
      openStdblk
    #endif
    openStdexp
    openStdimp
    #ifdef FVMO_STDIN_FROM_FILE
      const static char strx[] PROGMEM = "openStdin..."; // FIXME DELETEME
      fvmTrace(strx); fvmTraceNewline();
      openStdin
      const static char stry[] PROGMEM = "Did openStdin"; // FIXME DELETEME
      fvmTrace(stry); fvmTraceNewline();
    #endif
  #endif

  #ifndef FVMO_INCORPORATE_ROM
    openRom

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
      case iREADOR:
        readBuf = 0; // zero the buffer
        switch(rchannel) {
          case STDIN:
            if (fvmReadWordStdin(&readBuf) < 1) {
              branch // read failed
              break;
            }
            rA = readBuf;
#ifdef FVMO_MULTIPLEX
            // WARNING: input buffering would need to be added for this
            // to be of any real use for true multiplexing. This currently
            // can only work reliably because stdin is the only input
            // stream that this 'fvm.c' 'multiplexes'. Also note that here
            // in iREADOR we are only using byte-level multiplexing which
            // is inefficient compared to word-level multiplexing but
            // probably has to be used for reasons of practicality;
            // thus rendering word-level multiplexing redundant.

            // FIXME untested:
            if (readBuf != STDIN_BYTE) { branch break; }
            if (fvmReadByteStdin(&readBuf) < 1) { branch break; }
            rA = readBuf;
            if (readBuf != STDIN_BYTE) { branch break; }
            if (fvmReadByteStdin(&readBuf) < 1) { branch break; }
            rA = rA << 8; rA = rA & (readBuf & 0x000000ff);
            if (readBuf != STDIN_BYTE) { branch break; }
            if (fvmReadByteStdin(&readBuf) < 1) { branch break; }
            rA = rA << 8; rA = rA & (readBuf & 0x000000ff);
            if (readBuf != STDIN_BYTE) { branch break; }
            if (fvmReadByteStdin(&readBuf) < 1) { branch break; }
            rA = rA << 8; rA = rA & (readBuf & 0x000000ff);
#endif
            pushDs
            dontBranch
            break;
          break;
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
            // WARNING: input buffering would need to be added for this
            // to be of any real use for true multiplexing. This currently
            // only works reliably because stdin is the only input
            // stream that this 'fvm.c' 'multiplexes'.
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
          case STDIMP:
            if (fvmReadByte(&readBuf,stdimpHandle) < 1) {
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
#ifdef FVMO_MULTIPLEX
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
#ifdef FVMO_STDEXP // FIXME roll these out across all relevant
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
#endif
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
#ifdef FVMO_STDEXP
          case STDEXP:
            popDs
            writeBuf = rA;
            if (fvmWrite(&writeBuf,1,1,stdexpHandle) < 1) {
              branch  // write failed
            } else {
              dontBranch
              break;
            }
#endif
          default:
            branch // Unsupported wchannel
            break;
        }
        break;
      case iTRACOR:
        popDs
#ifdef FVMO_MULTIPLEX
          // FIXME untested
          writeBuf = STDTRC_BYTE;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }
          writeBuf = rA;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }

          writeBuf = STDTRC_BYTE;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }
          rA = rA >> 8; writeBuf = rA;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }

          writeBuf = STDTRC_BYTE;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }
          rA = rA >> 8; writeBuf = rA;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }

          writeBuf = STDTRC_BYTE;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }
          rA = rA >> 8; writeBuf = rA;
          if (fvmWrite(&writeBuf,1,1,stdtrcHandle) < 1) { branch break; }
          dontBranch
#else
          writeBuf = rA;
          if (fvmWrite(&writeBuf,WORD_SIZE,1,stdtrcHandle) < 1) {
            branch // trace failed
            break;
          } else {
            dontBranch
            break;
          }
#endif
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
          WORD i = 0;
          WORD addr;
          WORD result = -1; // -1 means not found
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
          WORD i = 0;
          WORD addr;
          WORD result = -1; // -1 means not found
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
          WORD i = 0;
          WORD addr;
          WORD result = -1; // -1 means not found
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
          WORD i = 0;
          WORD addr;
          WORD result = -1; // -1 means not found
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
            WORD i = 0;
            WORD addr;
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
            WORD i = 0;
            WORD addr;
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
          WORD i = 0;
          WORD w1addr;
          WORD w2addr;
          WORD result = TRUE;
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
          WORD i = 0;
          WORD b1addr;
          WORD b2addr;
          WORD result = TRUE;
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
          WORD i = 0;
          WORD b1addr;
          WORD b2addr;
          for (; i<rA; i++) {
            b1addr = rB+i;
            b2addr = rC+i;
            setByteAtAddr(byteAtAddr(b1addr), b2addr)
          }
          break;
        } else {
          // numBytes is negative, do descending move
          WORD i = 0;
          WORD b1addr;
          WORD b2addr;
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
          WORD i = 0;
          WORD w1addr;
          WORD w2addr;
          for (; i<rA; i++) {
            w1addr = rB+(i*WORD_SIZE);
            w2addr = rC+(i*WORD_SIZE);
            setWordAtAddr(wordAtAddr(w1addr), w2addr);
          }
          break;
        } else {
          // numWords is negative, do descending move
          WORD i = 0;
          WORD w1addr;
          WORD w2addr;
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
            if ((rA != 0) && (rA - NEG_INT_MAX) < rB) {
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
  #ifndef FVMO_INCORPORATE_ROM
    #ifdef FVMO_SDCROM
      if (romHandle) { closeRom }
    #endif
  #endif
  #if STDBLK_SIZE > 0
    if (stdblkHandle) { closeStdblk }
  #endif
  if (stdexpHandle) { closeStdexp }
  if (stdimpHandle) { closeStdimp }
  if (stdtrcHandle) { closeStdtrc }
  #ifdef FVMO_STDIN_FROM_FILE
    if (stdinHandle) { closeStdin }
  #endif
#else
  #if FVMP == FVMP_STDIO
    #if STDBLK_SIZE > 0
    if (stdblkHandle) { closeStdblk }
    #endif
    if (stdexpHandle) { closeStdexp }
    if (stdimpHandle) { closeStdimp }
    if (stdtrcHandle) { closeStdtrc }
  #endif
#endif

#ifdef FVMO_FVMTEST
// The next line should be uncommented for fvm16-16MB-sr-append for fvmtest
   goto systemSoftReset;    // Uncomment for soft reset
#else
// The next line should normally be the uncommented one for most FVMs
   goto exitFail;           // Uncomment for exit with specific failure code
// goto exitFailGeneric;    // Uncomment for exit with generic failure code
// goto systemHardReset;    // Uncomment for hard reset
#endif

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

/* FIXME Actually, if we arrive at systemExit without first going through
   systemReset, which indeed we do if we jumped to exitSuccess or exitFail,
   then any open files will not be explicitly closed prior to the end of the
   run of the FVM instance. On Linux this does not seem to matter since
   the operating system presumably closes them when the process
   ends but on bare metal this might be a problem. However, since this
   behaviour has existed since FVM 1.0 without causing any problems,
   fixing it is a low priority for now. */
systemExit:                   // Exit using exitCode in rB
  return rB;                  // Return from runfvm()

// ===========================================================================
//                                TRAPS
// ===========================================================================
trap:
  lastTrapAddress = safePreviousAddress(pc);
  traceExit(pc,msgTrap);
  goto systemReset;
die:
  lastTrapAddress = safePreviousAddress(pc);
  traceExit(pc,msgDied);
  goto systemExit;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL PROGRAM FLOW
//----------------------------------------------------------------------------
trapWall:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 2;
  traceExit(pc,msgTrapWall);
  goto systemReset;
trapData:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 3;
  traceExit(pc,msgTrapData);
  goto systemReset;
trapPcOverflow:
  lastTrapAddress = pc;
  rB = 4;
  traceExitMsg(msgTrapPcOverflow);
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL OPCODES
//----------------------------------------------------------------------------
iNONE:
trapIllegalOpcode:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 11;
  traceExit(pc,msgTrapIllegalOpcode);
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
  traceExit(pc,msgTrapMathOverflow);
  goto systemReset;
trapDivideByZero:
  if (iOntrapAddress != NONE) {
    branchFromMath
    goto nextInstruction;
  }
  lastTrapAddress = safePreviousAddress(pc);
  rB = 22;
  traceExit(pc,msgTrapDivideByZero);
  goto systemReset;
trapXsBitshift:
  if (iOntrapAddress != NONE) {
    branchFromMath
    goto nextInstruction;
  }
  lastTrapAddress = safePreviousAddress(pc);
  rB = 23;
traceExit(pc,msgTrapXsBitshift);
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL STACK OPERATIONS
//----------------------------------------------------------------------------
trapDsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 31;
  traceExit(pc,msgTrapDsUnderflow);
  goto systemReset;
trapDsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 32;
  traceExit(pc,msgTrapDsOverflow);
  goto systemReset;
trapRsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 33;
  traceExit(pc,msgTrapRsUnderflow);
  goto systemReset;
trapRsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 34;
  traceExit(pc,msgTrapRsOverflow);
  goto systemReset;
trapSsUnderflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 35;
  traceExit(pc,msgTrapSsUnderflow);
  goto systemReset;
trapSsOverflow:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 36;
  traceExit(pc,msgTrapSsOverflow);
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ILLEGAL MEMORY ACCESS
//----------------------------------------------------------------------------
trapMemBounds:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 41;
  traceExitMsg(msgTrapMemBounds);
  goto systemReset;
trapRAMBounds:
  lastTrapAddress = safePreviousAddress(pc);
  rB = 42;
  traceExitMsg(msgTrapRAMBounds);
  goto systemReset;
//----------------------------------------------------------------------------
//                         TRAPS: ROM
//----------------------------------------------------------------------------
//Note: a ROM file ('rom.fp') can be created using a Freelang compiler
trapCantOpenRom:
  rB = 51;
  traceExitMsg(msgTrapCantOpenRom);
  goto exitFail;
trapCantCloseRom:
  rB = 52;
  traceExitMsg(msgTrapCantCloseRom);
  goto exitFail;
trapCantReadRom:
  rB = 53;
  traceExitMsg(msgTrapCantReadRom);
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
  traceExitMsg(msgTrapCantOpenStdblk);
  goto exitFail;
trapCantCloseStdblk:
  rB = 62;
  traceExitMsg(msgTrapCantCloseStdblk);
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
  traceExitMsg(msgTrapCantOpenStdexp);
  goto exitFail;
trapCantCloseStdexp:
  rB = 75;
  traceExitMsg(msgTrapCantCloseStdexp);
  goto exitFail;

//Note: to create a 'std.imp' file of 0 size on Linux simply use:
//           touch std.imp
trapCantOpenStdimp:
  rB = 77;
  traceExitMsg(msgTrapCantOpenStdimp);
  goto exitFail;
trapCantCloseStdimp:
  rB = 78;
  traceExitMsg(msgTrapCantCloseStdimp);
  goto exitFail;

#ifdef FVMO_STDIN_FROM_FILE
trapCantOpenStdin:
  rB = 81;
  traceExitMsg(msgTrapCantOpenStdin);
  goto exitFail;
trapCantCloseStdin:
  rB = 82;
  traceExitMsg(msgTrapCantCloseStdin);
  goto exitFail;
#endif
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
  /* Note that none of the diagnostic messages in this function will
     appear in 'std.trc' on an SD card, because they precede the running
     of the FVM itself. They can however be seen over serial if
     configuration is appropriate for that. */
  void setup() {
    #ifdef FVMO_LOCAL_TAPE
      #ifdef FVMO_STDTRC_ALSO_SERIAL
        Serial.begin(BAUD_RATE);
        while (!Serial) {}
      #endif
      #ifdef FVMO_STDTRC_SEP
        memcom_begin(&memcom1, BAUD_RATE);
      #endif
        memcom_begin(&memcom, BAUD_RATE);
    #else
        Serial.begin(BAUD_RATE);
        while (!Serial) {}    
      #ifdef FVMO_STDTRC_SEP
        Serial1.begin(BAUD_RATE_STDTRC);
        while (!Serial1) {}
      #endif
    #endif // #ifdef FVMO_LOCAL_TAPE

    #ifdef FVMO_TRON
      fvmTraceNewline();
    #endif

    #ifdef FVMO_SD
      // Only some boards require the next 2 lines but apparently
      // they can be used whether or not the board actually requires them
      // (see note, far above, regarding hardware SS pin).
      pinMode(FVMO_SD_CS_PIN, OUTPUT);
      digitalWrite(FVMO_SD_CS_PIN, HIGH);

      if (!SD.begin(FVMO_SD_CS_PIN)) {
        #ifdef FVMO_TRON
          const static char str13[] PROGMEM = "SD card FAIL";
          fvmTraceNewline();
          fvmTrace(str13);
        #endif
        // FIXME probably should bail out here so we do not start FVM
      } else {
        #ifdef FVMO_TRON
          const static char str14[] PROGMEM = "SD card OK";
          fvmTrace(str14);
          fvmTraceNewline();
        #endif
      }
    #endif

    #ifdef FVMO_LOCAL_TAPE
      #ifdef FVMO_TRON
        const static char str1[] PROGMEM = "Init tape...";
        fvmTrace(str1);
        fvmTraceNewline();
      #endif
        bool initialized = tape_local_init();
        if (initialized) {
        #ifdef FVMO_TRON
          const static char str2[] PROGMEM = "Tape OK";
          fvmTrace(str2);
          fvmTraceNewline();
        #endif
        } else {
          tape_local_shutdown();
        #ifdef FVMO_TRON
          const static char str3[] PROGMEM = "Tape FAIL";
          fvmTrace(str3);
          fvmTraceNewline();
        #endif
          // FIXME probably should bail out here so we do not start FVM
        }
    #endif

    #ifdef FVMO_TRON
      fvmTraceNewline();
      const static char str4[] PROGMEM = "Run FVM...";
      fvmTrace(str4);
      fvmTraceNewline();
    #endif
  }

  void loop() {
    int theExitCode = runfvm();
  #ifdef FVMO_TRON
    const static char str5[] PROGMEM = "FVM exit code: ";
    fvmTrace(str5);
    fvmTraceWordHex(theExitCode);
    fvmTraceNewline();
  #endif
    // If any of the devices started in setup() need graceful shutdown
    // then that would be done here. The current implementation does
    // not require it. Note: this is NOT the place to close any
    // open files, that should be done at systemReset: and/or systemExit:
    // and there is a code comment at systemExit: regarding that. 
    while(true);
  }
#endif // #if FVMP == FVMP_ARDUINO_IDE // -------------------------------------
// ============================================================================

