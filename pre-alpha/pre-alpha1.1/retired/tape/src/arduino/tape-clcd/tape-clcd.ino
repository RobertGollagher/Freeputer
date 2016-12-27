/*

Program:    tape-clcd.ino
Copyright © Robert Gollagher 2016
Author :    Robert Gollagher   robert.gollagher@freeputer.net
Created:    2016
Updated:    20130324:2249
Version:    pre-alpha-0.0.0.5

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

==============================================================================

WARNING: This is pre-alpha software and as such may well be incomplete,
unstable and unreliable. It is considered to be suitable only for
experimentation and nothing more.

==============================================================================

The tape is a textual user interface (TUI) as described in tape/README.md.

This implementation:

  * is for 5-Volt Arduino boards (e.g. Uno or Mega 2560),
    used as a physical tape terminal with keyboard and display,
    using serial communication to connect to a tape server (such as
    an FVM running 'ts.fl' on another Arduino or on a Linux computer)
  * uses a PS/2 keyboard via the PS2KeyAdvanced and PS2KeyMap libraries
  * uses a character LCD display (abbreviated here as CLCD hence 'tape-clcd.ino')
    which has 40x4 columns x rows with a HD44780-compatible interface with
    two enable lines (Winstar WH4004A-YYH-JT LCD display module) and therefore
    requires the LiquidCrystalFast library (from www.pjrc.com) rather
    than the ordinary Arduino LiquidCrystal library
  * or can use smaller CLCDs (such as 16x2 or 20x4) in SINGLE_ENABLE_PIN mode
    and this uses the ordinary Arduino LiquidCrystal library
  * supports 7-bit ASCII display only (not proper UTF-8 display)
  * supports multiplexing (an optional feature of FVM 1.1)
  * supports split-tape tracing (stdtrc display in second half of tape)

WARNING: When running a 40x4 CLCD on an Arduino Mega 2560 and it is
in MULTIPLEX mode, it is necessary to use TAPE_VERY_SLOW_BAUD otherwise
unfortunately such a slow tape terminal cannot handle multiplexing at all.
It is better to simply not use MULTIPLEX and not use TAPE_SPLIT_TRC
when running this sketch on such slow hardware, since even such
slow hardware works very well when not in MULTIPLEX mode.

To build use the Arduino IDE 1.6.7 or higher.

To connect to a physical Freeputer instance (such as on another Arduino)
simply connect the RX pin of this tape terminal board to the TX pin of the
tape server board and vice versa; to do this you must use a logic level
converter if the boards do not both run at the same voltage. You must
also connect a GND pin of one board to a GND pin of the other board
so that the boards share a common ground. Disconnect all of these
connections when programming the boards using the Arduino IDE.

Note: to create such a physical Freeputer tape server instance on another Arduino
you would use 'fvm.c' for FVM 1.1 (see instructions therein) and build that
using the Arduino IDE. You would of course first use the 'romMake.sh' script
to compile the 'ts.fl' Freelang program into a 'rom.h' header file
that would then be incorporated into the FVM executable built
by the Arduino IDE (see 'fvm.c', 'romMake.sh', 'ts.fl'). 

To connect to a virtual Freeputer instance (such as on a Linux computer)
simply use the serial connection built into the USB cable of your Arduino board
in the normal manner. For example, this might be recognized by Linux as serial
device /dev/ttyACM0. However, you must first ensure that Linux knows the
configuration of the serial device. That can be achieved either by
connecting to /dev/ttyACM0 at least once with the serial monitor in the
Arduino IDE set to 115200 baud or by briefly connecting at least
once with minicom thus:

  minicom -D /dev/ttyACM0 -c on -b 115200

Then in Linux you can connect your FVM instance to the tape terminal by:

  ./fvm < /dev/ttyACM0 > /dev/ttyACM0

For the tape to behave meaningfully, there must be an application running
on the (physical or virtual) FVM instance and that application must be written
to control a tape. A simple example of such a Freeputer application is
the Freelang program 'ts.fl' (short for Tape Server) which selectively and
appropriately echos characters typed into the tape so
that they appear on the tape while respecting escape sequences
(which for a tape, unlike an ANSI terminal, begin with the DLE character
not the ESC character). Actually the application controlling the tape
via the aforementioned device does not necessary have to be a Freeputer
application; it could be any kind of application that appropriately
handles its stdin and stdout streams so as to control a tape.

This 'tape-clcd.ino' implementation of the tape happens to communicate via
serial connection and accepts keyboard input; therefore it can be properly
described as a 'tape terminal'. A tape does not necessarily have to use
serial communication and does not necessarily have to accept input
(some tapes might be only for display not input). However, the most
common kind of tape implementation is a tape terminal since such
an implementation has many uses and favours modular rather than
monolithic system design (rather than building a display
into a standalone Freeputer instance).

Since this implementation uses a rectangular screen,
the tape is wrapped into a 2D geometry and therefore this
implementation tracks things such as row and column. It is important to
remember that the tape itself has no concept of multi-dimensional geometry,
it is just a sequence of cells. All matters of geometry are private to
and internal to an implementation and have nothing to do with the
logical nature of the tape.

You will also note there is an option to use multiplexing.
In this context, multiplexing refers to a new optional feature in some
FVM 1.1 implementations that uses a simple multiplexing scheme to allow
a single physical serial connection to be used to support not only
the Freeputer stdin and stdout streams but also stdtrc (the Freeputer
output stream on which tracing output can be sent from an FVM instance
as an aid to debugging a Freeputer application). In this simple
multiplexing scheme each data byte is preceded by a byte identifying
the stream to which that data byte belongs. This 'tape-clcd.ino' supports
that multiplexing and indeed you must have it enabled if you wish to see
stdtrc output in the second half of the tape when running in
split-tape mode (entered by pressing Ctrl-T) unless you wish instead to
use a second physical serial connection for stdtrc (see TAPE_STDTRC_SEP).

If the tape appears to be unresponsive and displays an exclamation mark
this means an error has occurred; this will occur if you leave MULTIPLEX
defined yet the FVM you are connected to is not using multiplexing,
or if for any other reason the tape is sent garbage input that
cannot be meaningfully decoded.

If implementing your own tape on some other platform, it is important
to understand that the fields of the tape struct in this 'tape-clcd.ino'
are particular to this implementation. The only fields of that
struct that belong to the logical nature of a tape are pos
(the current cell), maxPos (the length of the tape),
and arguably maybe some of the mode information.

Important: stdtrc output generally does not support UTF-8.
Only send 7-bit ASCII characters to stdtrc. Furthermore, this 'tape-clcd.ino'
does not support the proper display of UTF-8 characters anywhere on the
tape; at best they will be displayed as unknown-character symbols.

==============================================================================

CHARACTER DISPLAY: since this tape uses a monochrome display device, it is
sometimes impossible to tell the difference between certain special characters
(e.g. CR, LF, HT) which are all displayed as the same character (e.g. '*')
but which would, on a colour device, be differentiated by colour.
This means, for example, that you cannot tell the difference between
a real exclamation mark and one that indicates the approximate location
of corrupt data in the stream being displayed. This problem may be
solved in future by the use of custom characters but for now
this is a known issue with this monochrome tape.

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

============================================================================== */
#include <stdbool.h>

/* GENERAL CONFIGURATION OPTIONS FOR THE TAPE TERMINAL: */
#define CHAR_TYPE unsigned char // Don't change this (other types unsupported)
//#define SUPPORT_UTF8 // Don't uncomment this (UTF-8 display not implemented)
#define TAPE_SPLIT_TRC // Use split-tape mode (stdtrc display)
//#define MULTIPLEX // Use FVM 1.1 multiplexing
//#define TAPE_SLOW_BAUD // Use slow baud rate when multiplexing
//#define TAPE_VERY_SLOW_BAUD // Use very slow baud rate when multiplexing
#define TAPE_STDTRC_SEP // Use second serial connection for stdtrc

// Experimental (see FVMO_LOCAL_TAPE in 'fvm.c')
#ifdef FVMO_LOCAL_TAPE
  #define TAPE_LOCAL_TAPE
#endif
// ============================================================================

#ifdef TAPE_STDTRC_SEP 
  #define BAUD_RATE_STDTRC 38400 // Baud rate for stdtrc serial connection
#endif
#ifdef MULTIPLEX  //   (note: an FVM 1.1 may or may not support mutliplexing)
  #ifdef TAPE_SLOW_BAUD
    #define BAUD_RATE 4800
  #endif
  #ifndef BAUD_RATE
    #ifdef TAPE_VERY_SLOW_BAUD
      #define BAUD_RATE 2400
    #else
      #define BAUD_RATE 115200
    #endif
  #endif
  #define STDIN_BYTE  0b00000001
  #define STDOUT_BYTE 0b01000001
  #define STDTRC_BYTE 0b01000000
  #define UNKNOWN_DEVICE_BYTE 0b01111111
  CHAR_TYPE multiRouteIn;
  int multiRouteInKnown;
#else
  #define BAUD_RATE 115200
#endif
#define DLE_LENGTH 6
#define SERVER_DLE_LEN 6
#define KEY_DLE_LEN 6
typedef struct sequence {
  CHAR_TYPE index;
  CHAR_TYPE seq[SERVER_DLE_LEN];
} sequence_t;
typedef struct tape {
  int pos; // Current cell (starts at 0)
  int maxPos; // Highest cell (not including split-tape mode stdtrc display)
  int lastPos; // Most recent cell
  int col; // Currrent column
  int row; // Current row
  int maxX; // Number of columns
  int maxY; // Number of rows
  bool wrap; // In wrap mode? (wrap = overwrite; no wrap = stops writing)
  bool over; // When in wrap mode, have we wrapped (that is, overwritten)?
  sequence_t dleseq; // Current escape sequence (from server)
  bool serverReady; // Is the server ready to receive another character?
  char utf8buf[5]; // Zero-terminated buffer for input of a UTF-8 character
  int iUtf8buf; // Current index in the utf8buf (0 to 3)
  int lenUtf8char; // Byte length of current UTF-8 character (1 to 4)
  bool splitMode; // In split-tape mode? (allows stdtrc display)
  int maxPos2; // Highest stdtrc cell (only used in split-tape mode)
  int pos2; // Current stdtrc cell (only used in split-tape mode)
  bool visibleSpc; // Show spaces as visible characters?
} tape_t;

#ifndef TAPE_LOCAL_TAPE
  bool tape_run(tape_t *t);
#endif
bool tape_local_init();
bool tape_init(tape_t *t);
bool tape_putc(tape_t *t, CHAR_TYPE c);
bool tape_goto(tape_t *t, int pos);
bool tape_clear(tape_t *t);
bool tape_clearFrom(tape_t *t, int posAfter);
bool tape_local_shutdown();
bool tape_shutdown(tape_t *t);
static void quit(int sig);
int ttrace(tape_t *t, CHAR_TYPE c);
void handleOver(tape_t *t);
void notOver(tape_t *t);

#define NUM_DLE_SIMPLE_INSTRS 12
#define NUM_DLE_COMPLEX_INSTRS 1

#define CMD_DLE_CLEAR 1
#define CMD_DLE_CLEAR_FROM 2
#define CMD_DLE_NO_WRAP 3
#define CMD_DLE_WRAP 4
#define CMD_DLE_RIGHT 5
#define CMD_DLE_LEFT 6
#define CMD_DLE_UP 7
#define CMD_DLE_DOWN 8
#define CMD_DLE_REPORT_POS 9
#define CMD_DLE_REPORT_MAX_POS 10
#define CMD_DLE_SPLIT_MODE 11
#define CMD_DLE_VISIBLE_SPC 12
#define CMD_DLE_GOTO_POS 128

#define CHAR_HT 9 // Horizontal tab (an invisible character)
#define CHAR_NL 10 // Newline (an invisible character)
#define CHAR_CR 13 // Return (an invisible character)
#define CHAR_EM 25 // End of medium (indicates tape full when not in wrap mode)
#define CHAR_SPC 32 // Space
#define CHAR_VISIBLE_SPC '.' // Character to show to make spaces visible
#define CHAR_INVIS '*' // Character to show for invisible characters (not space)

#define CHAR_DLE 16 // Data Link Escape character (used for escape sequences)
#define CHAR_XON 17 // = DC1 (used by server to indicate ready)
#define CHAR_XOFF 19 // = DC3 (used by server to indicate not ready)

#define KEY_CLEARFROM 0x204b // Ctrl-k  Clear from cursor to end of tape
#define KEY_CLEAR 0x204c // Ctrl-l Clear entire tape
#define KEY_WRAPON 0x2057 // Ctrl-w Turn on wrap mode
#define KEY_WRAPOFF 0x2052 // Ctrl-r Turn off wrap mode
#define KEY_SPLIT_TGL 0x2054 // Ctrl-t Toggle split-tape mode (stdtrc display)
#define KEY_RIGHT 0x0116 // These are status bits 0x01 with code byte 0x16 etc...
#define KEY_LEFT 0x0115
#define KEY_UP 0x0117
#define KEY_DOWN 0x0118
#define KEY_VSPC_TGL 0x2055 // Ctrl-u Toggle visible spaces

typedef enum { ORIGIN_LOCAL, ORIGIN_SERVER, ORIGIN_SERVER_STDTRC } origin;
typedef struct {
  origin o;
  CHAR_TYPE c;
  uint16_t rawKeycode;
} ochar;

tape_t currentTape = {0};
void addchar(char c, tape_t *t, int atPos);

// ============================================================================
/* Important pin configuration: */
#define SINGLE_ENABLE_PIN // Using a small CLCD with only 1 enable pin

// ============================================================================
/* Configuration for PS/2 keyboard: */
#include <PS2KeyAdvanced.h>
#include <PS2KeyMap.h>
#ifndef SINGLE_ENABLE_PIN
  #define DATAPIN 7
  #define IRQPIN  3
#else
  #define DATAPIN 11
  #define IRQPIN  2
#endif

PS2KeyAdvanced keyboard;
PS2KeyMap keymap;
uint16_t keycode;

// ============================================================================
/* Configuration for CLCD: */

#ifndef SINGLE_ENABLE_PIN
  /* Must be used for displays larger than 80 characters total;
     that is, which require more than 1 enable line. */
  #include <LiquidCrystalFast.h>

  /* 40x4 display (Winstar WH4004A-YYH-JT LCD display module)
     WARNING: follow data sheet instructions for circuit,
     at least 1 resistor is needed (but not on the pins listed below).

              LCD pins: RS  RW  E1  E2  D4  D5  D6  D7 */
  LiquidCrystalFast lcd(12, 10, 11, 9,  5,  4,  6,  2);
  #define COLS 40
  #define ROWS 4

#else
  /* Cannot be used for displays larger than 80 characters total;
     that is, which require more than 1 enable line. */
  #include <LiquidCrystal.h>

  /* 16x2 display (Freetronics LCD Shield)
     WARNING: check data sheet for your display as you may need
     a different circuit to the one noted below!
     Note: no resistors are needed when using the Freetronics LCD Shield.
     However, rather than affixing the shield to your Arduino,
     you must connect the pins using jumper wires,
     so that you still have access to the pins needed to
     also connect to the PS/2 keyboard adaptor.

              LCD pins: RS, EN, D4, D5, D6, D7 *//* 
  LiquidCrystal     lcd(8,  9,  4,  5,  6,  7 );

  #define COLS 16
  #define ROWS 2*/

  /* 20x4 display
     WARNING: check data sheet for your display as you may need
     a different circuit to the one noted below!
     Notes:
      * connect GND of Arduino to RW pin of LCD
      * connect 5V pin of Arduino via a 100 Ohm resistor to A pin of LCD
        (so as to limit backlight current and brightness)
      * if you do not have a 10 kOhm potentiometer available to adjust
        contrast, create a voltage divider using 2 resistors as follows
        (where 'X' represents an unused location on a breadboard):
          * 10 kOhm resistor from 5V pin of Arduino to 'X'
          * 1.5 kOkm resistor from GND pin of Arduino to 'X'
          * connect 'X' to Vo pin (typically pin 3) of LCD

          If your voltage divider (or potentiometer) controlling the
          voltage to the Vo pin of your LCD is supplying an appropriate
          voltage for good contrast for good readability, prior to
          programming your Arduino you should see solid block characters
          filling the first and third rows of your 20x4 LCD and these
          should only moderately contrast with the empty second and
          fourth rows; if you see no solid block characters at all, or you
          see extremely obvious solid block characters that contrast very
          greatly with the empty rows, you will not be able to read
          the tape as ordinary characters will not be visible.

              LCD pins: RS, EN, D4, D5, D6, D7 */
  LiquidCrystal     lcd(8,  9,  4,  5,  6,  7 );
  #define COLS 20
  #define ROWS 4
#endif

// ============================================================================
// Default colors
#define MONOCHROME 0
#define BG_COLOR MONOCHROME
#define FG_COLOR MONOCHROME

// This 'tape-clcd.ino' implementation does NOT support colour but the code
// relating to colour support has not been removed because it is useful for the
// purpose of understanding where alternative means of highlighting
// (such as the use of custom characters) could be used to
// substitute for the use of colour (not yet done)
#define DISPLAY_COLOR false
#if DISPLAY_COLOR == true
  typedef struct {
    int id;
    int fgColor;
    int bgColor;
  } colorPair;

  // Set up convenient color pairs for subsequent use
  //   (not all of these are actually used, and more combinations
  //      are possible than those defined here).
  colorPair COLOR_BLACK = {0,FG_COLOR,BG_COLOR};
  colorPair COLOR_GREEN = {1,FG_COLOR,BG_COLOR};
  colorPair COLOR_RED = {2,FG_COLOR,BG_COLOR};
  colorPair COLOR_CYAN = {3,FG_COLOR,BG_COLOR};
  colorPair COLOR_WHITE = {4,FG_COLOR,BG_COLOR};
  colorPair COLOR_MAGENTA = {5,FG_COLOR,BG_COLOR};
  colorPair COLOR_BLUE = {6,FG_COLOR,BG_COLOR};
  colorPair COLOR_YELLOW = {7,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_BLACK = {8,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_GREEN = {9,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_RED = {10,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_CYAN = {11,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_WHITE = {12,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_MAGENTA = {13,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_BLUE = {14,FG_COLOR,BG_COLOR};
  colorPair COLOR_BG_YELLOW = {15,FG_COLOR,BG_COLOR};
  colorPair COLORS_DEFAULT = {16,FG_COLOR,BG_COLOR};

  #define COLOR_OK COLOR_GREEN
  #define COLOR_OVER COLOR_YELLOW
  #define COLOR_WRAP COLOR_YELLOW
  #define COLOR_INVIS COLOR_YELLOW
  #define COLOR_ILLEGAL_UTF8 COLOR_RED
  #define COLOR_UNSUPPORTED_UFT8 COLOR_CYAN
  #define COLOR_HT COLOR_MAGENTA
  #define COLOR_NL COLOR_BLUE
  #define COLOR_CR COLOR_GREEN
  #define COLOR_ERR COLOR_RED
  #define COLOR_CORRUPT_DATA COLOR_BG_RED
  #define COLOR_VIS_SPACE COLOR_MAGENTA

  void setColors(colorPair cp) {
    tft.textColor(cp.fgColor,cp.bgColor);
  }
#endif

void hideCursor()
{
  lcd.noCursor();
}

void showCursor()
{
  lcd.cursor();
}

void defaultColors() {
  // do nothing (since this is a monochrome implementation)
}

/* Move to stipulated row and column. */
void move(int row, int col) {
  lcd.setCursor(col, row);
}

void clear(tape_t *t) {
  lcd.clear(); // FIXME this positions cursor to home (not necessarily wanted)
}

void clrtobot(tape_t *t, int posAfter) { // FIXME slow
  hideCursor();
  int clearTo;
  #ifdef TAPE_SPLIT_TRC
    clearTo = t->maxPos2;
  #else
    clearTo = t->maxPos;
  #endif
  char spc = ' ';
  for (int i=t->pos; i < clearTo; i++) {
    int col = i % t->maxX;
    int row = i / t->maxX;
    move(row,col); // relocate cursor
    lcd.write(spc);
    move(row,col);
  }
  // FIXME refactor this position logic into a function of its own
  int col = posAfter % t->maxX;
  int row = posAfter / t->maxX;
  move(row,col); // relocate cursor
  showCursor();
}
// ============================================================================
// ============================================================================
/* Experimental for FVMO_LOCAL_TAPE */
#ifdef TAPE_LOCAL_TAPE

void writeServer(tape_t *t, ochar oc);
void handleServerChar(tape_t *t, CHAR_TYPE c);
/*
  This memcom feature is only to be used by embedding within tape source code.
  It is meant to allow this tape source code to be easily dropped in to
  form part of an FVM executable and act as a local tape (one that uses shared
  memory instead of a serial connection to communicate; that is, a standalone
  computing device with inbuilt local keyboard and inbuilt local display
  rather than being split into separate tape server and tape terminal).
  Currently works for FVMP_ARDUINO_IDE platform only.

  Note: this is done in such a way that Freeputer remains modular, since
  the same application will run either using a local tape or using a remote
  tape terminal (the application, such as 'ts.fl', running inside the FVM,
  simply reads from stdin and writes to stdout and stdtrc as usual).
*/
#define READBUF_SIZE 64 // FIXME make smaller, this large size is not needed
typedef struct memcom {
  int iReadBuf;
  uint8_t readBuf[READBUF_SIZE];
} memcom_t;

memcom_t memcom = {0};
memcom_t memcom1 = {0};

// Baud rate is ignored here
void memcom_begin(memcom_t *lt, unsigned long baud) {
  memset(lt->readBuf,0,READBUF_SIZE);
  lt->iReadBuf = 0;
}

// You MUST call memcom_available before calling memcom_read.
// It is the trigger for readBuf to be populated with data to be read.
// Note: this function, along with functionality in the memcom_write
// functions below, completely replaces input(tape_t *t); the latter is
// never used when in TAPE_LOCAL_TAPE mode.
//
// The number of available characters is returned.
// If there are not already at least n characters available,
// then we will block until a key has been pressed, so that at least
// one more character is available (although this does NOT necessarily
// imply that at least n characters will thereafter be available).
int memcom_available(memcom_t *lt, int n) {

  if (n < 1) { n == 1; } // Refuse nonsense values of n

  // It is possible that readBuf might already contain one or more
  // characters due to previous calls to writeByteServer
  if (lt->iReadBuf >= n) {
    return lt->iReadBuf;
  }

  showCursor();

  CHAR_TYPE c;
  uint16_t rawKeycode;
  uint16_t mappedKeycode;
  ochar oc;

  // Wait for a keypress
  bool received = false;
  while(!received) {
    if (keyboard.available()) {
      rawKeycode = keyboard.read();
      if (rawKeycode > 0) {
        mappedKeycode = keymap.remapKey(rawKeycode);
        c = mappedKeycode & 0x00ff;
        if (c != 0xfa) { // two 0xfa bytes often follow CapsLock, NumLock...
          received = true;
          oc.o = ORIGIN_LOCAL;
          oc.c = c;
          oc.rawKeycode = rawKeycode;
        }
      }
    }
  }

  // Process that keypress character (this will cause one or more characters
  // to be added to readBuf since writeServer intelligently calls
  // memcom_putForRead as appropriate)
  writeServer(&currentTape, oc);
 
  return lt->iReadBuf;
}

// This puts a character into the read buffer.
size_t memcom_putForRead(memcom_t *lt, uint8_t n) {
  if (lt->iReadBuf >= 0 && lt->iReadBuf < READBUF_SIZE) {
    lt->readBuf[(lt->iReadBuf++)] = n;
  } else {
    // First clear buffer and rewind cursor:
    memset(lt->readBuf,0,READBUF_SIZE);
    lt->iReadBuf = 0;
    lt->readBuf[(lt->iReadBuf)++] = n;
  }
  return 1;
}

// You MUST call memcom_available to populate the readBuf before calling
// memcom_read otherwise the behaviour of memcom_read may effectively be
// meaningless since the readBuf might be empty!
uint8_t memcom_read(memcom_t *lt) {
  uint8_t result;
  if (lt->iReadBuf > 0 && lt->iReadBuf < READBUF_SIZE) {
    result = lt->readBuf[0];
    for (int i=0; i < lt->iReadBuf; i++) {
      lt->readBuf[i] = lt->readBuf[i+1];
    }
    lt->readBuf[lt->iReadBuf] = 0;
    --(lt->iReadBuf);
    return result;
  } else {
    return 0;
  }
}

size_t memcom_write(memcom_t *lt, uint8_t n) {
  CHAR_TYPE c = n;
  ochar oc;
  #ifdef TAPE_STDTRC_SEP
    if (lt == &memcom1) {
      oc.o = ORIGIN_SERVER_STDTRC;
      oc.c = c;
      ttrace(&currentTape, oc.c);
    } else {
      oc.o = ORIGIN_SERVER;
      oc.c = c;
      handleServerChar(&currentTape, oc.c);
    }
  #else
    oc.o = ORIGIN_SERVER;
    oc.c = c;
    handleServerChar(&currentTape, oc.c);
  #endif
  return 1;
}

size_t memcom_write(memcom_t *lt, const uint8_t *buffer, size_t size) {
  int i = 0;
  for (; i<size; i++) {
    memcom_write(lt, buffer[i]);
  }
  return i;
}

#endif
// ============================================================================
// ============================================================================

void addchar(char c, tape_t *t, int atPos) {
  int col = atPos % t->maxX;
  int row = atPos / t->maxX;
  char buf = c;
  char spc = ' ';
  move(row,col);
  lcd.write(spc);
  // move back to row,col
  move(row,col);
  lcd.write(buf);
  // move right
  if (col == t->maxX-1) {
    col = 0;
    row = row % t->maxY;
  } else {
    ++col;
  }
}

/*
  Decode escape sequences (note: these start with DLE not ESC)
  being received from the server. These are essentially
  commands from the server to the tape. This logic is good enough
  to experiment with the tape as a user interface but is not
  particularly robust with respect to the error handling
  of nonsensical sequences. In future this logic could
  be expanded so as to improve error handling.
*/
void filterDleseq(tape_t *t, CHAR_TYPE c) {
  if (t->dleseq.seq[0] == CHAR_DLE) {
    // Already in an esc seq
    switch(t->dleseq.index) {    
      case(1):
        if ((c > 0 && c <= NUM_DLE_SIMPLE_INSTRS) ||
            (c > 127 && c<= NUM_DLE_COMPLEX_INSTRS+127)) {
          t->dleseq.seq[t->dleseq.index++] = c;
        } else {
          // bad dle esc seq at 1 (unknown instruction) 
        }
        break;
      case(2):
      case(3):
      case(4):
      case(5):
          t->dleseq.seq[t->dleseq.index++] = c;
        break;
      default:
        // bad dle esc seq, too long (sextet maximum)
      break;
    }
  } else {
    // Not already in an esc seq
      if (c == CHAR_DLE && t->dleseq.index == 0) {
        // Starting a new esc seq        
          t->dleseq.seq[t->dleseq.index++] = c;
      } else {
        // Normal input (not part of an esc seq)
        //   so do nothing here
      }
  }
}

/*
  To get correct behaviour of the interaction between server and tape,
  it is sometimes necessary to wait before sending a character to the server.
  This is important whenever the server is sending some multi-byte sequence,
  most notably escape sequences (commands, see filterDleseq) but also
  presumably multi-byte UTF-8 characters.
*/
void setServerReady(tape_t* t, bool boolValue) {
    t->serverReady = boolValue;
}

#ifndef TAPE_LOCAL_TAPE
  /*
    This input function waits for input coming from
    either the local keyboard or the server.
    Each received character is put in an ochar struct which is
    tagged with either ORIGIN_LOCAL or ORIGIN_SERVER or ORIGIN_SERVER_STDTRC
    so we know where the character came from.

    Note: this is never called in TAPE_LOCAL_TAPE mode.
  */
  ochar input(tape_t *t) {

    showCursor(); // FIXME appears necessary for CLCDs here

    CHAR_TYPE c;
    uint16_t rawKeycode;
    uint16_t mappedKeycode;
    ochar oc;

    // showCursor();
    bool received = false;
    while(!received) {
      if (keyboard.available()) {
        rawKeycode = keyboard.read();
        if (rawKeycode > 0) {
          mappedKeycode = keymap.remapKey(rawKeycode);
          c = mappedKeycode & 0x00ff;
          if (c != 0xfa) { // two 0xfa bytes often follow CapsLock, NumLock...
            received = true;
            oc.o = ORIGIN_LOCAL;
            oc.c = c;
            oc.rawKeycode = rawKeycode;
          }
        }
      } else if (Serial.available() > 0) {
        c = Serial.read();
        received = true;
        oc.o = ORIGIN_SERVER;
        oc.c = c;
      }
      #ifdef TAPE_STDTRC_SEP
        else if (Serial1.available() > 0) {
        c = Serial1.read();
        received = true;
        oc.o = ORIGIN_SERVER_STDTRC;
        oc.c = c;
      }
      #endif
    }
    return oc;
  }
#endif // #ifndef TAPE_LOCAL_TAPE

/*
  Display a message on the tape. By convention (since some tapes may be
  extremely short, only a few characters long), a tape 'message' is always
  only 1 character long and is always displayed in the last normal cell
  of the tape (that is, at maxPos; that is, prior to any stdtrc output
  should the tape happen to be in split-tape mode at the time).

  This allows the user to glance at the last normal cell of the tape
  and to easily see (depending on what character is displayed there,
  often with the help of colour-highlighting) whether all is well,
  whether an error has occurred, whether text has wrapped and
  therefore some text has been overwritten, or whether text
  cannot be displayed because the tape is full and
  not in wrap mode. See msgOK, msgOver, msgWrapped, msgErr.
*/
void msg(tape_t *t, CHAR_TYPE c) {
  tape_goto(t, t->maxPos);
  addchar(c,t,t->maxPos);
  tape_goto(t, t->lastPos);
}

#if DISPLAY_COLOR == true
void msgColor(tape_t *t, CHAR_TYPE c, colorPair cp) {
  setColors(cp);
  msg(t,c);
  defaultColors();
}

bool tape_putc_color(tape_t *t, CHAR_TYPE c, colorPair cp);
#endif

void msgOK(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '#', COLOR_OK);
  #else
    msg(t, '#');
  #endif
}

void msgOver(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '+', COLOR_OVER);
  #else
    msg(t, '+');
  #endif
}

void msgWrapped(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '&', COLOR_WRAP);
  #else
    msg(t, '&');
  #endif
}

void msgErr(tape_t *t) {
  #if DISPLAY_COLOR == true
    msgColor(t, '!', COLOR_ERR);
  #else
    msg(t, '!');
  #endif
}

void clearUtf8buf(tape_t *t) {
  for (int i=0; i++; i<6) {t->utf8buf[i] = 0;}
  t->iUtf8buf = 0;
  t->lenUtf8char = 0;
}

// FIXME change all the code that isn't getting compiled yet due to ifdefs
//   as it is still written for ncurses instead of RA8875
int showUtfchar(tape_t *t, CHAR_TYPE c) {
#ifdef SUPPORT_UTF8
  if (addstr(t->utf8buf) == ERR) {
    if (!t->wrap) {
      handleOver(t);
    }
  } else {
    getyx(stdscr, t->row, t->col); // ncurses: get cursor location
    t->pos = (t->row*t->maxX) + t->col; 
  }
#else
  #if DISPLAY_COLOR == true
    tape_putc_color(t, '~', COLOR_UNSUPPORTED_UFT8);
  #else
    tape_putc(t, '~');
  #endif
#endif
}

/*
  Show a UTF-8 character. This simple implementation is sufficient for
  most purposes but doesn't do robust checking for invalid UTF-8 characters
  and is incapable of recognizing combining characters such as ɔ̃
  as a single character (you will notice such combining characters
  end up getting displayed as 2 unknown characters).

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
void utf(tape_t *t, CHAR_TYPE c) {
  t->utf8buf[t->iUtf8buf] = c; 
  if (t->iUtf8buf == 0) {
    if ((c & 0b11100000) == 0b11000000) {
      t->lenUtf8char = 2;
    } else if ((c & 0b11110000) == 0b11100000) {
      t->lenUtf8char = 3;
    } else if ((c & 0b11111000) == 0b11110000) {
      t->lenUtf8char = 4;
    } else {
      // invalid char
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '?', COLOR_ILLEGAL_UTF8);
      #else
        tape_putc(t, '?');
      #endif
      clearUtf8buf(t);
      return;
    }
  } else if (t->iUtf8buf == t->lenUtf8char - 1 ) {
    showUtfchar(t,c);
    clearUtf8buf(t);
    return;
  }
  ++(t->iUtf8buf);
}

/*
  Show an ordinary visible character.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
int vis(tape_t *t, CHAR_TYPE c) {
  if (c == CHAR_SPC && t->visibleSpc) {
    #if DISPLAY_COLOR == true
      tape_putc_color(t, CHAR_VISIBLE_SPC, COLOR_VIS_SPACE);
    #else
      tape_putc(t, CHAR_VISIBLE_SPC);
    #endif
  } else {
    tape_putc(t,c);
  }
}

/*
  Show an invisible character by subsituting it with the appropriate
  visible substitute. By convention the only times a cell is allowed to
  appear empty is: (1) if it contains a space; (2) if it is empty.

  Note: in future the tape conventions might change so as
  a cell containing a space must if posible always appear visually
  different to an empty cell but for now it is not yet clear
  whether such a rule is necessary. For now there is at least
  Ctrl-U which turns on visible spaces.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
int invis(tape_t *t, CHAR_TYPE c) { // é
  switch(c) {
    case(CHAR_HT):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_HT);
      #else
        tape_putc(t, '*');
      #endif
      break;
    case(CHAR_NL):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_NL);
      #else
        tape_putc(t, '*');
      #endif
      break;
    case(CHAR_CR):
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '*', COLOR_CR);
      #else
        tape_putc(t, '*');
      #endif
      break;
    default:
      #if DISPLAY_COLOR == true
        tape_putc_color(t, '?', COLOR_INVIS);
      #else
        tape_putc(t, '?');
      #endif
      break;
  }
}

/*
  Show a character on the tape.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
void show(tape_t *t, CHAR_TYPE c) {
  if (c > 127) { // WARNING: this test only works for CHAR_TYPE of unsigned char
    utf(t, c);
  } else if (c < CHAR_SPC) {
    invis(t, c);
  } else {
    vis(t, c);
  }
}

#ifdef TAPE_SPLIT_TRC
  void splitModeOn(tape_t *t) {
    t->splitMode = true;
    t->maxY = ROWS; t->maxX = COLS; // get screen size
    t->maxPos2 = (t->maxX * t->maxY) - 1;
    t->maxPos = t->maxPos2 / 2; // Leave room for stdtrc display
    if (t->maxPos % t->maxX != t->maxX - 1) {
      if (t->maxY > 1) {
        // Avoid split mid-way through row
        t->maxPos = t->maxPos - (t->maxPos % t->maxX) - 1;
      }
    }    
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
    t->pos2 = t->maxPos + 1;
    tape_clear(t);
  }

  void splitModeOff(tape_t *t) {
    t->splitMode = false;
    t->maxY = ROWS; t->maxX = COLS; // get screen size
    t->maxPos = (t->maxX * t->maxY) - 1;
    t->maxPos2 = t->maxPos;
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
    tape_clear(t);
  }
#endif

void visSpcOn(tape_t *t) {
  t->visibleSpc = true;
  tape_clear(t);
}

void visSpcOff(tape_t *t) {
  t->visibleSpc = false;
  tape_clear(t);
}

/*
  Initialize the tape.
*/
bool tape_init(tape_t *t) {

  #ifdef TAPE_LOCAL_TAPE
  // Note: initialization of memcom is done in 'fvm.c' not here
  // (see FVMO_LOCAL_TAPE therein)
  #else
    Serial.begin(BAUD_RATE);
    while (!Serial) {}
  #endif

  #ifdef TAPE_STDTRC_SEP
    #ifdef TAPE_LOCAL_TAPE
    // Note: initialization of memcom1 is done in 'fvm.c' not here
    // (see FVMO_LOCAL_TAPE therein)
    #else
      Serial1.begin(BAUD_RATE_STDTRC);
      while (!Serial1) {}
    #endif
  #endif

  lcd.begin(COLS,ROWS);

  keyboard.begin( DATAPIN, IRQPIN ); // see 'PS2KeyAdvanced.h'
  keyboard.setNoBreak( 1 ); // no break codes on key release
  keyboard.setNoRepeat( 1 ); // no repeat on modifiers
  keymap.selectMap( (char *)"US" ); // US keyboard

  // ==========

  t->wrap = true;
  t->serverReady = false;

  #ifdef MULTIPLEX
    multiRouteIn = UNKNOWN_DEVICE_BYTE;
    multiRouteInKnown = false;
  #endif

  #ifdef TAPE_SPLIT_TRC
    splitModeOff(t);
  #else
    clear(t);
    t->maxX = COLS;
    t->maxY = ROWS;
    t->maxPos = (t->maxX * t->maxY) - 1;
    t->col = 0;
    t->row = 0;
    t->lastPos = 0;
    t->pos = 0;
  #endif
  msgOK(t);
  showCursor();
  return true;
}

void complexDle(tape_t *t, CHAR_TYPE dleCmd, int val);

/*
  Respond to a command from the server. That command, given by escape
  sequence (starting with DLE) must already have been fully received by
  the tape. Only then should this handleServerEscseq function be
  be called to carry out the action the server has commanded (such as
  clearing the tape or entering wrap mode).

  Like other functions in this tape implementation which deal with
  escape sequences, the error handling here is not particularly
  robust and could be expanded in future. But for now this
  works well under normal usage conditions with a
  sever that is properly implemented.
*/
void handleServerEscseq(tape_t *t) {
  switch(t->dleseq.seq[1]) {
    case(CMD_DLE_CLEAR):
        tape_clear(t);
      break;
    case(CMD_DLE_CLEAR_FROM):
      tape_clearFrom(t, t->pos);
      break;
    case(CMD_DLE_NO_WRAP):
      t->wrap = false;
      break;
    case(CMD_DLE_SPLIT_MODE):
      #ifdef TAPE_SPLIT_TRC
        if (t->splitMode == true) {
          splitModeOff(t);
        } else {
          splitModeOn(t);
        }
      #endif
      break;
    case(CMD_DLE_VISIBLE_SPC):
      if (t->visibleSpc == true) {
        visSpcOff(t);
      } else {
        visSpcOn(t);
      }
      break;
    case(CMD_DLE_WRAP):
      t->wrap = true;
      break;
    case(CMD_DLE_RIGHT):
      if (t->pos+1 > t->maxPos) {
        tape_goto(t,0);
      } else {
        tape_goto(t,t->pos+1);
      }
      break;
    case(CMD_DLE_LEFT):
      if (t->pos-1 < 0) {
        tape_goto(t,t->maxPos);
      } else {
        tape_goto(t,t->pos-1);
      }
      break;
    case(CMD_DLE_UP):
      if (t->pos - t->maxX < 0) { 
        tape_goto(t,0);
      } else {
        tape_goto(t,t->pos - t->maxX);
      }
      break;
    case(CMD_DLE_DOWN):
      if (t->pos + t->maxX > t->maxPos) { 
        tape_goto(t,t->maxPos);
      } else {
        tape_goto(t,t->pos + t->maxX);
      }
      break;
    case(CMD_DLE_REPORT_POS):
      complexDle(t, t->dleseq.seq[1], t->pos);
      break;
    case(CMD_DLE_REPORT_MAX_POS):
      complexDle(t, t->dleseq.seq[1], t->maxPos);
      break;
    case(CMD_DLE_GOTO_POS):
      tape_goto(t,*(int *)&(t->dleseq.seq[2]));
      break;
    default:
      // bad command in dle esc seq
      break;
  }
}

void clearEscseq(tape_t *t) {
  for (int i=0; i<SERVER_DLE_LEN; i++) {
    t->dleseq.seq[i] = 0;
  }
  t->dleseq.index = 0; 
}

/*
  Take appropriate action given that the specified character c has
  been received from the server.
*/
void handleServerChar(tape_t *t, CHAR_TYPE c) {

#ifdef MULTIPLEX
  if (!multiRouteInKnown) {
    switch(c) {
      case(STDOUT_BYTE):
        multiRouteIn = c;
        multiRouteInKnown = true;
        return;
      case(STDTRC_BYTE):
        multiRouteIn = c;
        multiRouteInKnown = true;
        return;
      break;    
      default:
        // Corrupt data received. Assuming the tape server FVM really is
        // running in FVMO_MULTIPLEX mode, by far the most common cause of
        // data corruption here is overflow of the Arduino serial buffer
        // of the tape terminal due to a flood of data received from stdtrc.
        // Short of writing a new Arduino serial library with flow control,
        // there is basically no simple, reliable solution to prevent this,
        // although running at slow baud rates reduces the frequency of
        // corruption. If you find it intolerable then:
        //   * use very little stdtrc output in your programs
        //   * type very slowly (reduces frequency of corruption)
        //   * or use a very slow BAUD RATE (use TAPE_VERY_SLOW_BAUD mode)
        //   * or do not enable MULTIPLEX mode (best solution)
        //     and also do not enable TAPE_SPLIT_TRC mode
        //   * or simply do not use split-tape mode
        //   * or use TAPE_STDTRC_SEP instead of MULTIPLEX mode and
        //     specify a slow BAUD_RATE_STDTRC (a good compromise)
        //   * or hack the Arduino Serial library so that it uses a larger
        //     buffer size (in theory should work but difficult in practice)
        //   * or use TAPE_LOCAL_MODE (see FVMO_LOCAL_TAPE in 'fvm.c')
        //     to use a keyboard and display directly connected to your
        //     FVM Arduino board without any intervening serial connection
        //   * or use a Linux tape terminal instead (see 'tape.c')
        //     as Linux uses buffering which eliminates the problem and
        //     allows a 115200 baud rate quite happily when connected to an
        //     Arduino tape server board via USB in the normal manner
        //
        // Show an exclamation mark where the next ordinary character
        // would have been shown, to indicate data corruption
        // (in addition to calling msgErr):
        #if DISPLAY_COLOR == true
          setColors(COLOR_CORRUPT_DATA);
          show(t,'!');
          defaultColors();
        #else
          show(t,'!');
        #endif
        msgErr(t);
        return;
      break;
    }
  }

  if (multiRouteIn == STDTRC_BYTE) {
    // We received a data byte from server's STDTRC
    ttrace(t, c);
  } else { // We received a data byte from server's STDOUT
#endif
    if (t->dleseq.seq[0] == CHAR_DLE) {
      // We are in an esc seq sent by the server
      if (t->dleseq.index < SERVER_DLE_LEN-1) {
        // We have not yet received the complete esc seq
        t->dleseq.seq[(t->dleseq.index)++] = c;
        setServerReady(t, false);
      } else {
        // We have received the complete esc seq
        handleServerEscseq(t);
        clearEscseq(t);
        setServerReady(t,true);
      }
    } else {
      if (c == CHAR_DLE ) {
        // This appears to be the start of an esc seq sent by the server
        clearEscseq(t);
        t->dleseq.seq[0] = CHAR_DLE;
        t->dleseq.index = 1;
        setServerReady(t, false);
      } else {
        // Not an esc seq at all
        if (c == CHAR_XON) {
          setServerReady(t,true);
        } else if ( c == CHAR_XOFF ) {
          setServerReady(t, false);
        } else {
          show(t,c);
          setServerReady(t,true);
        }
      }
    }
#ifdef MULTIPLEX
  }
  multiRouteInKnown = false;
#endif
}

/*
  Write a byte out over the serial device so that it is sent to the server.
  This code assumes that CHAR_TYPE is the size of a byte.
*/
void writeByteServer(tape_t *t, CHAR_TYPE c) {
  CHAR_TYPE buf[1];
  int countWritten;
#ifdef MULTIPLEX
    buf[0] = STDIN_BYTE;
  #ifdef TAPE_LOCAL_TAPE
    countWritten = memcom_putForRead(&memcom, buf[0]);
  #else
    countWritten = Serial.write(buf[0]);
  #endif

    if (countWritten < 1) {
      msgErr(t);
      return;
    }
#endif
  buf[0] = c;
  #ifdef TAPE_LOCAL_TAPE
    countWritten = memcom_putForRead(&memcom, buf[0]);
  #else
    countWritten = Serial.write(buf[0]);
  #endif
  if (countWritten < 1) {
    msgErr(t);
  }
  setServerReady(t, false); // Not ready until get a server response
}

/*
  Send a simple DLE escape sequence to the server.
  The format of escape sequences communicated between the server and
  the tape is always DLE_LENGTH bytes long (currently 6 bytes).
  The first byte is CHAR_DLE, the second byte is a command number,
  and the last 4 bytes are zeroes since a simple command takes no argument.
*/
void simpleDle(tape_t *t, CHAR_TYPE dleCmd) {
  writeByteServer(t,CHAR_DLE);
  writeByteServer(t,dleCmd);
  for (int i=0; i<DLE_LENGTH-2; i++) {
    writeByteServer(t,0);
  }
}

/*
  Send a complex DLE escape sequence to the server.
  The format of escape sequences communicated between the server and
  the tape is always DLE_LENGTH bytes long (currently 6 bytes).
  The first byte is CHAR_DLE, the second byte is a command number,
  and the last 4 bytes are represent a Freeputer WORD argument
  (two's complement, little-endian, 32-bit integer).

  Note: the format of a complex DLE escape sequence is used not only
  for the server to command the tape with an argument but also for the
  tape to respond to a query by providing information as an argument.
*/
void complexDle(tape_t *t, CHAR_TYPE dleCmd, int val) {
  writeByteServer(t,CHAR_DLE);
  writeByteServer(t,dleCmd);
  writeByteServer(t,(CHAR_TYPE)(val & 0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>8)&0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>16)&0xff));
  writeByteServer(t,(CHAR_TYPE)((val>>24)&0xff));
}

/*
  Write to the server. The supplied ochar oc might contain
  a simple character to be written to the server as is or might
  be recognized as a special key code (e.g. Ctrl-L or Up Arrow) from
  PS2KeyAdvanced/PS2KeyMap. If so we send (rather than the character oc.c)
  an appropriate simple or complex DLE escape sequence that the
  sever will recognize. The server, upon seeing such a command escape
  sequence, will make its own decision to either ignore the command or
  echo the command escape sequence back to the tape so that the
  tape takes action (such as clearing the tape) appropriate
  as commanded. See 'ts.fl' for an example of how that is done.
 */
void writeServer(tape_t *t, ochar oc) { 

    if (oc.rawKeycode == KEY_CLEARFROM) {
      simpleDle(t, CMD_DLE_CLEAR_FROM); return;
    }
    if (oc.rawKeycode == KEY_CLEAR) {
      simpleDle(t, CMD_DLE_CLEAR); return;
    }
    if (oc.rawKeycode == KEY_WRAPOFF) {
      simpleDle(t, CMD_DLE_NO_WRAP); return;
    }
    if (oc.rawKeycode == KEY_VSPC_TGL) {
      simpleDle(t, CMD_DLE_VISIBLE_SPC); return;
    }
    if (oc.rawKeycode == KEY_SPLIT_TGL) {
      simpleDle(t, CMD_DLE_SPLIT_MODE); return;
    }
    if (oc.rawKeycode == KEY_WRAPON) {
      simpleDle(t, CMD_DLE_WRAP); return;
    }
    if (oc.rawKeycode == KEY_RIGHT) {
      simpleDle(t, CMD_DLE_RIGHT); return;
    }
    if (oc.rawKeycode == KEY_LEFT) {
      simpleDle(t, CMD_DLE_LEFT); return;
    }
    if (oc.rawKeycode == KEY_UP) {
      simpleDle(t, CMD_DLE_UP); return;
    }
    if (oc.rawKeycode == KEY_DOWN) {
      simpleDle(t, CMD_DLE_DOWN); return;
    }
  if (oc.c != 0) {
    writeByteServer(t,oc.c);
  }
}

/*
  This handleOver function should be called when not in wrap mode and
  output has reached the end of the tape (not including the message cell
  and not including, if in split-tape mode, the second half of the
  tape which is used for stdtrc output). This handleOver function
  displays a message (by calling msgOver()) so the user knows
  overflow occurred and it also informs the server that overflow
  occurred by sending CHAR_EM to the server). The aim of this
  is to inform both the user and the server that some
  information the server wanted the user to see was
  not seen because the tape was full. What happens next is up to the
  server (it might keep sending characters anyway, knowing they
  will not be visible, or it might stop sending characters,
  or it might reposition the cursor, or command the tape to clear,
  or whatever is the appropriate behaviour for the application).

  In short: when in wrap mode, previous output is simply overwritten
  as the tape automatically would clear itself and keep displaying
  characters received (thus handleOver should never be called in wrap mode).
  That means the most recent output is always visible but earlier
  output might be overwriten. Whereas when not in wrap mode,
  earlier output is visible and more recent output is not visible
  once the tape becomes full (unless the server decides to make
  something else happen when handleOver is called and it subsequently
  receives CHAR_EM as notification). Note that EM is End of Medium.
*/
void handleOver(tape_t *t) {
  t->over = true;
  msgOver(t);
  writeByteServer(t,CHAR_EM);
}

#ifndef TAPE_LOCAL_TAPE
  /*
    This function could equally well have been called interact.
    It causes the tape to listen for bytes and respond appropriately
    to them, taking into account whether they originate locally from
    the keyboard or from the server (via the serial device). This is
    the main loop for the tape user interface interaction.

    Note: this is never called in TAPE_LOCAL_TAPE mode.
  */
  void slave(tape_t *t) {
    for (;;)
    {
        ochar oc = input(t);
        switch(oc.o) {
          case(ORIGIN_SERVER):
            handleServerChar(t, oc.c);
          break;
          case(ORIGIN_LOCAL):
            writeServer(t, oc);
          break;
          case(ORIGIN_SERVER_STDTRC):
            ttrace(t, oc.c);
          break;
          default:
            msgErr(t);
          break;
        }
    }
  }
#endif

#ifndef TAPE_LOCAL_TAPE
  /* 
    This is the entry point for starting and running a tape.
    It initializes the tape by calling tape_init and then enters interactive
    mode by calling slave. The user can then happily use the tape
    as the user interface to the application controlling it
    via the serial connection.

    Note: this is never called in TAPE_LOCAL_TAPE mode.
  */
  bool tape_run(tape_t *t) {
    bool initialized = tape_init(t);
    if (initialized) {
      slave(t);
    } else {
      tape_shutdown(t);
    }
    return initialized;
  }
#endif

// FIXME experimental
bool tape_local_init() {
  return tape_init(&currentTape);
}

/*
  Display a character on the tape. Monochrome only.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
bool tape_putc(tape_t *t, CHAR_TYPE c) {
  if (t->pos >= t->maxPos) {
    if (t->wrap) { // FIXME this is wrapping 1 char early? first time only?
      tape_clear(t);
      msgWrapped(t);
    } else {
      handleOver(t);
      return false;
    }
  }
  t->lastPos = t-> pos;
  addchar(c,t,t->pos);
  ++(t->pos); // FIXME possibly wrong? see addchar(
  return true;
}

/*
  Only used in split-tape mode.

  Goto the specified position pos2 in the the second half of the tape where
  stdtrc output is displayed. The cursor is sent to that cell.
*/
#ifdef TAPE_SPLIT_TRC
bool tape_goto2(tape_t *t, int pos2) {
  if (pos2 > t->maxPos2 || pos2 < t->maxPos+1 ) {
    return false;
  }
  t->pos2 = pos2;
  int col2 = pos2 % t->maxX;
  int row2 = pos2 / t->maxX;
  t->col = col2; // FIXME tricky, need elsewhere too? for goto2 etc?
  t->row = row2; // FIXME this looks wrong, why is it not causing a malfunction?
  move(row2,col2); // relocate cursor
  return true;
}

/*
  Only used in split-tape mode.

  Display a character in the second half of the tape where
  stdtrc output is displayed. Only 7-bit ASCII should be used here.

  Monochrome only.
*/
bool tape_putc2(tape_t *t, CHAR_TYPE c) {
  if (t->pos2 >= t->maxPos2) { // Always wrap tracing
    t->pos2 = t->maxPos+1;
    tape_goto2(t,t->pos2);
    tape_clearFrom(t,t->pos2); // FIXME slow!
  } else {
    tape_goto2(t,t->pos2);
  }
  addchar(c,t,t->pos2); // show character
  ++(t->pos2); // FIXME probably wrong? (see addchar)
  return true;
}
#endif

/*
  Display a character on the tape in colour.

  Note: this will only show a character on the normal part of the tape,
  no higher than maxPos. It will not show a character on the stdtrc
  part of the tape in split-tape mode.
*/
#if DISPLAY_COLOR == true
bool tape_putc_color(tape_t *t, CHAR_TYPE c, colorPair cp) {
  setColors(cp);
  bool outcome = tape_putc(t,c);
  defaultColors();
  return outcome;
}
  #ifdef TAPE_SPLIT_TRC
  bool tape_putc_color2(tape_t *t, CHAR_TYPE c, colorPair cp) {
    setColors(cp);
    bool outcome = tape_putc2(t,c);
    defaultColors();
    return outcome;
  }
  #endif
#endif

/*
  Goto the specified position pos in the tape.
  The cursor is sent to that cell.

  Note: this will only goto a cell in the normal part of the tape,
  no higher than maxPos. It will goto a cell on the stdtrc
  part of the tape in split-tape mode.
*/
bool tape_goto(tape_t *t, int pos) {
  if (pos > t->maxPos || pos < 0 ) {
    return false;
  }
  t->lastPos = t-> pos;
  t->pos = pos;
  t->col = pos % t->maxX;
  t->row = pos / t->maxX;
  move(t->row,t->col);
  return true;
}

/*
  Clear the whole tape.

  The OK message (a single character at maxPos) will be displayed
  to indicate all is well.
*/
bool tape_clear(tape_t *t) {
  clear(t); // clear screen
  msgOK(t);
  #ifdef TAPE_SPLIT_TRC
    if (t->splitMode == true) {
      t->pos2 = t->maxPos+1;
    }
  #endif
  tape_goto(t,0);
  return true;
}

/*
  Clear from the cursor to the end of the tape.

  The OK message (a single character at maxPos) will be displayed
  to indicate all is well.
*/
// FIXME hide cursor and/or speed this up
// FIXME does not correspond to func sig of tape.c
bool tape_clearFrom(tape_t *t, int posAfter) { 
  clrtobot(t,posAfter);
  msgOK(t);
  #ifdef TAPE_SPLIT_TRC
    if (t->splitMode == true) {
      t->pos2 = t->maxPos+1;
    }
  #endif
  return true;
}

/* Gracefully exit, shutting down the tape. */
bool tape_shutdown(tape_t *t) {
  // FIXME anything to do here on Arduino?
  return true;
}

// FIXME experimental
bool tape_local_shutdown() {
  tape_shutdown(&currentTape);
}

/*
  The following functions (vis2, invis2, show2) serve the same
  purpose for displaying characters on the second half of the tape
  in split-tape mode, where stdtrc output is displayed, as the
  corresponding (vis, invis, show) normal functions do but
  are only used for stdtrc output.

  Important: UTF-8 is not supported here.
  The second half of the tape, in this implementation,
  in split-tape mode only supports 7-bit ASCII. 
*/
#ifdef TAPE_SPLIT_TRC
int vis2(tape_t *t, CHAR_TYPE c) {
    if (c == CHAR_SPC && t->visibleSpc) {
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, CHAR_VISIBLE_SPC, COLOR_VIS_SPACE);
      #else
        tape_putc2(t, CHAR_VISIBLE_SPC);
      #endif
    } else {
      tape_putc2(t,c);
    }
}

int invis2(tape_t *t, CHAR_TYPE c) { // é
  switch(c) {
    case(CHAR_HT):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_HT);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    case(CHAR_NL):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_NL);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    case(CHAR_CR):
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '*', COLOR_CR);
      #else
        tape_putc2(t, '*');
      #endif
      break;
    default:
      #if DISPLAY_COLOR == true
        tape_putc_color2(t, '?', COLOR_INVIS);
      #else
        tape_putc2(t, '?');
      #endif
      break;
  }
}

void show2(tape_t *t, CHAR_TYPE c) {
  // Important: note the deliberate lack of UTF-8 support here.
  // The second half of the tape (in split-mode) only supports 7-bit ASCII
  // in this implementation. This keeps things simple.
  if (c > 127 || c < CHAR_SPC) {
    invis2(t, c);
  } else {
    vis2(t, c);
  }
}
#endif // #ifdef TAPE_SPLIT_TRC

/*
  Character c is assumed to be from stdtrc of the FVM instance the tape
  is communicating with over the physical serial connection. This ttrace
  function (in split-tape mode only) displays that character if TAPE_SPLIT_TRC
  is defined. None of this will work, however, unless either TAPE_STDTRC_SEP
  is defined and a second physical serial connection is used (see 'fvm.c')
  or MULTIPLEX is defined and the connected FVM instance properly supports
  multiplexing; these are optional features new in FVM 1.1. So this will
  not work with current FVM 1.0 implementations.

  This ttrace function (and split-tape mode in general) is mainly
  intended to be used when you are using multiplexing to connect this tape
  via a physical serial device to a remote computer or remote microcontroller
  and so you do not have direct access to the std.trc output file
  of the FVM instance running on that device. Therefore you would arrange
  for that remote FVM instance to be started up in MULTIPLEX mode
  (or in TAPE_STDTRC_SEP mode with a second physical serial connection)
  and then use split-tape mode (which uses this ttrace function)
  to happily view tracing output from the remote FVM.
*/
int ttrace(tape_t *t, CHAR_TYPE c) {
  #ifdef TAPE_SPLIT_TRC
    if (t->splitMode == true) {
      hideCursor();
      show2(t,c);
      tape_goto(t,t->pos);
      showCursor();
    }
  #endif
}

// ============================================================================
#ifndef TAPE_LOCAL_TAPE
  void setup() {
    // Do nothing here
  }

  void loop() 
  {
    tape_run(&currentTape); // FIXME evaluate bool returned
    while(true);
  }
#endif
